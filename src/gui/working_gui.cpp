#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressBar>
#include <QTextEdit>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QTime>
#include <QCloseEvent>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QTextCursor>

// Include the PKG extraction library
#include "pkg_tool/lib.h"

class ExtractionWorker : public QObject {
    Q_OBJECT

public:
    ExtractionWorker(const QString& pkgPath, const QString& outputPath, QObject* parent = nullptr)
        : QObject(parent), m_pkgPath(pkgPath), m_outputPath(outputPath), m_isRunning(false), 
          m_progressTimer(nullptr), m_currentProgress(0), m_lastActivityTime(0) {}

    ~ExtractionWorker() {
        m_isRunning = false;
        if (m_progressTimer) {
            m_progressTimer->stop();
            m_progressTimer->deleteLater();
        }
    }

signals:
    void progressUpdate(int percentage);
    void statusUpdate(const QString& status);
    void fileUpdate(const QString& currentFile);
    void finished(bool success, const QString& error);

public slots:
    void startExtraction() {
        m_isRunning = true;
        m_currentProgress = 0;
        
        // Start a timer to provide regular progress updates
        m_progressTimer = new QTimer(this);
        connect(m_progressTimer, &QTimer::timeout, this, &ExtractionWorker::providePulseUpdate);
        m_progressTimer->start(500); // Update every 500ms to keep UI responsive
        
        emit statusUpdate("Starting PKG extraction...");
        emit progressUpdate(5);
        
        try {
            // Convert Qt strings to std::filesystem::path
            std::filesystem::path pkgPath = m_pkgPath.toStdString();
            std::filesystem::path outputPath = m_outputPath.toStdString();
            
            // Make sure output directory exists
            QDir().mkpath(m_outputPath);
            
            emit statusUpdate("Reading PKG metadata...");
            emit progressUpdate(10);
            
            // First, read PKG metadata to get file count and validate the file
            PkgMetadata metadata;
            auto metadataError = ReadPkgMetadata(pkgPath, metadata);
            if (metadataError) {
                emit finished(false, QString("Failed to read PKG metadata: %1").arg(QString::fromStdString(*metadataError)));
                return;
            }
            
            emit statusUpdate(QString("PKG contains %1 files, Title ID: %2, Size: %3 MB")
                            .arg(metadata.file_count)
                            .arg(QString::fromStdString(metadata.title_id))
                            .arg(metadata.pkg_size / (1024 * 1024)));
            emit progressUpdate(15);
            
            // Set up progress callback with enhanced tracking for large files
            auto progressCallback = [this](const ExtractionProgress& progress) {
                if (!m_isRunning) return;
                
                QMutexLocker locker(&m_mutex);
                
                // Convert progress to percentage (15% already used for setup)
                int percentage = 15 + static_cast<int>(progress.total_progress * 80.0);
                m_currentProgress = percentage;
                m_lastActivityTime = QDateTime::currentMSecsSinceEpoch();
                
                emit progressUpdate(percentage);
                
                // Enhanced file update for better large file tracking
                QString currentFileInfo;
                if (progress.file_progress > 0.0 && progress.file_progress < 1.0) {
                    // Show intra-file progress for large files
                    currentFileInfo = QString("Extracting: %1 (%2%) - File %3/%4")
                                    .arg(QString::fromStdString(progress.current_file))
                                    .arg(static_cast<int>(progress.file_progress * 100))
                                    .arg(progress.current_file_index + 1)
                                    .arg(progress.total_files);
                } else {
                    currentFileInfo = QString("Extracting: %1 (%2/%3)")
                                    .arg(QString::fromStdString(progress.current_file))
                                    .arg(progress.current_file_index + 1)
                                    .arg(progress.total_files);
                }
                
                m_currentFileInfo = currentFileInfo;
                emit fileUpdate(currentFileInfo);
                
                // More frequent status updates for large file operations
                static size_t lastUpdateIndex = SIZE_MAX;
                if (progress.current_file_index != lastUpdateIndex || 
                    (progress.file_progress > 0.0 && static_cast<int>(progress.file_progress * 100) % 2 == 0)) {
                    
                    QString statusInfo;
                    if (progress.file_progress > 0.0 && progress.file_progress < 1.0) {
                        statusInfo = QString("Processing large file: %1% complete")
                                   .arg(static_cast<int>(progress.file_progress * 100));
                    } else {
                        statusInfo = QString("Extracted %1/%2 files... (%3%)")
                                   .arg(progress.current_file_index)
                                   .arg(progress.total_files)
                                   .arg(percentage);
                    }
                    
                    m_currentStatusInfo = statusInfo;
                    emit statusUpdate(statusInfo);
                    lastUpdateIndex = progress.current_file_index;
                }
            };
            
            emit statusUpdate("Extracting PKG files...");
            
            // Extract all files (empty indices vector means extract all)
            std::vector<int> indices;
            auto extractError = ExtractPkg(pkgPath, outputPath, indices, progressCallback);
            
            if (extractError) {
                emit finished(false, QString("Extraction failed: %1").arg(QString::fromStdString(*extractError)));
                return;
            }
            
            if (!m_isRunning) {
                emit finished(false, "Extraction cancelled");
                return;
            }
            
            // Stop the progress timer
            if (m_progressTimer) {
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_progressTimer = nullptr;
            }
            
            emit progressUpdate(100);
            emit statusUpdate("Extraction completed successfully!");
            emit finished(true, "");
            
        } catch (const std::exception& e) {
            if (m_progressTimer) {
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_progressTimer = nullptr;
            }
            emit finished(false, QString("Extraction failed with exception: %1").arg(e.what()));
        } catch (...) {
            if (m_progressTimer) {
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_progressTimer = nullptr;
            }
            emit finished(false, "Extraction failed with unknown error");
        }
    }

private slots:
    void providePulseUpdate() {
        if (!m_isRunning) return;
        
        QMutexLocker locker(&m_mutex);
        
        // Check if we haven't received an update in a while (more than 2 seconds)
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        qint64 timeSinceLastActivity = currentTime - m_lastActivityTime;
        
        if (timeSinceLastActivity > 2000) {
            // Provide a visual pulse to show we're still working
            static int pulseCounter = 0;
            pulseCounter++;
            
            int dotCount = (pulseCounter % 4);
            QString dots = QString(".").repeated(dotCount);
            
            QString pulseStatus = m_currentStatusInfo;
            if (!pulseStatus.isEmpty()) {
                // Add dots to show activity
                pulseStatus += QString(" Working%1").arg(dots);
                emit statusUpdate(pulseStatus);
            }
            
            // Keep file info current
            if (!m_currentFileInfo.isEmpty()) {
                emit fileUpdate(m_currentFileInfo + QString(" [Processing%1]").arg(dots));
            }
        }
    }

private:
    QString m_pkgPath;
    QString m_outputPath;
    bool m_isRunning;
    QMutex m_mutex;
    QTimer* m_progressTimer;
    int m_currentProgress;
    qint64 m_lastActivityTime;
    QString m_currentFileInfo;
    QString m_currentStatusInfo;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent), isBatchMode(false), currentBatchIndex(0), totalBatchFiles(0) {
        setWindowTitle("ShadPS4Manager v1.0.0");
        setMinimumSize(600, 500);
        setupUI();
        applyStyles();
    }
    
    void closeEvent(QCloseEvent* event) override {
        // Ask for confirmation if extraction is running
        if (extractButton && !extractButton->isEnabled()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, 
                "Confirm Exit", 
                "An extraction is currently in progress. Are you sure you want to exit?",
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply != QMessageBox::Yes) {
                event->ignore();
                return;
            }
        }
        event->accept();
    }

