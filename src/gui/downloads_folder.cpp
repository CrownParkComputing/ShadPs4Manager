#include "downloads_folder.h"
#include "settings.h"
#include "param_sfo.h"
#include "pkg_tool/lib.h"
#include <QHeaderView>
#include <QRegularExpression>
#include <QDebug>
#include <algorithm>
#include <filesystem>
#include <numeric>

DownloadsFolder::DownloadsFolder(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyStyles();
    refreshDownloads();
}

void DownloadsFolder::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* headerLabel = new QLabel("Downloads Folder - Games Grouped");
    headerLabel->setObjectName("downloadsHeader");
    refreshButton = new QPushButton("Refresh");
    installSelectedButton = new QPushButton("Install Selected Game");
    installAllButton = new QPushButton("Install All Games");
    headerLayout->addWidget(headerLabel, 1);
    headerLayout->addWidget(refreshButton);
    headerLayout->addWidget(installSelectedButton);
    headerLayout->addWidget(installAllButton);
    mainLayout->addLayout(headerLayout);

    // Game tree (grouped by game with proper ordering)
    gameTreeWidget = new QTreeWidget();
    gameTreeWidget->setHeaderLabels({"Game / Package", "Type", "Version", "Size"});
    gameTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    gameTreeWidget->setAlternatingRowColors(true);
    
    // Set column widths to show full PKG names
    gameTreeWidget->setColumnWidth(0, 400);  // Game/Package name - wider for full names
    gameTreeWidget->setColumnWidth(1, 100);  // Type
    gameTreeWidget->setColumnWidth(2, 80);   // Version
    gameTreeWidget->setColumnWidth(3, 100);  // Size
    
    // Enable word wrapping and resize modes
    gameTreeWidget->header()->setStretchLastSection(false);
    gameTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    gameTreeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    gameTreeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    gameTreeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    
    mainLayout->addWidget(gameTreeWidget);

    // Status label
    statusLabel = new QLabel("Ready");
    mainLayout->addWidget(statusLabel);

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &DownloadsFolder::refreshDownloads);
    connect(installSelectedButton, &QPushButton::clicked, this, &DownloadsFolder::installGameInOrder);
    connect(installAllButton, &QPushButton::clicked, this, &DownloadsFolder::extractSelectedPkgs);
    connect(gameTreeWidget, &QTreeWidget::itemDoubleClicked, this, &DownloadsFolder::onGameDoubleClicked);
    connect(gameTreeWidget, &QTreeWidget::customContextMenuRequested, this, &DownloadsFolder::onGameRightClicked);
}

void DownloadsFolder::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #downloadsHeader {
            font-size: 16px;
            font-weight: bold;
            color: #2196F3;
            padding: 5px;
        }

        QTreeWidget {
            background-color: #1e1e1e;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
            outline: none;
            alternate-background-color: #2a2a2a;
        }

        QTreeWidget::item {
            padding: 8px;
            border-bottom: 1px solid #444444;
            background-color: #353535;
            margin: 2px;
            border-radius: 3px;
        }

        QTreeWidget::item:selected {
            background-color: #2196F3;
            color: #ffffff;
        }

        QTreeWidget::item:hover {
            background-color: #404040;
        }

        QTreeWidget::branch {
            background-color: #353535;
        }

        QTreeWidget::branch:has-children:!has-siblings:closed,
        QTreeWidget::branch:closed:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/arrow_right.png);
        }

        QTreeWidget::branch:open:has-children:!has-siblings,
        QTreeWidget::branch:open:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/arrow_down.png);
        }

        QListWidget::item:hover {
            background-color: #404040;
        }

        QPushButton {
            background-color: #404040;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
            min-width: 80px;
        }

        QPushButton:hover {
            background-color: #505050;
            border-color: #777777;
        }

        QLabel {
            color: #cccccc;
            font-size: 12px;
            padding: 5px;
        }
    )");
}

void DownloadsFolder::refreshDownloads() {
    Settings& settings = Settings::instance();
    setDownloadsPath(settings.getDownloadsPath());
}

void DownloadsFolder::setDownloadsPath(const QString& path) {
    downloadsPath = path;
    loadPkgs();
}

