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

// Include the PKG extraction library
#include "pkg_tool/lib.h"

class ExtractionWorker : public QObject {
    Q_OBJECT

public:
    ExtractionWorker(const QString& pkgPath, const QString& outputPath, QObject* parent = nullptr)
        : QObject(parent), m_pkgPath(pkgPath), m_outputPath(outputPath), m_isRunning(false) {}

    ~ExtractionWorker() {
        m_isRunning = false;
    }

signals:
    void progressUpdate(int percentage);
    void statusUpdate(const QString& status);
    void fileUpdate(const QString& currentFile);
    void finished(bool success, const QString& error);

public slots:
    void startExtraction() {
        m_isRunning = true;
        
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
            
            emit statusUpdate(QString("PKG contains %1 files, Title ID: %2")
                            .arg(metadata.file_count)
                            .arg(QString::fromStdString(metadata.title_id)));
            emit progressUpdate(15);
            
            // Set up progress callback
            auto progressCallback = [this](const ExtractionProgress& progress) {
                if (!m_isRunning) return;
                
                QMutexLocker locker(&m_mutex);
                
                // Convert progress to percentage (15% already used for setup)
                int percentage = 15 + static_cast<int>(progress.total_progress * 80.0);
                
                emit progressUpdate(percentage);
                emit fileUpdate(QString("Extracting: %1 (%2/%3)")
                              .arg(QString::fromStdString(progress.current_file))
                              .arg(progress.current_file_index + 1)
                              .arg(progress.total_files));
                
                if (progress.current_file_index % 10 == 0) {
                    emit statusUpdate(QString("Extracted %1/%2 files...")
                                    .arg(progress.current_file_index)
                                    .arg(progress.total_files));
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
            
            emit progressUpdate(100);
            emit statusUpdate("Extraction completed successfully!");
            emit finished(true, "");
            
        } catch (const std::exception& e) {
            emit finished(false, QString("Extraction failed with exception: %1").arg(e.what()));
        } catch (...) {
            emit finished(false, "Extraction failed with unknown error");
        }
    }
    
    private:
    QString m_pkgPath;
    QString m_outputPath;
    bool m_isRunning;
    QMutex m_mutex;
};

private slots:
    void readProcessOutput() {
        if (!m_process) return;
        QByteArray data = m_process->readAllStandardOutput();
        QString output = QString::fromLocal8Bit(data);
        if (!output.trimmed().isEmpty()) {
            emit fileUpdate("Processing: " + output.trimmed());
        }
    }
    
    void readProcessError() {
        if (!m_process) return;
        QByteArray data = m_process->readAllStandardError();
        QString error = QString::fromLocal8Bit(data);
        if (!error.trimmed().isEmpty()) {
            emit statusUpdate("Warning: " + error.trimmed());
        }
    }
    
    void updateProgress() {
        if (m_currentProgress < 85) {
            // Calculate elapsed time
            qint64 elapsed = m_startTime.secsTo(QDateTime::currentDateTime());
            
            // Update progress more conservatively for large files
            if (elapsed > 60) { // After 1 minute, slow down progress increments
                m_currentProgress += 2;
            } else {
                m_currentProgress += 5;
            }
            
            emit progressUpdate(m_currentProgress);
            
            // Update status messages with time info
            QString timeStr = QString(" (Elapsed: %1:%2)")
                .arg(elapsed / 60, 2, 10, QChar('0'))
                .arg(elapsed % 60, 2, 10, QChar('0'));
            
            if (m_currentProgress == 25) {
                emit statusUpdate("Reading PKG header..." + timeStr);
            } else if (m_currentProgress == 35) {
                emit statusUpdate("Extracting game files..." + timeStr);
            } else if (m_currentProgress == 50) {
                emit statusUpdate("Processing assets..." + timeStr);
            } else if (m_currentProgress == 65) {
                emit statusUpdate("Extracting additional content..." + timeStr);
            } else if (m_currentProgress == 80) {
                emit statusUpdate("Finalizing extraction..." + timeStr);
            }
        }
    }
    
