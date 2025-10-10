#include "installation_folder.h"
#include "settings.h"
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>

InstallationFolder::InstallationFolder(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyStyles();
    refreshView();
}

void InstallationFolder::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* headerLabel = new QLabel("📁 Installed Games & Content");
    headerLabel->setObjectName("installationHeader");
    refreshButton = new QPushButton("Refresh");
    headerLayout->addWidget(headerLabel, 1);
    headerLayout->addWidget(refreshButton);
    mainLayout->addLayout(headerLayout);

    // Installation Folder Structure Info Panel
    auto* infoGroup = new QGroupBox("Installation Folder Structure");
    infoGroup->setObjectName("folderStructureInfo");
    auto* infoLayout = new QGridLayout(infoGroup);
    infoLayout->setSpacing(10);
    
    // Row 0: Game Library Path
    auto* gameLibLabel = new QLabel("<b>Game Library:</b>");
    gameLibLabel->setTextFormat(Qt::RichText);
    gameLibPathLabel = new QLabel();
    gameLibPathLabel->setWordWrap(true);
    infoLayout->addWidget(gameLibLabel, 0, 0, Qt::AlignTop);
    infoLayout->addWidget(gameLibPathLabel, 0, 1);
    
    // Row 1: DLC Path
    auto* dlcLabel = new QLabel("<b>DLC Folder:</b>");
    dlcLabel->setTextFormat(Qt::RichText);
    dlcPathLabel = new QLabel();
    dlcPathLabel->setWordWrap(true);
    infoLayout->addWidget(dlcLabel, 1, 0, Qt::AlignTop);
    infoLayout->addWidget(dlcPathLabel, 1, 1);
    
    // Row 2: Base Games Count
    auto* baseGamesLabel = new QLabel("<b>📦 Base Games:</b>");
    baseGamesLabel->setTextFormat(Qt::RichText);
    baseGamesCountLabel = new QLabel("0");
    baseGamesCountLabel->setTextFormat(Qt::RichText);
    infoLayout->addWidget(baseGamesLabel, 2, 0);
    infoLayout->addWidget(baseGamesCountLabel, 2, 1);
    
    // Row 3: Updates Count
    auto* updatesLabel = new QLabel("<b>🔄 Updates:</b>");
    updatesLabel->setTextFormat(Qt::RichText);
    updatesCountLabel = new QLabel("0");
    updatesCountLabel->setTextFormat(Qt::RichText);
    infoLayout->addWidget(updatesLabel, 3, 0);
    infoLayout->addWidget(updatesCountLabel, 3, 1);
    
    // Row 4: DLC Count
    auto* dlcCountLabel = new QLabel("<b>🎮 DLC:</b>");
    dlcCountLabel->setTextFormat(Qt::RichText);
    dlcCountValueLabel = new QLabel("0");
    dlcCountValueLabel->setTextFormat(Qt::RichText);
    infoLayout->addWidget(dlcCountLabel, 4, 0);
    infoLayout->addWidget(dlcCountValueLabel, 4, 1);
    
    infoLayout->setColumnStretch(1, 1);
    mainLayout->addWidget(infoGroup);

    // Installed games tree
    installedGamesTree = new QTreeWidget();
    installedGamesTree->setHeaderLabels({"Game / Content", "Type", "Location"});
    installedGamesTree->setAlternatingRowColors(true);
    installedGamesTree->setColumnWidth(0, 350);
    installedGamesTree->setColumnWidth(1, 120);
    installedGamesTree->header()->setStretchLastSection(true);
    installedGamesTree->setWordWrap(false);
    mainLayout->addWidget(installedGamesTree);

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &InstallationFolder::refreshView);
}