void DownloadsFolder::loadPkgs() {
    clearPkgs();

    if (downloadsPath.isEmpty()) {
        statusLabel->setText("No downloads path configured");
        return;
    }

    QDir downloadsDir(downloadsPath);
    if (!downloadsDir.exists()) {
        statusLabel->setText("Downloads directory does not exist");
        return;
    }

    // Look for PKG files
    QStringList pkgFiles = downloadsDir.entryList(QStringList{"*.pkg"}, QDir::Files);

    if (pkgFiles.isEmpty()) {
        statusLabel->setText("No PKG files found in downloads");
        return;
    }

    // Clear existing data
    downloads.clear();
    gameGroups.clear();

    // Parse all PKG files
    for (const QString& pkgFile : pkgFiles) {
        QString pkgPath = downloadsDir.absoluteFilePath(pkgFile);
        
        // Validate PKG file before parsing
        QFileInfo pkgFileInfo(pkgPath);
        if (!pkgFileInfo.exists() || pkgFileInfo.size() == 0) {
            qDebug() << "Skipping empty or invalid PKG file:" << pkgPath;
            continue;
        }
        
        if (pkgFileInfo.size() < 1024) { // PKG files should be at least 1KB
            qDebug() << "Skipping too small PKG file:" << pkgPath << "Size:" << pkgFileInfo.size();
            continue;
        }
        
        DownloadInfo pkgInfo = parsePkgInfo(pkgPath);
        downloads.append(pkgInfo);
    }

    // Group games and update the tree view
    groupGamesByTitle();
    updateGameTree();

    statusLabel->setText(QString("Found %1 games with %2 PKG files")
                        .arg(gameGroups.size())
                        .arg(downloads.size()));
}

void DownloadsFolder::clearPkgs() {
    gameTreeWidget->clear();
    downloads.clear();
    gameGroups.clear();
}

