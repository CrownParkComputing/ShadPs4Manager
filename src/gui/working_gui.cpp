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
#include <QPointer>
#include <QThread>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <filesystem>
#include <algorithm>

// Include new components
#include "settings.h"
#include "settings_page.h"
#include "game_library.h"
#include "downloads_folder.h"
#include "pkg_tool/lib.h"
#include "core/file_format/pkg.h"
#include "core/update_merger.h"

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

            // Determine if this is an update package by analyzing the filename
            bool isUpdate = detectIfUpdate(pkgPath);

            // Convert Qt strings to filesystem paths
            std::filesystem::path pkgPathFs = std::filesystem::path(pkgPath.toStdString());
            std::filesystem::path outputPathFs = std::filesystem::path(outputPath.toStdString());
            
            // Read metadata via PKG class
            PKG pkg;
            std::string failReason;
            if (!pkg.Open(pkgPathFs, failReason)) {
                QMessageBox::critical(this, "Extraction Error",
                    QString("Failed to open PKG: %1\n\nFile: %2").arg(QString::fromStdString(failReason)).arg(pkgPath));
                return;
            }
            const PKGMeta metadata = pkg.GetMetadata();

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

            // Clean up any incomplete/partial extraction first
            QDir outputDir(outputPath);
            if (outputDir.exists()) {
                // Check if it's a complete extraction (has param.sfo)
                bool hasParamSfo = false;
                QDirIterator it(outputPath, QStringList{"param.sfo"}, QDir::Files, QDirIterator::Subdirectories);
                if (it.hasNext()) {
                    hasParamSfo = true;
                }
                
                // If no param.sfo found, this is likely incomplete - remove it
                if (!hasParamSfo) {
                    QMessageBox::StandardButton reply = QMessageBox::question(this,
                        "Incomplete Extraction Detected",
                        QString("The output directory already exists but appears incomplete:\n%1\n\nWould you like to remove it and start fresh?").arg(outputPath),
                        QMessageBox::Yes | QMessageBox::No);
                    
                    if (reply == QMessageBox::Yes) {
                        if (!outputDir.removeRecursively()) {
                            QMessageBox::critical(this, "Cleanup Failed",
                                QString("Failed to remove incomplete extraction directory:\n%1").arg(outputPath));
                            return;
                        }
                        // Recreate the directory
                        outputDir.mkpath(".");
                    } else {
                        return; // User chose not to proceed
                    }
                }
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

            // Use QPointer for safe access in lambda
            QPointer<QProgressDialog> dialogPtr(progressDialog);

            // Wire PKG progress to dialog
            pkg.SetProgressCallback([this, dialogPtr](const PKGProgress& pr){
                try {
                    if (!dialogPtr) return; // Check if dialog still exists
                    
                    // Update progress bar
                    int percentage = static_cast<int>(std::clamp(pr.percent, 0.0, 100.0));
                    dialogPtr->setValue(percentage);
                    
                    // Build stage-specific label
                    QString stageText;
                    switch (pr.stage) {
                        case PKGProgress::Stage::Opening:
                            stageText = "Opening PKG file...";
                            break;
                        case PKGProgress::Stage::ReadingMetadata:
                            stageText = "Reading PKG metadata...";
                            break;
                        case PKGProgress::Stage::ParsingPFS:
                            stageText = "Parsing PFS structure...";
                            break;
                        case PKGProgress::Stage::Extracting:
                            stageText = "Extracting files...";
                            break;
                        case PKGProgress::Stage::Done:
                            stageText = "Extraction complete!";
                            break;
                        case PKGProgress::Stage::Error:
                            stageText = "Error during extraction";
                            break;
                    }
                    
                    // Build detailed progress label
                    QString label;
                    if (pr.stage == PKGProgress::Stage::Extracting && pr.files_total > 0) {
                        QString currentFile = pr.current_file.empty() ? QString("...") : QString::fromStdString(pr.current_file);
                        label = QString("%1\n\nFile: %2\nProgress: %3 / %4 files (%5%)\nData: %6 / %7")
                            .arg(stageText)
                            .arg(currentFile)
                            .arg(pr.files_done)
                            .arg(pr.files_total)
                            .arg(percentage)
                            .arg(formatBytes(pr.bytes_done))
                            .arg(formatBytes(pr.bytes_total));
                    } else if (!pr.message.empty()) {
                        label = QString("%1\n\n%2")
                            .arg(stageText)
                            .arg(QString::fromStdString(pr.message));
                    } else {
                        label = stageText;
                    }
                    
                    dialogPtr->setLabelText(label);
                    QApplication::processEvents();
                } catch (...) {
                    // Swallow any UI update exception
                }
            });
            
            QString actualExtractionPath = outputPath;
            QString tempUpdatePath;
            
            // If this is an update, extract to temporary directory first
            if (isUpdate) {
                // Create a simpler temp path without special characters
                QString safeTitleId = QString::fromStdString(metadata.title_id);
                safeTitleId = safeTitleId.replace(QRegularExpression("[^A-Za-z0-9]"), "_");
                
                tempUpdatePath = QDir::temp().absoluteFilePath(
                    QString("shadps4_update_%1_%2")
                    .arg(safeTitleId)
                    .arg(QDateTime::currentMSecsSinceEpoch())
                );
                
                // Ensure the temp directory exists and is writable
                QDir tempDir(tempUpdatePath);
                if (!tempDir.mkpath(".")) {
                    QMessageBox::critical(this, "Extraction Error", 
                        QString("Failed to create temporary directory: %1").arg(tempUpdatePath));
                    return;
                }
                
                actualExtractionPath = tempUpdatePath;
                progressDialog->setLabelText("Extracting update to temporary location...");
            }
            
            bool extractSuccess = pkg.Extract(pkgPathFs, std::filesystem::path(actualExtractionPath.toStdString()), failReason);
            
            // If Extract succeeded, now extract all individual files
            if (extractSuccess) {
                uint32_t totalFiles = pkg.GetNumberOfFiles();
                for (uint32_t i = 0; i < totalFiles; ++i) {
                    try {
                        pkg.ExtractFiles(static_cast<int>(i));
                    } catch (const std::exception& e) {
                        failReason = std::string("Failed to extract file index ") + std::to_string(i) + ": " + e.what();
                        extractSuccess = false;
                        break;
                    } catch (...) {
                        failReason = std::string("Unknown error extracting file index ") + std::to_string(i);
                        extractSuccess = false;
                        break;
                    }
                    
                    // Dialog might be closed by user
                    if (!dialogPtr) {
                        extractSuccess = false;
                        failReason = "Extraction cancelled by user";
                        break;
                    }
                }
            }
            
            // Handle update merging if this was an update
            if (extractSuccess && isUpdate) {
                progressDialog->setLabelText("Merging update files...");
                progressDialog->setValue(95);
                QApplication::processEvents();
                
                auto mergeResult = UpdateMerger::mergeUpdateToBaseGame(tempUpdatePath.toStdString(), outputPath.toStdString(), true);
                if (!mergeResult.success) {
                    // Clean up temp directory on failure
                    QDir(tempUpdatePath).removeRecursively();
                    progressDialog->close();
                    progressDialog->deleteLater();
                    QMessageBox::critical(this, "Update Merge Failed", 
                        QString("Failed to merge update: %1").arg(QString::fromStdString(mergeResult.errorMessage)));
                    return;
                }
                
                // Update progress to show merge completion
                progressDialog->setValue(100);
                QApplication::processEvents();
            }
            
            // Clear callback before deleting dialog to prevent use-after-free
            pkg.SetProgressCallback(nullptr);
            
            progressDialog->close();
            progressDialog->deleteLater();
            
            if (!extractSuccess) {
                // Clean up temp directory if extraction failed
                if (isUpdate && !tempUpdatePath.isEmpty()) {
                    QDir(tempUpdatePath).removeRecursively();
                }
                QMessageBox::critical(this, "Extraction Failed", 
                    QString("Failed to extract PKG file: %1").arg(QString::fromStdString(failReason)));
            } else {
                // Auto-refresh the game library without confirmation
                gameLibrary->refreshLibrary();
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

    bool detectIfUpdate(const QString& pkgPath) {
        QFileInfo fileInfo(pkgPath);
        QString baseName = fileInfo.completeBaseName();
        QString originalBaseName = baseName; // Keep original case for better matching
        
        // Check for update/patch patterns (same logic as in downloads_folder.cpp)
        if (originalBaseName.contains("PATCH", Qt::CaseInsensitive) ||
            originalBaseName.contains("UPDATE", Qt::CaseInsensitive) ||
            // Only consider versions > 1.00 as updates, not v1.00 which is usually base
            QRegularExpression("v([2-9]\\d*\\.\\d+|1\\.[1-9]\\d*|1\\.0[1-9])", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch() ||
            // A0101 and higher (A0100 is base, A0101+ are updates)
            QRegularExpression("A0(10[1-9]|1[1-9]\\d|[2-9]\\d\\d)", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch()) {
            return true;
        }
        
        return false;
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