void InstallationFolder::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #installationHeader {
            font-size: 16px;
            font-weight: bold;
            color: #FFA500;
            padding: 5px;
        }

        QGroupBox#folderStructureInfo {
            background-color: #1e1e1e;
            border: 2px solid #4a9eff;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 15px;
            font-weight: bold;
            color: #4a9eff;
        }

        QGroupBox#folderStructureInfo::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 5px 10px;
            background-color: #2b2b2b;
            border-radius: 3px;
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
        }

        QTreeWidget::item:hover {
            background-color: #3a3a3a;
        }

        QTreeWidget::item:selected {
            background-color: #4a6a8a;
            color: white;
        }

        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #45a049;
        }

        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )");
}

void InstallationFolder::refreshView() {
    loadInstalledGames();
}

void InstallationFolder::loadInstalledGames() {
    Settings& settings = Settings::instance();
    QString gameLibraryPath = settings.getGameLibraryPath();
    QString dlcPath = settings.getDlcFolderPath();
    
    // Update path labels
    gameLibPathLabel->setText(gameLibraryPath);
    dlcPathLabel->setText(dlcPath);
    
    // Count installed content
    int baseGamesCount = 0;
    int updatesCount = 0;
    int dlcCount = 0;
    
    // Scan game library for base games and updates
    QDir gameDir(gameLibraryPath);
    if (gameDir.exists()) {
        QStringList entries = gameDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& entry : entries) {
            if (entry.startsWith("CUSA") || entry.startsWith("PPSA")) {
                if (entry.contains("-UPDATE", Qt::CaseInsensitive)) {
                    updatesCount++;
                } else {
                    baseGamesCount++;
                }
            }
        }
    }
    
    // Scan DLC folder
    QDir dlcDir(dlcPath);
    if (dlcDir.exists()) {
        QStringList titleIdFolders = dlcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& titleId : titleIdFolders) {
            if (titleId.startsWith("CUSA") || titleId.startsWith("PPSA")) {
                QDir titleDir(dlcDir.absoluteFilePath(titleId));
                QStringList dlcSubdirs = titleDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
                dlcCount += dlcSubdirs.size();
            }
        }
    }
    
    // Update count labels with styling
    baseGamesCountLabel->setText(QString("<span style='color: #5cb85c; font-weight: bold;'>%1</span>").arg(baseGamesCount));
    updatesCountLabel->setText(QString("<span style='color: #f0ad4e; font-weight: bold;'>%1</span>").arg(updatesCount));
    dlcCountValueLabel->setText(QString("<span style='color: #d9534f; font-weight: bold;'>%1</span>").arg(dlcCount));

    // Clear tree
    installedGamesTree->clear();
    
    // Create a map to group items by title ID (base game ID without -UPDATE)
    QMap<QString, QTreeWidgetItem*> gameGroups; // titleId -> parent item
    
    // First pass: Create base game parent items
    if (gameDir.exists()) {
        QStringList entries = gameDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& entry : entries) {
            if ((entry.startsWith("CUSA") || entry.startsWith("PPSA")) && !entry.contains("-UPDATE")) {
                // This is a base game - create parent item
                auto* gameItem = new QTreeWidgetItem(installedGamesTree);
                gameItem->setText(0, entry); // Just the Title ID
                gameItem->setText(1, "🎮 Base Game");
                
                // Show just the folder name in location column
                gameItem->setText(2, entry + "/");
                
                // Set tooltip with full path
                QString fullPath = gameDir.absoluteFilePath(entry);
                gameItem->setToolTip(0, fullPath);
                gameItem->setToolTip(2, fullPath);
                
                gameItem->setIcon(0, QIcon::fromTheme("folder"));
                gameItem->setExpanded(false); // Collapsed by default
                
                // Store in map for later reference
                gameGroups[entry] = gameItem;
            }
        }
    }
    
    // Second pass: Add updates as children of their base game
    if (gameDir.exists()) {
        QStringList entries = gameDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& entry : entries) {
            if ((entry.startsWith("CUSA") || entry.startsWith("PPSA")) && entry.contains("-UPDATE")) {
                // Extract base title ID (remove -UPDATE suffix)
                QString baseTitleId = entry;
                baseTitleId.replace("-UPDATE", "");
                
                // Find parent game item
                QTreeWidgetItem* parentItem = gameGroups.value(baseTitleId, nullptr);
                QString fullPath = gameDir.absoluteFilePath(entry);
                
                if (parentItem) {
                    // Add as child of base game
                    auto* updateItem = new QTreeWidgetItem(parentItem);
                    updateItem->setText(0, "Patch/Update");
                    updateItem->setText(1, "🔄 Update");
                    updateItem->setText(2, entry + "/");
                    updateItem->setToolTip(0, fullPath);
                    updateItem->setToolTip(2, fullPath);
                    updateItem->setIcon(0, QIcon::fromTheme("folder-sync"));
                } else {
                    // Orphaned update (no base game found) - add as top-level item
                    auto* updateItem = new QTreeWidgetItem(installedGamesTree);
                    updateItem->setText(0, entry);
                    updateItem->setText(1, "⚠️ Update (No Base)");
                    updateItem->setText(2, entry + "/");
                    updateItem->setToolTip(0, fullPath);
                    updateItem->setToolTip(2, fullPath);
                    updateItem->setIcon(0, QIcon::fromTheme("folder-sync"));
                }
            }
        }
    }
    
    // Third pass: Add DLC as children of their base game
    if (dlcDir.exists()) {
        QStringList titleIdFolders = dlcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& titleId : titleIdFolders) {
            if (titleId.startsWith("CUSA") || titleId.startsWith("PPSA")) {
                QDir titleDir(dlcDir.absoluteFilePath(titleId));
                QStringList dlcSubdirs = titleDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
                
                // Find parent game item
                QTreeWidgetItem* parentItem = gameGroups.value(titleId, nullptr);
                
                for (const QString& dlcName : dlcSubdirs) {
                    QString fullPath = titleDir.absoluteFilePath(dlcName);
                    
                    if (parentItem) {
                        // Add as child of base game
                        auto* dlcItem = new QTreeWidgetItem(parentItem);
                        dlcItem->setText(0, dlcName);
                        dlcItem->setText(1, "📦 DLC");
                        dlcItem->setText(2, titleId + "/" + dlcName + "/");
                        dlcItem->setToolTip(0, fullPath);
                        dlcItem->setToolTip(2, fullPath);
                        dlcItem->setIcon(0, QIcon::fromTheme("folder-download"));
                    } else {
                        // Orphaned DLC (no base game found) - add as top-level item
                        auto* dlcItem = new QTreeWidgetItem(installedGamesTree);
                        dlcItem->setText(0, QString("%1/%2").arg(titleId, dlcName));
                        dlcItem->setText(1, "⚠️ DLC (No Base)");
                        dlcItem->setText(2, titleId + "/" + dlcName + "/");
                        dlcItem->setToolTip(0, fullPath);
                        dlcItem->setToolTip(2, fullPath);
                        dlcItem->setIcon(0, QIcon::fromTheme("folder-download"));
                    }
                }
            }
        }
    }
}

int InstallationFolder::getTotalInstalledCount() {
    Settings& settings = Settings::instance();
    QString gameLibraryPath = settings.getGameLibraryPath();
    QString dlcPath = settings.getDlcFolderPath();
    
    int count = 0;
    
    // Count base games and updates
    QDir gameDir(gameLibraryPath);
    if (gameDir.exists()) {
        QStringList entries = gameDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& entry : entries) {
            if (entry.startsWith("CUSA") || entry.startsWith("PPSA")) {
                count++; // Count both base games and updates
            }
        }
    }
    
    // Count DLC
    QDir dlcDir(dlcPath);
    if (dlcDir.exists()) {
        QStringList titleIdFolders = dlcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString& titleId : titleIdFolders) {
            if (titleId.startsWith("CUSA") || titleId.startsWith("PPSA")) {
                QDir titleDir(dlcDir.absoluteFilePath(titleId));
                QStringList dlcSubdirs = titleDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
                count += dlcSubdirs.size();
            }
        }
    }
    
    return count;
}