DownloadInfo DownloadsFolder::parsePkgInfo(const QString& pkgPath) {
    DownloadInfo info;
    QFileInfo fileInfo(pkgPath);

    info.fileName = fileInfo.fileName();
    info.path = pkgPath;
    info.size = fileInfo.size();

    // Extract metadata from filename patterns (common PS4 PKG naming)
    QString baseName = fileInfo.completeBaseName(); // Use completeBaseName to get full name without extension
    
    // Look for title ID pattern (CUSA12345 or similar)
    QRegularExpression titleIdRegex("(CUSA\\d{5}|PCJS\\d{5}|PCAS\\d{5})");
    QRegularExpressionMatch titleMatch = titleIdRegex.match(baseName);
    if (titleMatch.hasMatch()) {
        info.titleId = titleMatch.captured(1);
    } else {
        info.titleId = "UNKNOWN";
    }

    // Extract version information
    QRegularExpression versionRegex("v?(\\d+\\.\\d+(?:\\.\\d+)?)");
    QRegularExpressionMatch versionMatch = versionRegex.match(baseName);
    if (versionMatch.hasMatch()) {
        info.version = versionMatch.captured(1);
    } else {
        info.version = "1.0";
    }

    // Extract game name (everything before title ID or version)
    info.gameName = extractGameName(pkgPath);
    
    // Detect package type from filename patterns - ORDER MATTERS!
    QString lowerBaseName = baseName.toLower();
    QString originalBaseName = baseName; // Keep original case for better matching
    
    qDebug() << "Analyzing PKG:" << originalBaseName;
    
    // First check for DLC patterns (most specific)
    if (lowerBaseName.contains("dlc") || lowerBaseName.contains("addon") || 
        lowerBaseName.contains("-ac") || lowerBaseName.contains("_ac") ||
        QRegularExpression("dlc\\d+", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch()) {
        info.contentId = info.titleId + "-DLC01";
        info.pkgType = PkgType::DLC;
        info.installOrder = 300; // DLC installs last
        qDebug() << "  -> Detected as DLC";
    } 
    // Then check for update/patch patterns (be more specific about updates)
    else if (originalBaseName.contains("PATCH", Qt::CaseInsensitive) ||
             originalBaseName.contains("UPDATE", Qt::CaseInsensitive) ||
             // Only consider versions > 1.00 as updates, not v1.00 which is usually base
             QRegularExpression("v([2-9]\\d*\\.\\d+|1\\.[1-9]\\d*|1\\.0[1-9])", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch() ||
             // A0101 and higher (A0100 is base, A0101+ are updates)
             QRegularExpression("A0(10[1-9]|1[1-9]\\d|[2-9]\\d\\d)", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch()) {
        info.contentId = info.titleId + "-PATCH";
        info.pkgType = PkgType::Update;
        info.installOrder = 200; // Updates install after base game
        qDebug() << "  -> Detected as Update/Patch";
    } 
    // Base game - everything else (including v1.00, A0100, files with "base" in name)
    else {
        info.contentId = info.titleId + "-APP";
        info.pkgType = PkgType::BaseGame;
        info.installOrder = 100; // Base game installs first
        qDebug() << "  -> Detected as Base Game";
    }

    return info;
}

// New methods for game grouping and ordering
void DownloadsFolder::groupGamesByTitle() {
    gameGroups.clear();
    QMap<QString, GameGroup> groupMap;

    for (const DownloadInfo& pkg : downloads) {
        QString baseTitle = pkg.titleId.left(9); // First 9 chars are the base title
        
        if (!groupMap.contains(baseTitle)) {
            GameGroup group;
            group.titleId = baseTitle;
            group.gameName = pkg.gameName.isEmpty() ? QString("Game %1").arg(baseTitle) : pkg.gameName;
            groupMap[baseTitle] = group;
        }
        
        GameGroup& group = groupMap[baseTitle];
        group.packages.append(pkg);
        
        if (pkg.pkgType == PkgType::BaseGame) {
            group.hasBaseGame = true;
        } else if (pkg.pkgType == PkgType::Update) {
            group.updateCount++;
        } else if (pkg.pkgType == PkgType::DLC) {
            group.dlcCount++;
        }
    }

    // Sort packages within each group by install order
    for (auto& group : groupMap) {
        std::sort(group.packages.begin(), group.packages.end(), 
            [](const DownloadInfo& a, const DownloadInfo& b) {
                return a.installOrder < b.installOrder;
            });
        gameGroups.append(group);
    }
}

void DownloadsFolder::updateGameTree() {
    gameTreeWidget->clear();
    
    for (const GameGroup& group : gameGroups) {
        auto* gameItem = new QTreeWidgetItem(gameTreeWidget);
        gameItem->setText(0, QString("%1 (%2)").arg(group.gameName).arg(group.titleId));
        gameItem->setText(1, QString("%1 packages").arg(group.packages.size()));
        gameItem->setText(2, "");
        
        qint64 totalSize = 0;
        for (const DownloadInfo& pkg : group.packages) {
            totalSize += pkg.size;
        }
        gameItem->setText(3, formatFileSize(totalSize));
        
        // Set different colors based on completeness
        if (group.hasBaseGame) {
            gameItem->setForeground(0, QColor("#4CAF50")); // Green if has base game
        } else {
            gameItem->setForeground(0, QColor("#ff9800")); // Orange if missing base game
        }
        
        // Add package items
        for (const DownloadInfo& pkg : group.packages) {
            auto* pkgItem = new QTreeWidgetItem(gameItem);
            pkgItem->setText(0, pkg.fileName);
            pkgItem->setToolTip(0, QString("Full path: %1").arg(pkg.path)); // Show full path on hover
            
            QString typeStr;
            switch (pkg.pkgType) {
                case PkgType::BaseGame: typeStr = "Base Game"; break;
                case PkgType::Update: typeStr = "Update"; break;
                case PkgType::DLC: typeStr = "DLC"; break;
                default: typeStr = "Unknown"; break;
            }
            pkgItem->setText(1, typeStr);
            pkgItem->setText(2, pkg.version);
            pkgItem->setText(3, formatFileSize(pkg.size));
            pkgItem->setData(0, Qt::UserRole, pkg.path);
            
            // Color code by type
            if (pkg.pkgType == PkgType::BaseGame) {
                pkgItem->setForeground(1, QColor("#4CAF50"));
            } else if (pkg.pkgType == PkgType::Update) {
                pkgItem->setForeground(1, QColor("#2196F3"));
            } else if (pkg.pkgType == PkgType::DLC) {
                pkgItem->setForeground(1, QColor("#9C27B0"));
            }
        }
        
        gameItem->setExpanded(true);
    }
}

PkgType DownloadsFolder::detectPkgType(const QString& contentId, const QString& titleId) {
    // PS4 content ID format analysis
    if (contentId.contains("-app", Qt::CaseInsensitive)) {
        return PkgType::BaseGame;
    } else if (contentId.contains("-patch", Qt::CaseInsensitive) || 
               contentId.contains("_00")) {
        return PkgType::Update;
    } else if (contentId.contains("-dlc", Qt::CaseInsensitive) ||
               contentId.contains("-ac", Qt::CaseInsensitive)) {
        return PkgType::DLC;
    }
    
    return PkgType::Unknown;
}

QString DownloadsFolder::extractGameName(const QString& pkgPath) {
    // Try to extract game name from param.sfo if possible
    // For now, use filename as fallback
    QFileInfo info(pkgPath);
    QString baseName = info.baseName();
    
    // Clean up common patterns in PKG names
    baseName.replace(QRegularExpression("[-_]CUSA\\d+.*"), "");
    baseName.replace(QRegularExpression("[-_]v\\d+.*"), "");
    baseName.replace(QRegularExpression("[-_]\\d+\\.\\d+.*"), "");
    
    return baseName;
}

QString DownloadsFolder::formatFileSize(qint64 size) {
    if (size < 1024) return QString("%1 B").arg(size);
    if (size < 1024 * 1024) return QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
    if (size < 1024 * 1024 * 1024) return QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

bool DownloadsFolder::checkBatchDiskSpace(const QList<DownloadInfo>& packages, const QString& outputBasePath, QString& errorMessage) {
    try {
        std::filesystem::path basePath(outputBasePath.toStdString());
        
        // Get parent directory if the path doesn't exist yet
        while (!std::filesystem::exists(basePath) && basePath.has_parent_path()) {
            basePath = basePath.parent_path();
        }
        
        if (!std::filesystem::exists(basePath)) {
            errorMessage = "Target directory path is invalid or inaccessible.";
            return false;
        }
        
        std::filesystem::space_info spaceInfo = std::filesystem::space(basePath);
        uint64_t availableSpace = spaceInfo.available;
        
        // Calculate total size needed for all packages
        uint64_t totalSize = 0;
        for (const DownloadInfo& pkg : packages) {
            totalSize += static_cast<uint64_t>(pkg.size);
        }
        
        // Add 30% buffer for batch extraction overhead and temporary files
        uint64_t requiredWithBuffer = totalSize + (totalSize * 30 / 100);
        
        if (availableSpace < requiredWithBuffer) {
            errorMessage = QString("Insufficient disk space for batch installation!\n\n"
                "Required: %1 (+ 30% buffer)\n"
                "Available: %2\n"
                "Shortage: %3\n\n"
                "Number of packages: %4")
                .arg(formatBytes(requiredWithBuffer))
                .arg(formatBytes(availableSpace))
                .arg(formatBytes(requiredWithBuffer - availableSpace))
                .arg(packages.size());
            return false;
        }
        
        // Warn if less than 2GB free space will remain after batch installation
        uint64_t remainingAfter = availableSpace - requiredWithBuffer;
        if (remainingAfter < (2ULL * 1024 * 1024 * 1024)) {
            errorMessage = QString("Warning: Very low disk space after batch installation!\n\n"
                "Required: %1\n"
                "Available: %2\n"
                "Remaining after extraction: %3\n"
                "Number of packages: %4\n\n"
                "Continue anyway?")
                .arg(formatBytes(requiredWithBuffer))
                .arg(formatBytes(availableSpace))
                .arg(formatBytes(remainingAfter))
                .arg(packages.size());
            return false; // Will be handled as warning, not error
        }
        
        return true;
        
    } catch (const std::exception& e) {
        errorMessage = QString("Error checking disk space: %1").arg(QString::fromStdString(e.what()));
        return false;
    }
}

QString DownloadsFolder::formatBytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', (unitIndex > 0) ? 2 : 0).arg(units[unitIndex]);
}

void DownloadsFolder::onGameDoubleClicked(QTreeWidgetItem* item, int column) {
    if (!item) return;
    
    // If it's a package item (has parent), install just that package
    if (item->parent()) {
        QString pkgPath = item->data(0, Qt::UserRole).toString();
        if (!pkgPath.isEmpty()) {
            extractGame();
        }
    } else {
        // If it's a game group, install in order
        installGameInOrder();
    }
}

void DownloadsFolder::onGameRightClicked(const QPoint& pos) {
    QTreeWidgetItem* item = gameTreeWidget->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);
    
    if (item->parent()) {
        // Package item
        contextMenu.addAction("Extract This Package", this, &DownloadsFolder::extractGame);
        contextMenu.addAction("Show Package Info", this, &DownloadsFolder::showGameInfo);
    } else {
        // Game group
        contextMenu.addAction("Install Game (In Order)", this, &DownloadsFolder::installGameInOrder);
        contextMenu.addAction("Show Game Info", this, &DownloadsFolder::showGameInfo);
    }

    contextMenu.exec(gameTreeWidget->mapToGlobal(pos));
}

void DownloadsFolder::extractGame() {
    QTreeWidgetItem* item = gameTreeWidget->currentItem();
    if (!item) return;
    
    QString pkgPath;
    if (item->parent()) {
        // Package item
        pkgPath = item->data(0, Qt::UserRole).toString();
    } else {
        // Game group - extract first package (should be base game)
        if (item->childCount() > 0) {
            pkgPath = item->child(0)->data(0, Qt::UserRole).toString();
        }
    }
    
    if (pkgPath.isEmpty()) return;

    QString gameLibraryPath = Settings::instance().getGameLibraryPath();
    if (gameLibraryPath.isEmpty()) {
        QMessageBox::warning(this, "No Library Path",
            "Please configure the Game Library path in Settings first.");
        return;
    }

    QString outputDir = gameLibraryPath + "/" + getProperDirectoryName(pkgPath);

    emit extractionRequested(pkgPath, outputDir);
}

void DownloadsFolder::installGameInOrder() {
    QTreeWidgetItem* item = gameTreeWidget->currentItem();
    if (!item) return;
    
    // Find the game group
    QTreeWidgetItem* gameItem = item->parent() ? item->parent() : item;
    
    QString gameLibraryPath = Settings::instance().getGameLibraryPath();
    if (gameLibraryPath.isEmpty()) {
        QMessageBox::warning(this, "No Library Path",
            "Please configure the Game Library path in Settings first.");
        return;
    }
    
    // Find the corresponding game group for space checking
    GameGroup* targetGroup = nullptr;
    QString gameName = gameItem->text(0);
    for (GameGroup& group : gameGroups) {
        if (group.gameName == gameName) {
            targetGroup = &group;
            break;
        }
    }
    
    if (targetGroup) {
        // Check disk space before starting installation
        QString spaceError;
        bool hasSpace = checkBatchDiskSpace(targetGroup->packages, gameLibraryPath, spaceError);
        
        if (!hasSpace) {
            if (spaceError.contains("Warning: Very low disk space")) {
                // Show warning and ask user if they want to continue
                QMessageBox::StandardButton reply = QMessageBox::warning(this, 
                    "Low Disk Space Warning", 
                    spaceError,
                    QMessageBox::Yes | QMessageBox::No);
                
                if (reply != QMessageBox::Yes) {
                    return; // User chose not to continue
                }
            } else {
                // Critical space error - don't allow installation
                QMessageBox::critical(this, "Insufficient Disk Space", spaceError);
                return;
            }
        }
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Install Game in Order",
        QString("Install all packages for '%1' in the correct order?\n\n"
                "This will install: Base Game → Updates → DLC")
                .arg(gameItem->text(0)),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Extract packages in order
        for (int i = 0; i < gameItem->childCount(); ++i) {
            QTreeWidgetItem* pkgItem = gameItem->child(i);
            QString pkgPath = pkgItem->data(0, Qt::UserRole).toString();
            
            QString outputDir = gameLibraryPath + "/" + getProperDirectoryName(pkgPath);
            
            emit extractionRequested(pkgPath, outputDir);
        }
    }
}

void DownloadsFolder::extractSelectedPkgs() {
    if (gameGroups.isEmpty()) {
        QMessageBox::information(this, "No Games", "No games found to install.");
        return;
    }

    QString gameLibraryPath = Settings::instance().getGameLibraryPath();
    if (gameLibraryPath.isEmpty()) {
        QMessageBox::warning(this, "No Library Path",
            "Please configure the Game Library path in Settings first.");
        return;
    }

    // Collect all packages for space checking
    QList<DownloadInfo> allPackages;
    for (const GameGroup& group : gameGroups) {
        allPackages.append(group.packages);
    }

    // Check disk space before starting batch installation
    QString spaceError;
    bool hasSpace = checkBatchDiskSpace(allPackages, gameLibraryPath, spaceError);
    
    if (!hasSpace) {
        if (spaceError.contains("Warning: Very low disk space")) {
            // Show warning and ask user if they want to continue
            QMessageBox::StandardButton reply = QMessageBox::warning(this, 
                "Low Disk Space Warning", 
                spaceError,
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply != QMessageBox::Yes) {
                return; // User chose not to continue
            }
        } else {
            // Critical space error - don't allow installation
            QMessageBox::critical(this, "Insufficient Disk Space", spaceError);
            return;
        }
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Install All Games",
        QString("Install all %1 games in the correct order?\n\nTotal size: %2")
            .arg(gameGroups.size())
            .arg(formatBytes(std::accumulate(allPackages.begin(), allPackages.end(), 0ULL, 
                [](uint64_t sum, const DownloadInfo& pkg) { return sum + static_cast<uint64_t>(pkg.size); }))),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (const GameGroup& group : gameGroups) {
            for (const DownloadInfo& pkg : group.packages) {
                QString outputDir = gameLibraryPath + "/" + getProperDirectoryName(pkg.path);
                emit extractionRequested(pkg.path, outputDir);
            }
        }
    }
}

void DownloadsFolder::showGameInfo() {
    QTreeWidgetItem* item = gameTreeWidget->currentItem();
    if (!item) return;
    
    QString info;
    if (item->parent()) {
        // Package info
        QString pkgPath = item->data(0, Qt::UserRole).toString();
        for (const DownloadInfo& pkg : downloads) {
            if (pkg.path == pkgPath) {
                info = QString("Package: %1\nType: %2\nVersion: %3\nSize: %4\nTitle ID: %5\nContent ID: %6")
                    .arg(pkg.fileName)
                    .arg(pkg.pkgType == PkgType::BaseGame ? "Base Game" : 
                         pkg.pkgType == PkgType::Update ? "Update" : 
                         pkg.pkgType == PkgType::DLC ? "DLC" : "Unknown")
                    .arg(pkg.version)
                    .arg(formatFileSize(pkg.size))
                    .arg(pkg.titleId)
                    .arg(pkg.contentId);
                break;
            }
        }
    } else {
        // Game group info
        for (const GameGroup& group : gameGroups) {
            if (item->text(0).contains(group.titleId)) {
                info = QString("Game: %1\nTitle ID: %2\nTotal Packages: %3\n- Base Game: %4\n- Updates: %5\n- DLC: %6")
                    .arg(group.gameName)
                    .arg(group.titleId)
                    .arg(group.packages.size())
                    .arg(group.hasBaseGame ? "Yes" : "No")
                    .arg(group.updateCount)
                    .arg(group.dlcCount);
                break;
            }
        }
    }
    
    QMessageBox::information(this, "Game/Package Information", info);
}

QString DownloadsFolder::getProperDirectoryName(const QString& pkgPath) {
    // Try to read PKG metadata to get the proper Title ID
    PkgMetadata metadata;
    std::filesystem::path pkgPathFs = std::filesystem::path(pkgPath.toStdString());
    
    auto metadataResult = ReadPkgMetadata(pkgPathFs, metadata);
    if (!metadataResult && !metadata.title_id.empty()) {
        // Successfully read metadata, use the Title ID (e.g., "CUSA00093")
        QString titleId = QString::fromStdString(metadata.title_id);
        if (titleId.length() >= 9) {
            return titleId.left(9); // Use first 9 characters (e.g., "CUSA00093")
        }
    }
    
    // Fallback: use filename-based extraction if metadata reading fails
    QFileInfo pkgInfo(pkgPath);
    QString baseName = pkgInfo.baseName();
    
    // Try to extract Title ID from filename (e.g., "CUSA00093" from filename)
    QRegularExpression titleIdRegex("(CUSA\\d{5}|PCJS\\d{5}|PCAS\\d{5})");
    QRegularExpressionMatch match = titleIdRegex.match(baseName);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    // Final fallback: use cleaned filename
    return baseName + "_extracted";
}