    void monitorExtractionProgress() {
        if (!m_process || m_process->state() != QProcess::Running) {
            return;
        }
        
        // Check output directory size and file count
        qint64 currentOutputSize = getDirectorySize(m_outputPath);
        int currentFileCount = countFilesInDirectory(m_outputPath);
        
        // Calculate progress based on output size vs expected size
        if (m_pkgSizeBytes > 0 && currentOutputSize > m_lastOutputSize) {
            double sizeProgress = (double)currentOutputSize / (double)m_pkgSizeBytes;
            int calculatedProgress = 15 + (int)(sizeProgress * 70); // 15% to 85% based on size
            
            if (calculatedProgress > m_currentProgress && calculatedProgress <= 85) {
                m_currentProgress = calculatedProgress;
                emit progressUpdate(m_currentProgress);
            }
            
            // Update file info
            QString latestFile = getLatestModifiedFile(m_outputPath);
            if (!latestFile.isEmpty() && latestFile != m_lastProcessedFile) {
                m_lastProcessedFile = latestFile;
                emit fileUpdate(QString("Extracting: %1").arg(QFileInfo(latestFile).fileName()));
            }
            
            // Update status with size and speed info
            qint64 sizeDiff = currentOutputSize - m_lastOutputSize;
            m_lastOutputSize = currentOutputSize;
            
            QString sizeStr = formatFileSize(currentOutputSize);
            QString speedStr = "";
            if (sizeDiff > 0) {
                double speedMBps = (sizeDiff / (1024.0 * 1024.0)) / 1.0; // Per second (timer runs every 1s)
                speedStr = QString(" (Speed: %1 MB/s)").arg(speedMBps, 0, 'f', 1);
            }
            
            emit statusUpdate(QString("Extracted %1 files, %2%3")
                .arg(currentFileCount - m_initialFileCount)
                .arg(sizeStr)
                .arg(speedStr));
        }
    }
    
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        // Stop all timers
        if (m_progressTimer) {
            m_progressTimer->stop();
            m_progressTimer->deleteLater();
            m_progressTimer = nullptr;
        }
        
        if (m_directoryTimer) {
            m_directoryTimer->stop();
            m_directoryTimer->deleteLater();
            m_directoryTimer = nullptr;
        }
        
        if (exitStatus == QProcess::CrashExit) {
            emit finished(false, "Extraction process crashed");
            return;
        }
        
        if (exitCode == 0) {
            emit statusUpdate("Extraction completed successfully!");
            emit progressUpdate(100);
            emit finished(true, "");
        } else {
            QString error = QString("Extraction failed with exit code %1")
                           .arg(exitCode);
            if (m_process) {
                QString stderr = QString::fromLocal8Bit(m_process->readAllStandardError());
                if (!stderr.isEmpty()) {
                    error += "\nError details: " + stderr;
                }
            }
            emit finished(false, error);
        }
    }

