#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QProgressDialog>
#include <QThread>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <filesystem>

// Include new components
#include "settings.h"
#include "settings_page.h"
#include "game_library.h"
#include "downloads_folder.h"
#include "pkg_tool/lib.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        applyStyles();
        connectSignals();
        
        // Initialize settings
        Settings& settings = Settings::instance();
        
        // Set window properties
        setWindowTitle("ShadPs4 Manager - Game Library");
        setMinimumSize(1000, 700);
        resize(1200, 800);
    }

private slots:
    void refreshAllData() {
        gameLibrary->refreshLibrary();
        // Downloads folder will be refreshed when dialog is opened
        // This prevents unnecessary refreshes when dialog is not visible
    }

    void openSettings() {
        // Create settings dialog
        QDialog* settingsDialog = new QDialog(this);
        settingsDialog->setWindowTitle("Settings");
        settingsDialog->setModal(true);
        settingsDialog->resize(600, 500);
        
        auto* dialogLayout = new QVBoxLayout(settingsDialog);
        
        // Create new settings page for the dialog
        auto* settingsPageDialog = new SettingsPage();
        dialogLayout->addWidget(settingsPageDialog);
        
        // Add close button
        auto* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        auto* closeButton = new QPushButton("Close");
        closeButton->setObjectName("closeButton");
        connect(closeButton, &QPushButton::clicked, settingsDialog, &QDialog::accept);
        buttonLayout->addWidget(closeButton);
        dialogLayout->addLayout(buttonLayout);
        
        // Connect settings changes
        connect(settingsPageDialog, &SettingsPage::settingsChanged, this, &MainWindow::onSettingsChanged);
        
        settingsDialog->exec();
        settingsDialog->deleteLater();
    }

    void openDownloads() {
        // Create downloads dialog  
        QDialog* downloadsDialog = new QDialog(this);
        downloadsDialog->setWindowTitle("Downloads");
        downloadsDialog->setModal(true);
        downloadsDialog->resize(800, 600);
        
        auto* dialogLayout = new QVBoxLayout(downloadsDialog);
        
        // Create new downloads page for the dialog
        auto* downloadsPageDialog = new DownloadsFolder();
        dialogLayout->addWidget(downloadsPageDialog);
        
        // Connect extraction signal from this dialog instance
        connect(downloadsPageDialog, &DownloadsFolder::extractionRequested, this, [this](const QString& pkgPath, const QString& outputPath) {
            extractPkgFile(pkgPath, outputPath);
        });
        
        // Add close button
        auto* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        auto* closeButton = new QPushButton("Close");
        closeButton->setObjectName("closeButton");
        connect(closeButton, &QPushButton::clicked, downloadsDialog, &QDialog::accept);
        buttonLayout->addWidget(closeButton);
        dialogLayout->addLayout(buttonLayout);
        
        downloadsDialog->exec();
        downloadsDialog->deleteLater();
    }

    void onSettingsChanged() {
        // Refresh library when settings change (like library path)
        gameLibrary->refreshLibrary();
    }