private slots:
    void selectPkgFile() {
        // Reset batch mode when selecting single file
        isBatchMode = false;
        selectedPkgFiles.clear();
        
        QString file = QFileDialog::getOpenFileName(
            this, 
            "Select PS4 PKG File", 
            QDir::homePath(), 
            "PKG Files (*.pkg);;All Files (*)"
        );
        
        if (!file.isEmpty()) {
            QFileInfo info(file);
            selectedFile = file;
            fileLabel->setText("Selected: " + info.fileName());
            fileSizeLabel->setText(QString("Size: %1 MB").arg(info.size() / (1024.0 * 1024.0), 0, 'f', 1));
            extractButton->setEnabled(true);
        }
    }

    void selectPkgDirectory() {
        QString dir = QFileDialog::getExistingDirectory(
            this, 
            "Select Directory with PKG Files", 
            QDir::homePath()
        );
        
        if (!dir.isEmpty()) {
            // Find all PKG files in the selected directory
            QDir pkgDir(dir);
            QStringList pkgFiles = pkgDir.entryList(QStringList("*.pkg"), QDir::Files);
            
            if (pkgFiles.isEmpty()) {
                QMessageBox::information(this, "No PKG Files", 
                    QString("No PKG files found in directory:\n%1").arg(dir));
                return;
            }
            
            // Convert to absolute paths
            selectedPkgFiles.clear();
            for (const QString& file : pkgFiles) {
                selectedPkgFiles.append(pkgDir.absoluteFilePath(file));
            }
            
            isBatchMode = true;
            selectedFile.clear(); // Clear single file selection
            
            fileLabel->setText(QString("Selected: %1 PKG files from directory").arg(pkgFiles.size()));
            
            // Calculate total size
            qint64 totalSize = 0;
            for (const QString& file : selectedPkgFiles) {
                totalSize += QFileInfo(file).size();
            }
            fileSizeLabel->setText(QString("Total Size: %1 MB").arg(totalSize / (1024.0 * 1024.0), 0, 'f', 1));
            extractButton->setEnabled(true);
        }
    }

    void selectOutputDir() {
        QString dir = QFileDialog::getExistingDirectory(
            this, 
            "Select Output Directory", 
            QDir::homePath()
        );
        
        if (!dir.isEmpty()) {
            outputDirectory = dir;
            outputLabel->setText("Output: " + dir);
        }
    }

    void startExtraction() {
        if (!isBatchMode && selectedFile.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select a PKG file or directory first.");
            return;
        }
        
        if (isBatchMode && selectedPkgFiles.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select a PKG file or directory first.");
            return;
        }
        
        if (outputDirectory.isEmpty()) {
            if (isBatchMode) {
                // For batch mode, ask user to select output directory
                QString dir = QFileDialog::getExistingDirectory(
                    this, 
                    "Select Output Directory for Batch Extraction", 
                    QDir::homePath()
                );
                if (dir.isEmpty()) {
                    QMessageBox::information(this, "Output Required", 
                        "Please select an output directory for batch extraction.");
                    return;
                }
                outputDirectory = dir;
            } else {
                // For single file, auto-generate output directory
                QFileInfo info(selectedFile);
                outputDirectory = info.dir().absolutePath() + "/" + info.baseName() + "_extracted";
            }
        }
        
        // Disable buttons during extraction
        extractButton->setEnabled(false);
        selectFileButton->setEnabled(false);
        selectDirButton->setEnabled(false);
        selectOutputDirButton->setEnabled(false);
        
        // Clear previous output
        outputText->clear();
        progressBar->setValue(0);
        
        if (isBatchMode) {
            // Start batch processing
            currentBatchIndex = 0;
            totalBatchFiles = selectedPkgFiles.size();
            statusLabel->setText(QString("Processing %1 of %2 files...").arg(1).arg(totalBatchFiles));
            processBatchFile();
        } else {
            // Process single file
            statusLabel->setText("Preparing extraction...");
            
            // Set up worker thread
            QThread* thread = new QThread;
            ExtractionWorker* worker = new ExtractionWorker(selectedFile, outputDirectory);
            worker->moveToThread(thread);
            
            // Connect signals
            connect(thread, &QThread::started, worker, &ExtractionWorker::startExtraction);
            connect(worker, &ExtractionWorker::progressUpdate, progressBar, &QProgressBar::setValue);
            connect(worker, &ExtractionWorker::statusUpdate, statusLabel, &QLabel::setText);
            connect(worker, &ExtractionWorker::fileUpdate, this, &MainWindow::onFileUpdate);
            connect(worker, &ExtractionWorker::finished, this, &MainWindow::onExtractionFinished);
            connect(worker, &ExtractionWorker::finished, thread, &QThread::quit);
            connect(thread, &QThread::finished, worker, &QObject::deleteLater);
            connect(thread, &QThread::finished, thread, &QObject::deleteLater);
            
            thread->start();
        }
    }

    void onFileUpdate(const QString& currentFile) {
        outputText->append(currentFile);
        // Auto-scroll to bottom
        QTextCursor cursor = outputText->textCursor();
        cursor.movePosition(QTextCursor::End);
        outputText->setTextCursor(cursor);
    }

    void onExtractionFinished(bool success, const QString& error) {
        // Re-enable buttons
        extractButton->setEnabled(true);
        selectFileButton->setEnabled(true);
        selectDirButton->setEnabled(true);
        selectOutputDirButton->setEnabled(true);
        
        if (success) {
            QMessageBox::information(this, "Success", 
                QString("PKG extraction completed successfully!\n\nOutput directory: %1").arg(outputDirectory));
            statusLabel->setText("Extraction completed successfully!");
        } else {
            QMessageBox::critical(this, "Error", 
                QString("PKG extraction failed:\n\n%1").arg(error));
            statusLabel->setText("Extraction failed!");
        }
    }
    
    void processBatchFile() {
        if (currentBatchIndex >= selectedPkgFiles.size()) {
            // Batch processing complete
            onBatchFinished();
            return;
        }
        
        QString currentFile = selectedPkgFiles[currentBatchIndex];
        QFileInfo info(currentFile);
        QString fileOutputDir = outputDirectory + "/" + info.baseName() + "_extracted";
        
        outputText->append(QString("\n=== Processing file %1 of %2 ===").arg(currentBatchIndex + 1).arg(totalBatchFiles));
        outputText->append(QString("File: %1").arg(info.fileName()));
        outputText->append(QString("Output: %1").arg(fileOutputDir));
        
        ExtractionWorker* worker = new ExtractionWorker(currentFile, fileOutputDir);
        QThread* thread = new QThread();
        
        worker->moveToThread(thread);
        
        connect(thread, &QThread::started, worker, &ExtractionWorker::startExtraction);
        connect(worker, &ExtractionWorker::progressUpdate, this, &MainWindow::updateBatchProgress);
        connect(worker, &ExtractionWorker::statusUpdate, statusLabel, &QLabel::setText);
        connect(worker, &ExtractionWorker::fileUpdate, this, &MainWindow::onFileUpdate);
        connect(worker, &ExtractionWorker::finished, this, &MainWindow::onBatchFileFinished);
        connect(worker, &ExtractionWorker::finished, thread, &QThread::quit);
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        
        thread->start();
    }
    
    void updateBatchProgress(int percentage) {
        // Calculate overall batch progress
        int fileProgress = percentage;
        int overallProgress = ((currentBatchIndex * 100) + fileProgress) / totalBatchFiles;
        progressBar->setValue(overallProgress);
    }
    
    void onBatchFileFinished(bool success, const QString& error) {
        if (success) {
            outputText->append(QString("✓ Successfully extracted file %1").arg(currentBatchIndex + 1));
        } else {
            outputText->append(QString("✗ Failed to extract file %1: %2").arg(currentBatchIndex + 1).arg(error));
        }
        
        currentBatchIndex++;
        
        // Process next file
        QTimer::singleShot(100, this, &MainWindow::processBatchFile);
    }
    
    void onBatchFinished() {
        statusLabel->setText("Batch extraction completed!");
        progressBar->setValue(100);
        
        // Re-enable buttons
        extractButton->setEnabled(true);
        selectFileButton->setEnabled(true);
        selectDirButton->setEnabled(true);
        selectOutputDirButton->setEnabled(true);
        
        outputText->append(QString("\n=== Batch Extraction Complete ==="));
        outputText->append(QString("Processed %1 PKG files").arg(totalBatchFiles));
        outputText->append(QString("Output directory: %1").arg(outputDirectory));
        
        QMessageBox::information(this, "Batch Extraction Complete", 
            QString("Successfully processed %1 PKG files!\n\nOutput directory: %2").arg(totalBatchFiles).arg(outputDirectory));
    }
    
    void showAbout() {
        QMessageBox::about(this, "About ShadPS4Manager",
            "<h3>ShadPS4Manager v1.0.0</h3>"
            "<p><b>PS4 PKG File Extractor</b></p>"
            "<p>A GUI application for extracting PlayStation 4 PKG files.</p>"
            "<br>"
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>Single PKG file extraction</li>"
            "<li>Batch directory processing</li>"
            "<li>Memory-optimized for large files (20+ GB)</li>"
            "<li>Real-time progress reporting</li>"
            "<li>Integrated cryptography and compression support</li>"
            "</ul>"
            "<br>"
            "<p><b>Usage:</b></p>"
            "<ul>"
            "<li><b>Single File:</b> Click 'Select PKG File' to choose a single .pkg file</li>"
            "<li><b>Batch Mode:</b> Click 'Select PKG Directory' to process all .pkg files in a folder</li>"
            "<li><b>Output:</b> Specify custom output directory or use auto-generated paths</li>"
            "</ul>"
            "<br>"
            "<p>Supports PKG files up to 20+ GB with optimized memory usage.</p>"
        );
    }