private:
    // Helper functions for directory monitoring
    qint64 getDirectorySize(const QString& path) {
        QDir dir(path);
        qint64 totalSize = 0;
        
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Name);
        for (const QFileInfo& info : files) {
            totalSize += info.size();
        }
        
        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo& info : dirs) {
            totalSize += getDirectorySize(info.absoluteFilePath());
        }
        
        return totalSize;
    }
    
    int countFilesInDirectory(const QString& path) {
        QDir dir(path);
        int count = 0;
        
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
        count += files.size();
        
        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (const QFileInfo& info : dirs) {
            count += countFilesInDirectory(info.absoluteFilePath());
        }
        
        return count;
    }
    
    QString getLatestModifiedFile(const QString& path) {
        QDir dir(path);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Time);
        
        if (!files.isEmpty()) {
            return files.first().absoluteFilePath();
        }
        
        // Check subdirectories
        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (const QFileInfo& dirInfo : dirs) {
            QString latestInSubdir = getLatestModifiedFile(dirInfo.absoluteFilePath());
            if (!latestInSubdir.isEmpty()) {
                return latestInSubdir;
            }
        }
        
        return QString();
    }
    
    QString formatFileSize(qint64 bytes) {
        if (bytes >= 1024LL * 1024 * 1024) {
            return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
        } else if (bytes >= 1024 * 1024) {
            return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
        } else if (bytes >= 1024) {
            return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 0);
        } else {
            return QString("%1 bytes").arg(bytes);
        }
    }
    
    QString m_pkgPath;
    QString m_outputPath;
    QProcess* m_process;
    QTimer* m_progressTimer = nullptr;
    QTimer* m_directoryTimer = nullptr;
    int m_currentProgress = 0;
    qint64 m_pkgSizeBytes = 0;
    qint64 m_lastOutputSize = 0;
    int m_initialFileCount = 0;
    QString m_lastProcessedFile;
    QDateTime m_startTime;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow() {
        setWindowTitle("ShadPS4Manager - PKG Extractor");
        setMinimumSize(700, 500);
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

    void selectOutputDir() {
        QString dir = QFileDialog::getExistingDirectory(
            this, 
            "Select Output Directory", 
            QDir::homePath()
        );
        
        if (!dir.isEmpty()) {
            outputDir = dir;
            outputLabel->setText("Output: " + dir);
        }
    }

    void startExtraction() {
        if (selectedFile.isEmpty()) {
            QMessageBox::warning(this, "No File", "Please select a PKG file first.");
            return;
        }

        // Use same directory as PKG file if no output directory selected
        if (outputDir.isEmpty()) {
            QFileInfo info(selectedFile);
            outputDir = info.dir().absolutePath() + "/" + info.baseName();
        }

        // Disable UI during extraction
        extractButton->setEnabled(false);
        selectFileButton->setEnabled(false);
        selectOutputButton->setEnabled(false);
        
        // Show progress UI
        progressBar->setVisible(true);
        statusLabel->setVisible(true);
        currentFileLabel->setVisible(true);
        logTextEdit->setVisible(true);
        logTextEdit->clear();
        
        // Create worker and thread
        QThread* thread = new QThread;
        ExtractionWorker* worker = new ExtractionWorker(selectedFile, outputDir);
        worker->moveToThread(thread);

        // Connect signals
        connect(thread, &QThread::started, worker, &ExtractionWorker::startExtraction);
        connect(worker, &ExtractionWorker::progressUpdate, progressBar, &QProgressBar::setValue);
        connect(worker, &ExtractionWorker::statusUpdate, this, &MainWindow::updateStatus);
        connect(worker, &ExtractionWorker::fileUpdate, this, &MainWindow::updateCurrentFile);
        connect(worker, &ExtractionWorker::finished, this, &MainWindow::extractionFinished);
        connect(worker, &ExtractionWorker::finished, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);

        // Start extraction
        thread->start();
        
        logTextEdit->append("Starting PKG extraction...");
        logTextEdit->append("Input: " + selectedFile);
        logTextEdit->append("Output: " + outputDir);
    }

    void updateStatus(const QString& status) {
        statusLabel->setText(status);
        logTextEdit->append("[" + QTime::currentTime().toString("hh:mm:ss") + "] " + status);
    }
    
    void updateCurrentFile(const QString& filename) {
        if (currentFileLabel) {
            currentFileLabel->setText(filename);
        }
        logTextEdit->append("[" + QTime::currentTime().toString("hh:mm:ss") + "] " + filename);
    }
    
    void exitApplication() {
        close();
    }

    void extractionFinished(bool success, const QString& error) {
        // Re-enable UI
        extractButton->setEnabled(true);
        selectFileButton->setEnabled(true);
        selectOutputButton->setEnabled(true);
        
        if (success) {
            logTextEdit->append("✅ Extraction completed successfully!");
            QMessageBox::information(this, "Success", 
                "PKG extraction completed!\n\nFiles extracted to:\n" + outputDir);
        } else {
            logTextEdit->append("❌ Extraction failed: " + error);
            QMessageBox::critical(this, "Extraction Failed", 
                "Failed to extract PKG file:\n\n" + error);
            
            // Reset progress bar on failure
            progressBar->setValue(0);
        }
    }

private:
    void setupUI() {
        auto* layout = new QVBoxLayout(this);
        layout->setSpacing(20);
        layout->setContentsMargins(30, 30, 30, 30);

        // Title
        auto* titleLabel = new QLabel("ShadPS4Manager");
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        // Subtitle
        auto* subtitleLabel = new QLabel("PS4 PKG File Extractor");
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(subtitleLabel);

        // File selection area
        auto* fileFrame = new QWidget;
        fileFrame->setObjectName("fileFrame");
        auto* fileLayout = new QVBoxLayout(fileFrame);
        
        selectFileButton = new QPushButton("📁 Select PKG File");
        selectFileButton->setObjectName("primaryButton");
        connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::selectPkgFile);
        
        fileLabel = new QLabel("No file selected");
        fileLabel->setObjectName("fileLabel");
        fileSizeLabel = new QLabel("");
        fileSizeLabel->setObjectName("fileSizeLabel");
        
        fileLayout->addWidget(selectFileButton);
        fileLayout->addWidget(fileLabel);
        fileLayout->addWidget(fileSizeLabel);
        layout->addWidget(fileFrame);

        // Output directory selection
        auto* outputFrame = new QWidget;
        outputFrame->setObjectName("outputFrame");
        auto* outputLayout = new QVBoxLayout(outputFrame);
        
        selectOutputButton = new QPushButton("📂 Select Output Directory (Optional)");
        selectOutputButton->setObjectName("secondaryButton");
        connect(selectOutputButton, &QPushButton::clicked, this, &MainWindow::selectOutputDir);
        
        outputLabel = new QLabel("Output: Same directory as PKG file");
        outputLabel->setObjectName("outputLabel");
        
        outputLayout->addWidget(selectOutputButton);
        outputLayout->addWidget(outputLabel);
        layout->addWidget(outputFrame);

        // Extract button
        extractButton = new QPushButton("🚀 Extract PKG");
        extractButton->setObjectName("extractButton");
        extractButton->setEnabled(false);
        connect(extractButton, &QPushButton::clicked, this, &MainWindow::startExtraction);
        layout->addWidget(extractButton);

        // Progress area
        progressBar = new QProgressBar;
        progressBar->setObjectName("progressBar");
        progressBar->setVisible(false);
        progressBar->setRange(0, 100);
        progressBar->setFormat("%p% - %v/%m");
        layout->addWidget(progressBar);

        statusLabel = new QLabel;
        statusLabel->setObjectName("statusLabel");
        statusLabel->setVisible(false);
        statusLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(statusLabel);
        
        // Current file being processed
        currentFileLabel = new QLabel;
        currentFileLabel->setObjectName("currentFileLabel");
        currentFileLabel->setVisible(false);
        currentFileLabel->setAlignment(Qt::AlignCenter);
        currentFileLabel->setWordWrap(true);
        layout->addWidget(currentFileLabel);

        // Log area
        logTextEdit = new QTextEdit;
        logTextEdit->setObjectName("logTextEdit");
        logTextEdit->setVisible(false);
        logTextEdit->setMaximumHeight(150);
        layout->addWidget(logTextEdit);

        // Info
        auto* infoLabel = new QLabel("Supports PS4 PKG files. Large files may take several minutes to extract.");
        infoLabel->setObjectName("infoLabel");
        infoLabel->setWordWrap(true);
        infoLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(infoLabel);
        
        // Exit button
        auto* buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();
        
        exitButton = new QPushButton("❌ Exit Application");
        exitButton->setObjectName("exitButton");
        connect(exitButton, &QPushButton::clicked, this, &MainWindow::exitApplication);
        
        buttonLayout->addWidget(exitButton);
        layout->addLayout(buttonLayout);
    }

    void applyStyles() {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', 'Ubuntu', sans-serif;
            }
            
            #titleLabel {
                font-size: 32px;
                font-weight: bold;
                color: #4CAF50;
                margin: 10px 0px;
            }
            
            #subtitleLabel {
                font-size: 16px;
                color: #cccccc;
                margin-bottom: 20px;
            }
            
            #fileFrame, #outputFrame {
                background-color: #3a3a3a;
                border-radius: 8px;
                padding: 15px;
                margin: 5px 0px;
            }
            
            #primaryButton {
                background-color: #4CAF50;
                border: 2px solid #388E3C;
                border-radius: 8px;
                color: white;
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                min-height: 20px;
            }
            
            #primaryButton:hover {
                background-color: #45a049;
            }
            
            #primaryButton:pressed {
                background-color: #2E7D32;
            }
            
            #secondaryButton {
                background-color: #2196F3;
                border: 2px solid #1976D2;
                border-radius: 8px;
                color: white;
                font-size: 14px;
                font-weight: bold;
                padding: 12px;
                min-height: 20px;
            }
            
            #secondaryButton:hover {
                background-color: #1E88E5;
            }
            
            #extractButton {
                background-color: #FF9800;
                border: 2px solid #F57C00;
                border-radius: 8px;
                color: white;
                font-size: 16px;
                font-weight: bold;
                padding: 15px;
                min-height: 25px;
            }
            
            #extractButton:hover {
                background-color: #FB8C00;
            }
            
            #extractButton:disabled {
                background-color: #555555;
                border-color: #444444;
                color: #888888;
            }
            
            #fileLabel, #fileSizeLabel, #outputLabel {
                font-size: 13px;
                color: #ffffff;
                margin: 5px 0px;
            }
            
            #fileSizeLabel {
                color: #4CAF50;
                font-weight: bold;
            }
            
            #statusLabel {
                font-size: 14px;
                color: #4CAF50;
                font-weight: bold;
                margin: 10px 0px;
            }
            
            #progressBar {
                background-color: #404040;
                border: 1px solid #555555;
                border-radius: 4px;
                text-align: center;
                color: white;
                font-weight: bold;
            }
            
            #progressBar::chunk {
                background-color: #4CAF50;
                border-radius: 3px;
            }
            
            #logTextEdit {
                background-color: #1e1e1e;
                border: 1px solid #555555;
                border-radius: 4px;
                color: #ffffff;
                font-family: 'Courier New', monospace;
                font-size: 12px;
            }
            
            #infoLabel {
                font-size: 12px;
                color: #888888;
                font-style: italic;
                margin-top: 10px;
            }
            
            #currentFileLabel {
                font-size: 11px;
                color: #FFC107;
                font-family: 'Courier New', monospace;
                background-color: #3a3a3a;
                border-radius: 4px;
                padding: 8px;
                margin: 5px 0px;
            }
            
            #exitButton {
                background-color: #f44336;
                border: 2px solid #d32f2f;
                border-radius: 6px;
                color: white;
                font-size: 12px;
                font-weight: bold;
                padding: 8px 16px;
                min-width: 120px;
            }
            
            #exitButton:hover {
                background-color: #e53935;
            }
            
            #exitButton:pressed {
                background-color: #c62828;
            }
        )");
    }

    QPushButton *selectFileButton;
    QPushButton *selectOutputButton;
    QPushButton *extractButton;
    QPushButton *exitButton;
    QLabel *fileLabel;
    QLabel *fileSizeLabel;
    QLabel *outputLabel;
    QLabel *statusLabel;
    QLabel *currentFileLabel;
    QProgressBar *progressBar;
    QTextEdit *logTextEdit;
    
    QString selectedFile;
    QString outputDir;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "working_gui.moc"