private:
    void setupUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // Header with menu buttons
        auto* headerLayout = new QHBoxLayout();
        auto* titleLabel = new QLabel("ShadPs4 Manager");
        titleLabel->setObjectName("mainTitle");
        
        // Menu buttons
        auto* menuLayout = new QHBoxLayout();
        
        auto* downloadsButton = new QPushButton("Downloads");
        downloadsButton->setObjectName("menuButton");
        downloadsButton->setToolTip("Open Downloads folder manager");
        
        auto* settingsButton = new QPushButton("Settings");
        settingsButton->setObjectName("menuButton");
        settingsButton->setToolTip("Open Settings");
        
        auto* refreshButton = new QPushButton("Refresh");
        refreshButton->setObjectName("refreshButton");
        refreshButton->setToolTip("Refresh game library");
        
        menuLayout->addWidget(downloadsButton);
        menuLayout->addWidget(settingsButton);
        menuLayout->addWidget(refreshButton);
        
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();
        headerLayout->addLayout(menuLayout);
        
        mainLayout->addLayout(headerLayout);

        // Main content - Game Library (no tabs)
        gameLibrary = new GameLibrary();
        mainLayout->addWidget(gameLibrary);
        
        // Connect buttons
        connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAllData);
        connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
        connect(downloadsButton, &QPushButton::clicked, this, &MainWindow::openDownloads);
    }

    void applyStyles() {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
            }

            #mainTitle {
                font-size: 24px;
                font-weight: bold;
                color: #4CAF50;
                padding: 10px;
            }

            #refreshButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
            }

            #refreshButton:hover {
                background-color: #45a049;
            }

            #refreshButton:pressed {
                background-color: #3d8b40;
            }

            #menuButton {
                background-color: #2196F3;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
                margin-left: 5px;
            }

            #menuButton:hover {
                background-color: #0b7dda;
            }

            #menuButton:pressed {
                background-color: #0969da;
            }

            #closeButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
            }

            #closeButton:hover {
                background-color: #45a049;
            }
        )");
    }

    void connectSignals() {
        // Settings changes will be connected per-dialog in openSettings()
    }

    // Helper function to check available disk space
    bool checkDiskSpace(const QString& outputPath, uint64_t requiredSize, QString& errorMessage) {
        try {
            std::filesystem::path path(outputPath.toStdString());
            
            // Get parent directory if the path doesn't exist yet
            while (!std::filesystem::exists(path) && path.has_parent_path()) {
                path = path.parent_path();
            }
            
            if (!std::filesystem::exists(path)) {
                errorMessage = "Target directory path is invalid or inaccessible.";
                return false;
            }
            
            std::filesystem::space_info spaceInfo = std::filesystem::space(path);
            uint64_t availableSpace = spaceInfo.available;
            
            // Add 20% buffer for extraction overhead and temporary files
            uint64_t requiredWithBuffer = requiredSize + (requiredSize / 5);
            
            if (availableSpace < requiredWithBuffer) {
                errorMessage = QString("Insufficient disk space!\n\n"
                    "Required: %1 (+ 20% buffer)\n"
                    "Available: %2\n"
                    "Shortage: %3")
                    .arg(formatBytes(requiredWithBuffer))
                    .arg(formatBytes(availableSpace))
                    .arg(formatBytes(requiredWithBuffer - availableSpace));
                return false;
            }
            
            // Warn if less than 1GB free space will remain
            uint64_t remainingAfter = availableSpace - requiredWithBuffer;
            if (remainingAfter < (1024ULL * 1024 * 1024)) {
                errorMessage = QString("Warning: Low disk space after extraction!\n\n"
                    "Required: %1\n"
                    "Available: %2\n"
                    "Remaining after extraction: %3\n\n"
                    "Continue anyway?")
                    .arg(formatBytes(requiredWithBuffer))
                    .arg(formatBytes(availableSpace))
                    .arg(formatBytes(remainingAfter));
                return false; // Will be handled as warning, not error
            }
            
            return true;
            
        } catch (const std::exception& e) {
            errorMessage = QString("Error checking disk space: %1").arg(QString::fromStdString(e.what()));
            return false;
        }
    }
    
    // Helper function to format bytes for display
    QString formatBytes(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }
        
        return QString("%1 %2").arg(size, 0, 'f', (unitIndex > 0) ? 2 : 0).arg(units[unitIndex]);
    }

    void extractPkgFile(const QString& pkgPath, const QString& outputPath) {
        try {
            // Validate PKG file exists and has content
            QFileInfo pkgFileInfo(pkgPath);
            if (!pkgFileInfo.exists()) {
                QMessageBox::critical(this, "Extraction Error", 
                    QString("PKG file not found: %1").arg(pkgPath));
                return;
            }
            
            if (pkgFileInfo.size() == 0) {
                QMessageBox::critical(this, "Extraction Error", 
                    QString("PKG file is empty: %1").arg(pkgPath));
                return;
            }
            
            if (pkgFileInfo.size() < 1024) { // PKG files should be at least 1KB
                QMessageBox::critical(this, "Extraction Error", 
                    QString("PKG file is too small to be valid: %1\nSize: %2 bytes").arg(pkgPath).arg(pkgFileInfo.size()));
                return;
            }

            // Convert Qt strings to filesystem paths
            std::filesystem::path pkgPathFs = std::filesystem::path(pkgPath.toStdString());
            std::filesystem::path outputPathFs = std::filesystem::path(outputPath.toStdString());
            
            // First read metadata to get file count and size
            PkgMetadata metadata;
            auto metadataResult = ReadPkgMetadata(pkgPathFs, metadata);
            if (metadataResult) {
                QMessageBox::critical(this, "Extraction Error", 
                    QString("Failed to read PKG metadata: %1\n\nFile: %2\nSize: %3 bytes")
                    .arg(QString::fromStdString(*metadataResult))
                    .arg(pkgPath)
                    .arg(pkgFileInfo.size()));
                return;
            }

            // Validate metadata makes sense - only check PKG size, not file count
            // Note: Some PKG files report file_count as 0 but still contain extractable PFS files
            if (metadata.pkg_size == 0) {
                QMessageBox::critical(this, "Extraction Error", 
                    QString("PKG file appears to be invalid or corrupted.\n\nFile: %1\nPKG Size: %2\nFile Count: %3")
                    .arg(pkgPath)
                    .arg(metadata.pkg_size)
                    .arg(metadata.file_count));
                return;
            }

            // Check disk space before starting extraction
            QString spaceError;
            bool hasSpace = checkDiskSpace(outputPath, metadata.pkg_size, spaceError);
            
            if (!hasSpace) {
                if (spaceError.contains("Warning: Low disk space")) {
                    // Show warning and ask user if they want to continue
                    QMessageBox::StandardButton reply = QMessageBox::warning(this, 
                        "Low Disk Space Warning", 
                        spaceError,
                        QMessageBox::Yes | QMessageBox::No);
                    
                    if (reply != QMessageBox::Yes) {
                        return; // User chose not to continue
                    }
                } else {
                    // Critical space error - don't allow extraction
                    QMessageBox::critical(this, "Insufficient Disk Space", spaceError);
                    return;
                }
            }

            QProgressDialog* progressDialog = new QProgressDialog("Extracting PKG file...", "Cancel", 0, 100, this);
            progressDialog->setWindowModality(Qt::WindowModal);
            progressDialog->setMinimumDuration(0);
            progressDialog->show();

            // Set up progress callback with safety checks
            ProgressCallback progressCallback = [progressDialog](const ExtractionProgress& progress) {
                try {
                    if (!progressDialog || progressDialog->wasCanceled()) {
                        return; // Note: Can't cancel extraction once started
                    }
                    
                    // Clamp percentage to valid range
                    int percentage = static_cast<int>(std::min(100.0, std::max(0.0, progress.total_progress * 100)));
                    progressDialog->setValue(percentage);
                    
                    // Safety check for string conversion
                    QString fileName = "Unknown";
                    if (!progress.current_file.empty()) {
                        try {
                            fileName = QString::fromStdString(progress.current_file);
                        } catch (...) {
                            fileName = "Invalid filename";
                        }
                    }
                    
                    progressDialog->setLabelText(QString("Extracting: %1 (%2/%3)")
                        .arg(fileName)
                        .arg(progress.current_file_index + 1)
                        .arg(progress.total_files));
                    
                    QApplication::processEvents(); // Keep UI responsive
                } catch (const std::exception& e) {
                    // Don't let progress callback exceptions crash the app
                    qDebug() << "Progress callback error:" << e.what();
                } catch (...) {
                    // Catch all other exceptions
                    qDebug() << "Unknown progress callback error";
                }
            };

            // Extract all files (empty indices vector means extract all)
            std::vector<int> indices; // Empty = extract all files
            auto extractResult = ExtractPkg(pkgPathFs, outputPathFs, indices, progressCallback);
            
            progressDialog->close();
            progressDialog->deleteLater();
            
            if (extractResult) {
                QMessageBox::critical(this, "Extraction Failed", 
                    QString("Failed to extract PKG file: %1").arg(QString::fromStdString(*extractResult)));
            } else {
                QMessageBox::information(this, "Extraction Complete", 
                    QString("PKG file extracted successfully to:\n%1\n\nTitle ID: %2\nFiles extracted: %3\nPKG Size: %4")
                    .arg(outputPath)
                    .arg(QString::fromStdString(metadata.title_id))
                    .arg(metadata.file_count)
                    .arg(formatBytes(metadata.pkg_size)));
                
                // Add confirmation before continuing
                int result = QMessageBox::question(this, "Continue?", 
                    QString("PKG extraction completed for %1.\n\nPress 'Yes' to refresh game library and continue, or 'No' to skip refresh.")
                    .arg(QString::fromStdString(metadata.title_id)),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                
                if (result == QMessageBox::Yes) {
                    // Refresh the game library to show the new game
                    gameLibrary->refreshLibrary();
                    
                    QMessageBox::information(this, "Refresh Complete", 
                        "Game library refresh completed. Ready for next operation.");
                }
            }
            
        } catch (const std::bad_alloc& e) {
            QMessageBox::critical(this, "Memory Error", 
                QString("Out of memory during PKG extraction: %1").arg(QString::fromStdString(e.what())));
        } catch (const std::filesystem::filesystem_error& e) {
            QMessageBox::critical(this, "File System Error", 
                QString("File system error during PKG extraction: %1").arg(QString::fromStdString(e.what())));
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Extraction Error", 
                QString("Error extracting PKG file: %1").arg(QString::fromStdString(e.what())));
        } catch (...) {
            QMessageBox::critical(this, "Unknown Error", 
                "An unknown error occurred during PKG extraction. The PKG file may be corrupted or invalid.");
        }
    }

private:
    GameLibrary* gameLibrary;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("ShadPs4 Manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ShadPs4");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "working_gui.moc"