private:
    void setupUI() {
        auto* mainLayout = new QVBoxLayout(this);
        
        // Header
        auto* headerLabel = new QLabel("ShadPS4Manager - PKG File Extractor");
        headerLabel->setObjectName("headerLabel");
        mainLayout->addWidget(headerLabel);
        
        // File selection section
        auto* fileSection = new QVBoxLayout();
        auto* fileRow = new QHBoxLayout();
        
        selectFileButton = new QPushButton("Select PKG File");
        selectDirButton = new QPushButton("Select PKG Directory"); // Changed from output to input directory
        fileLabel = new QLabel("No file selected");
        fileSizeLabel = new QLabel("");
        
        fileRow->addWidget(selectFileButton);
        fileRow->addWidget(selectDirButton);
        fileRow->addWidget(fileLabel, 1);
        fileSection->addLayout(fileRow);
        fileSection->addWidget(fileSizeLabel);
        
        // Output directory section
        auto* outputRow = new QHBoxLayout();
        selectOutputDirButton = new QPushButton("Select Output Directory");
        outputLabel = new QLabel("Output: (auto-selected)");
        
        outputRow->addWidget(selectOutputDirButton);
        outputRow->addWidget(outputLabel, 1);
        
        // Extract button
        extractButton = new QPushButton("Extract PKG");
        extractButton->setEnabled(false);
        extractButton->setObjectName("extractButton");
        
        // Help/About button
        auto* helpButton = new QPushButton("About");
        helpButton->setObjectName("helpButton");
        
        auto* buttonRow = new QHBoxLayout();
        buttonRow->addWidget(extractButton, 1);
        buttonRow->addWidget(helpButton);
        
        // Progress section
        auto* progressSection = new QVBoxLayout();
        statusLabel = new QLabel("Ready to extract");
        progressBar = new QProgressBar();
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
        
        progressSection->addWidget(statusLabel);
        progressSection->addWidget(progressBar);
        
        // Output text area
        outputText = new QTextEdit();
        outputText->setMaximumHeight(200);
        outputText->setReadOnly(true);
        outputText->setPlaceholderText("Extraction progress will be shown here...");
        
        // Add all sections to main layout
        mainLayout->addLayout(fileSection);
        mainLayout->addLayout(outputRow);
        mainLayout->addLayout(buttonRow);
        mainLayout->addLayout(progressSection);
        mainLayout->addWidget(outputText);
        
        // Connect signals
        connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::selectPkgFile);
        connect(selectDirButton, &QPushButton::clicked, this, &MainWindow::selectPkgDirectory);
        connect(selectOutputDirButton, &QPushButton::clicked, this, &MainWindow::selectOutputDir);
        connect(extractButton, &QPushButton::clicked, this, &MainWindow::startExtraction);
        connect(helpButton, &QPushButton::clicked, this, &MainWindow::showAbout);
    }
    
    void applyStyles() {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
            }
            
            #headerLabel {
                font-size: 18px;
                font-weight: bold;
                color: #4CAF50;
                padding: 10px;
                margin-bottom: 10px;
            }
            
            QPushButton {
                background-color: #404040;
                border: 1px solid #555555;
                border-radius: 4px;
                padding: 8px 16px;
                font-size: 14px;
                min-width: 120px;
            }
            
            QPushButton:hover {
                background-color: #505050;
                border-color: #777777;
            }
            
            QPushButton:pressed {
                background-color: #353535;
            }
            
            QPushButton:disabled {
                background-color: #303030;
                color: #666666;
                border-color: #444444;
            }
            
            #extractButton {
                background-color: #4CAF50;
                border-color: #45a049;
                font-weight: bold;
                min-height: 35px;
            }
            
            #extractButton:hover {
                background-color: #45a049;
            }
            
            #extractButton:disabled {
                background-color: #2d5530;
                color: #666666;
            }
            
            #helpButton {
                background-color: #2196F3;
                border-color: #1976D2;
                min-width: 80px;
            }
            
            #helpButton:hover {
                background-color: #1976D2;
            }
            
            QLabel {
                color: #cccccc;
                font-size: 13px;
            }
            
            QProgressBar {
                border: 1px solid #555555;
                border-radius: 4px;
                background-color: #353535;
                text-align: center;
                font-weight: bold;
            }
            
            QProgressBar::chunk {
                background-color: #4CAF50;
                border-radius: 3px;
            }
            
            QTextEdit {
                background-color: #1e1e1e;
                border: 1px solid #555555;
                border-radius: 4px;
                padding: 8px;
                font-family: 'Consolas', 'Monaco', monospace;
                font-size: 11px;
                color: #cccccc;
            }
        )");
    }
    
    // UI components
    QPushButton* selectFileButton;
    QPushButton* selectDirButton;
    QPushButton* selectOutputDirButton;
    QPushButton* extractButton;
    QLabel* fileLabel;
    QLabel* fileSizeLabel;
    QLabel* outputLabel;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    QTextEdit* outputText;
    
    // State
    QString selectedFile;
    QString outputDirectory;
    QStringList selectedPkgFiles; // For batch processing
    bool isBatchMode;
    int currentBatchIndex;
    int totalBatchFiles;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "working_gui.moc"