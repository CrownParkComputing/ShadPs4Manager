#include "game_library.h"
#include "settings.h"
#include "param_sfo.h"
#include "igdbservice.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>



GameLibrary::GameLibrary(QWidget* parent) : QWidget(parent) {
    // Initialize IGDB service
    m_igdbService = new IgdbService(this);
    
    // Set up IGDB credentials from settings
    Settings& settings = Settings::instance();
    m_igdbService->setCredentials(settings.getIgdbClientId(), settings.getIgdbClientSecret());
    
    // Connect IGDB signals using lambdas to pass the current game entry context
    connect(m_igdbService, &IgdbService::searchResultsReady, this, [this](const QList<IgdbGame>& games) {
        showIgdbSearchResults(games, m_currentSearchGameEntry);
    });
    
    connect(m_igdbService, &IgdbService::gameDetailsReady, this, [this](const IgdbGame& game) {
        onGameDetailsReady(game, m_currentSearchGameEntry);
    });
    
    connect(m_igdbService, &IgdbService::coverImageReady, this, [this](int coverId, const QString& url) {
        onCoverImageReady(coverId, url, m_currentSearchGameEntry);
    });
    
    connect(m_igdbService, &IgdbService::screenshotsReady, this, [this](const QList<QPair<int, QString>>& screenshots) {
        onScreenshotsReady(screenshots, m_currentSearchGameEntry);
    });
    
    setupUI();
    applyStyles();
    refreshLibrary();
}

void GameLibrary::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* headerLabel = new QLabel("Game Library");
    headerLabel->setObjectName("libraryHeader");
    refreshButton = new QPushButton("Refresh");
    headerLayout->addWidget(headerLabel, 1);
    headerLayout->addWidget(refreshButton);
    mainLayout->addLayout(headerLayout);

    // Scroll area for game cards
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    mainLayout->addWidget(scrollArea);

    // Container widget for cards
    cardsContainer = new QWidget();
    scrollArea->setWidget(cardsContainer);

    // Grid layout for cards (closer together)
    cardsLayout = new QGridLayout(cardsContainer);
    cardsLayout->setSpacing(8);  // Closer spacing (was 15)
    cardsLayout->setContentsMargins(10, 10, 10, 10);  // Tighter margins (was 15)

    // Status label
    statusLabel = new QLabel("Ready");
    mainLayout->addWidget(statusLabel);

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &GameLibrary::refreshLibrary);
}

void GameLibrary::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #libraryHeader {
            font-size: 16px;
            font-weight: bold;
            color: #4CAF50;
            padding: 5px;
        }

        QListWidget {
            background-color: #1e1e1e;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
            outline: none;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #444444;
            background-color: #353535;
            margin: 2px;
            border-radius: 3px;
        }

        QListWidget::item:selected {
            background-color: #4CAF50;
            color: #ffffff;
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

void GameLibrary::refreshLibrary() {
    Settings& settings = Settings::instance();
    setLibraryPath(settings.getGameLibraryPath());
}

void GameLibrary::setLibraryPath(const QString& path) {
    libraryPath = path;
    loadGames();
}

void GameLibrary::loadGames() {
    clearGames();

    if (libraryPath.isEmpty()) {
        statusLabel->setText("No library path configured");
        return;
    }

    QDir libraryDir(libraryPath);
    if (!libraryDir.exists()) {
        statusLabel->setText("Library directory does not exist");
        return;
    }

    // Look for game directories (extracted games)
    QStringList gameDirs = libraryDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (gameDirs.isEmpty()) {
        statusLabel->setText("No games found in library");
        return;
    }

    for (const QString& gameDirName : gameDirs) {
        QString gamePath = libraryDir.absoluteFilePath(gameDirName);

        // Quick check if this is actually a game directory
        QFileInfo gameDirInfo(gamePath);
        if (!gameDirInfo.isDir()) continue;

        // Check if directory has any content before doing expensive operations
        QDir gameDir(gamePath);
        QStringList entries = gameDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        if (entries.isEmpty()) continue;

        GameInfo gameInfo = parseGameInfo(gamePath);

        if (!gameInfo.name.isEmpty()) {
            games.append(gameInfo);

            // Create GameEntry for the new GameCard
            GameEntry gameEntry;
            gameEntry.exe = gameInfo.name;  // Set the exe string
            gameEntry.gameData.name = gameInfo.name;
            gameEntry.gameData.path = gameInfo.path;
            gameEntry.gameData.localCoverPath = gameInfo.iconPath;
            gameEntry.gameData.igdbId = gameInfo.titleId.toInt();  // Use titleId as IGDB ID for now

            // Load any existing IGDB metadata
            loadGameMetadata(gameEntry);

            // Create game card using the new GameCard class
            auto* gameCard = new GameCard(gameEntry);
            gameCards.append(gameCard);

            // Connect the widget signals to our handler slots
            connect(gameCard, &GameCard::launchRequested, this, [this, gameEntry]() {
                onLaunchGame(gameEntry.gameData.path);
            });
            connect(gameCard, &GameCard::settingsRequested, this, [this, gameEntry]() {
                onViewSfo(gameEntry.gameData.path);
            });
            connect(gameCard, &GameCard::infoRequested, this, [this, gameEntry]() {
                onShowInfo(gameEntry.gameData.path);
            });
            connect(gameCard, &GameCard::deleteRequested, this, [this, gameEntry]() {
                onDeleteGame(gameEntry.gameData.path);
            });
            connect(gameCard, &GameCard::refreshIgdbData, this, [this, gameEntry]() {
                onRefreshIgdbData(gameEntry);
            });
            connect(gameCard, &GameCard::igdbCoverImageRequested, this, [this](const QString& imageUrl, const GameEntry& gameEntry) {
                onIgdbCoverImageRequested(imageUrl, gameEntry);
            });
        }
    }

    // Arrange cards in grid layout
    arrangeGameCards();
    statusLabel->setText(QString("Found %1 games").arg(games.size()));
}

void GameLibrary::clearGames() {
    // Clear existing cards
    for (GameCard* card : gameCards) {
        cardsLayout->removeWidget(card);
        card->deleteLater();
    }
    gameCards.clear();
    games.clear();
}

void GameLibrary::arrangeGameCards() {
    // Clear existing layout
    QLayoutItem* item;
    while ((item = cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }

    // Calculate grid dimensions based on available space (accounting for GameCard size)
    int cardsPerRow = 2; // Default for GameCard (320px wide)
    QWidget* viewport = scrollArea->viewport();
    if (viewport) {
        int viewportWidth = viewport->width();
        if (viewportWidth > 0) {
            // Each card is 320px wide + 8px spacing
            cardsPerRow = qMax(1, (viewportWidth - 20) / 328); // 20px for margins, 320px card width + 8px spacing
        }
    }

    // Arrange cards in grid
    for (int i = 0; i < gameCards.size(); ++i) {
        int row = i / cardsPerRow;
        int col = i % cardsPerRow;
        cardsLayout->addWidget(gameCards[i], row, col);
    }
}

GameInfo GameLibrary::parseGameInfo(const QString& gamePath) {
    GameInfo info;
    info.path = gamePath;

    QFileInfo fileInfo(gamePath);
    info.name = fileInfo.baseName();
    info.size = calculateDirectorySize(gamePath);

    // Try to read game metadata from common locations
    QDir gameDir(gamePath);

    // Look for param.sfo file recursively
    QDirIterator it(gamePath, QStringList{"*.sfo"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sfoPath = it.next();
        QFileInfo sfoInfo(sfoPath);

        // Check if this is likely the main param.sfo file (not in subdirectories or named differently)
        if (sfoInfo.fileName().toLower() == "param.sfo" ||
            (sfoInfo.fileName().endsWith(".sfo") && sfoInfo.dir().absolutePath() == gamePath)) {

            info.paramSfoPath = sfoPath;
            ParamSfoData sfoData = parseParamSfo(sfoPath);

            if (!sfoData.titleId.isEmpty()) {
                info.titleId = sfoData.titleId;
            } else {
                info.titleId = "UNKNOWN";
            }

            if (!sfoData.version.isEmpty()) {
                info.version = sfoData.version;
            } else {
                info.version = "1.00";
            }

            if (!sfoData.title.isEmpty()) {
                info.name = sfoData.title;
            }

            break; // Use the first valid param.sfo found
        }
    }

    if (info.titleId == "UNKNOWN") {
        info.titleId = "UNKNOWN";
        info.version = "1.00";
    }

    return info;
}

void GameLibrary::extractGame() {
    // This would be implemented to extract a game from downloads to library
    QMessageBox::information(this, "Extract Game",
        "Game extraction from downloads to library would be implemented here.");
}

void GameLibrary::showGameInfo() {
    // For now, show a placeholder message
    // In a full implementation, this would show detailed game information
    QMessageBox::information(this, "Game Information",
        "Detailed game information would be displayed here.\n\nThis feature would show:\n• Game screenshots\n• Detailed metadata\n• File structure\n• Play time statistics");
}

void GameLibrary::launchGame() {
    // For now, show a placeholder message
    // In a full implementation, this would launch the selected game
    QMessageBox::information(this, "Launch Game",
        "Game launch functionality would be implemented here.\n\nThis would:\n• Find eboot.bin\n• Launch with ShadPS4\n• Handle game arguments");
}

void GameLibrary::deleteGame() {
    // For now, show a placeholder message
    // In a full implementation, this would delete the selected game
    QMessageBox::information(this, "Delete Game",
        "Game deletion would be implemented here.\n\nThis would:\n• Show confirmation dialog\n• Remove game directory\n• Update library view");
}

void GameLibrary::renameGame() {
    // For now, show a placeholder message
    // In a full implementation, this would rename the selected game
    QMessageBox::information(this, "Rename Game",
        "Game renaming would be implemented here.\n\nNote: Since game details come from param.sfo,\nrenaming is generally not needed.");
}

void GameLibrary::viewParamSfo() {
    // For now, show a placeholder message
    // In a full implementation, this would show param.sfo information
    QMessageBox::information(this, "param.sfo Viewer",
        "param.sfo viewer would be implemented here.\n\nThis would:\n• Parse param.sfo file\n• Display game metadata\n• Show file information");
}

QString GameLibrary::findParamSfoInDirectory(const QString& gamePath) {
    // Common locations for param.sfo files in PS4 games
    QStringList commonPaths = {
        gamePath + "/sce_sys/param.sfo",
        gamePath + "/param.sfo",
        gamePath + "/sce_module/param.sfo"
    };

    for (const QString& path : commonPaths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isFile()) {
            return path;
        }
    }

    return QString();
}



void GameLibrary::onRefreshIgdbData(const GameEntry& gameEntry) {
    // Show a search dialog to allow user to modify search parameters
    QString gameName = gameEntry.gameData.name;

    // Create a search dialog
    QDialog* searchDialog = new QDialog(this);
    searchDialog->setWindowTitle("Search IGDB for Game Metadata");
    searchDialog->setModal(true);
    searchDialog->resize(400, 200);

    auto* mainLayout = new QVBoxLayout(searchDialog);

    // Title
    auto* titleLabel = new QLabel("Search for game metadata on IGDB");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #4CAF50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Current game info
    auto* infoLabel = new QLabel(QString("Current game: %1").arg(gameName));
    infoLabel->setStyleSheet("color: #cccccc; margin-bottom: 15px;");
    mainLayout->addWidget(infoLabel);

    // Search field
    auto* searchLayout = new QFormLayout();
    auto* searchLabel = new QLabel("Search query:");
    auto* searchEdit = new QLineEdit(gameName);
    searchEdit->setPlaceholderText("Enter game name to search...");
    searchLayout->addRow(searchLabel, searchEdit);
    mainLayout->addLayout(searchLayout);

    // Platform info
    auto* platformLabel = new QLabel("Note: Searching PS4 games only (platform ID: 48)");
    platformLabel->setStyleSheet("color: #888888; font-size: 11px; margin-bottom: 15px;");
    mainLayout->addWidget(platformLabel);

    // Buttons
    auto* buttonsLayout = new QHBoxLayout();

    auto* searchButton = new QPushButton("Search & Download");
    searchButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");
    buttonsLayout->addWidget(searchButton);

    auto* cancelButton = new QPushButton("Cancel");
    cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #da190b; }");
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    // Connect button signals
    connect(searchButton, &QPushButton::clicked, searchDialog, [this, searchEdit, searchDialog, &gameEntry]() {
        QString searchQuery = searchEdit->text().trimmed();
        if (!searchQuery.isEmpty()) {
            searchDialog->accept();
            performIgdbSearch(gameEntry, searchQuery);
        }
    });

    connect(cancelButton, &QPushButton::clicked, searchDialog, &QDialog::reject);

    // Apply dialog styling
    searchDialog->setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
        }

        QLabel {
            color: #ffffff;
        }

        QLineEdit {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
            color: #ffffff;
        }

        QLineEdit:focus {
            border-color: #4CAF50;
        }
    )");

    if (searchDialog->exec() == QDialog::Accepted) {
        QString searchQuery = searchEdit->text().trimmed();
        if (!searchQuery.isEmpty()) {
            performIgdbSearch(gameEntry, searchQuery);
        }
    }

    searchDialog->deleteLater();
}

void GameLibrary::performIgdbSearch(const GameEntry& gameEntry, const QString& searchQuery) {
    statusLabel->setText(QString("Searching IGDB for: %1...").arg(searchQuery));

    // Check if IGDB credentials are configured
    Settings& settings = Settings::instance();
    QString clientId = settings.getIgdbClientId();
    QString clientSecret = settings.getIgdbClientSecret();

    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        QMessageBox::warning(this, "IGDB Not Configured",
            "IGDB API credentials are not configured.\n\nPlease configure your IGDB Client ID and Client Secret in Settings to use metadata search functionality.");
        statusLabel->setText("Ready");
        return;
    }

    // Store the current game entry context
    m_currentSearchGameEntry = gameEntry;
    
    // Update credentials and perform search
    m_igdbService->setCredentials(clientId, clientSecret);
    m_igdbService->searchGames(searchQuery);
}

void GameLibrary::showIgdbSearchResults(const QList<IgdbGame>& games, const GameEntry& gameEntry) {
    if (games.isEmpty()) {
        QMessageBox::information(this, "No Results",
            "No games found in IGDB matching your search criteria.");
        return;
    }

    // Create a results dialog
    QDialog* resultsDialog = new QDialog(this);
    resultsDialog->setWindowTitle("IGDB Search Results");
    resultsDialog->setModal(true);
    resultsDialog->resize(600, 400);

    auto* mainLayout = new QVBoxLayout(resultsDialog);

    // Title
    auto* titleLabel = new QLabel(QString("Search Results for: %1").arg(gameEntry.gameData.name));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #4CAF50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Results count
    auto* countLabel = new QLabel(QString("Found %1 games:").arg(games.size()));
    countLabel->setStyleSheet("color: #cccccc; margin-bottom: 15px;");
    mainLayout->addWidget(countLabel);

    // Create a scroll area for results
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: 1px solid #555555; border-radius: 4px; }");
    mainLayout->addWidget(scrollArea);

    // Container for results
    QWidget* resultsContainer = new QWidget();
    scrollArea->setWidget(resultsContainer);
    auto* resultsLayout = new QVBoxLayout(resultsContainer);

    // Add each game result
    for (int i = 0; i < games.size(); ++i) {
        const IgdbGame& game = games[i];

        // Game result widget
        QWidget* gameWidget = new QWidget();
        gameWidget->setStyleSheet(R"(
            QWidget {
                background-color: #353535;
                border: 1px solid #555555;
                border-radius: 4px;
                margin: 2px;
                padding: 8px;
            }

            QWidget:hover {
                background-color: #404040;
                border-color: #777777;
            }
        )");

        auto* gameLayout = new QHBoxLayout(gameWidget);

        // Game info
        auto* infoLayout = new QVBoxLayout();

        auto* nameLabel = new QLabel(game.name);
        nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #ffffff;");
        infoLayout->addWidget(nameLabel);

        QString summary = game.summary.left(100);
        if (game.summary.length() > 100) summary += "...";
        auto* summaryLabel = new QLabel(summary);
        summaryLabel->setStyleSheet("color: #cccccc; font-size: 11px;");
        summaryLabel->setWordWrap(true);
        infoLayout->addWidget(summaryLabel);

        auto* idLabel = new QLabel(QString("IGDB ID: %1").arg(game.id));
        idLabel->setStyleSheet("color: #888888; font-size: 10px;");
        infoLayout->addWidget(idLabel);

        gameLayout->addLayout(infoLayout, 1);

        // Action buttons
        auto* buttonsLayout = new QVBoxLayout();

        auto* selectButton = new QPushButton("Select This Game");
        selectButton->setStyleSheet(R"(
            QPushButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 6px 12px;
                border-radius: 4px;
                font-size: 11px;
            }
            QPushButton:hover {
                background-color: #45a049;
            }
        )");

        auto* previewButton = new QPushButton("Preview");
        previewButton->setStyleSheet(R"(
            QPushButton {
                background-color: #2196F3;
                color: white;
                border: none;
                padding: 6px 12px;
                border-radius: 4px;
                font-size: 11px;
            }
            QPushButton:hover {
                background-color: #0b7dda;
            }
        )");

        buttonsLayout->addWidget(selectButton);
        buttonsLayout->addWidget(previewButton);
        buttonsLayout->addStretch();

        gameLayout->addLayout(buttonsLayout);

        resultsLayout->addWidget(gameWidget);

        // Connect button signals
        connect(selectButton, &QPushButton::clicked, resultsDialog, [this, resultsDialog, game]() {
            resultsDialog->accept();
            
            // Store the current search context and fetch full game details
            m_currentSearchGameEntry.gameData.igdbId = game.id;
            
            // Update status and fetch detailed information
            statusLabel->setText(QString("Fetching details for: %1...").arg(game.name));
            
            // Fetch game details, cover, and screenshots
            m_igdbService->fetchGameDetails(game.id);
            if (game.cover > 0) {
                m_igdbService->fetchCoverImage(game.cover);
            }
            if (!game.screenshots.isEmpty()) {
                m_igdbService->fetchScreenshots(game.screenshots);
            }
        });

        connect(previewButton, &QPushButton::clicked, resultsDialog, [this, game]() {
            QMessageBox::information(this, "Game Preview",
                QString("Game: %1\nIGDB ID: %2\nSummary: %3").arg(game.name).arg(game.id).arg(game.summary));
        });
    }

    // Bottom buttons
    auto* buttonsLayout = new QHBoxLayout();

    auto* cancelButton = new QPushButton("Cancel");
    cancelButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #da190b;
        }
    )");
    buttonsLayout->addWidget(cancelButton);

    buttonsLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);

    // Connect cancel button
    connect(cancelButton, &QPushButton::clicked, resultsDialog, &QDialog::reject);

    // Apply dialog styling
    resultsDialog->setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
        }

        QLabel {
            color: #ffffff;
        }
    )");

    resultsDialog->exec();
    resultsDialog->deleteLater();
}

// New handler slots for the inline buttons
void GameLibrary::onLaunchGame(const QString& gamePath) {
    // Find the eboot.bin file to launch
    QString ebootPath = findEbootBin(gamePath);
    if (ebootPath.isEmpty()) {
        QMessageBox::warning(this, "Launch Error",
            "Could not find eboot.bin in the game directory. Make sure the game is properly extracted.");
        return;
    }

    // Get shadps4 executable path from settings or use default
    QString shadps4Path = getShadPS4Path();
    if (shadps4Path.isEmpty()) {
        QMessageBox::warning(this, "ShadPS4 Not Found",
            "Could not find ShadPS4 executable. Please configure the path in Settings.");
        return;
    }

    // Launch the game with shadps4
    QStringList args;
    args << ebootPath;

    QProcess* process = new QProcess(this);
    process->start(shadps4Path, args);

    if (!process->waitForStarted()) {
        QMessageBox::critical(this, "Launch Failed",
            QString("Failed to launch game with ShadPS4:\n%1").arg(process->errorString()));
    } else {
        statusLabel->setText("Game launched successfully");
    }
}

void GameLibrary::onViewSfo(const QString& gamePath) {
    // First, try to find param.sfo in the current game directory
    QString paramSfoPath = findParamSfoInDirectory(gamePath);

    if (paramSfoPath.isEmpty()) {
        // If not found, search recursively
        QDirIterator it(gamePath, QStringList{"*.sfo"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString sfoPath = it.next();
            QFileInfo sfoInfo(sfoPath);

            // Look for param.sfo specifically
            if (sfoInfo.fileName().toLower() == "param.sfo") {
                paramSfoPath = sfoPath;
                break;
            }
        }
    }

    if (paramSfoPath.isEmpty()) {
        QMessageBox::warning(this, "No param.sfo Found",
            "No param.sfo file found for this game.\n\nSearched in:\n" + gamePath);
        return;
    }

    // Show param.sfo editor dialog
    showParamSfoEditor(paramSfoPath, gamePath);
}

void GameLibrary::showParamSfoEditor(const QString& paramSfoPath, const QString& gamePath) {
    QDialog* editorDialog = new QDialog(this);
    editorDialog->setWindowTitle("param.sfo Editor");
    editorDialog->setModal(true);
    editorDialog->resize(500, 400);

    auto* mainLayout = new QVBoxLayout(editorDialog);

    // Title
    auto* titleLabel = new QLabel("Edit param.sfo File");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #4CAF50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // File path info
    auto* pathLabel = new QLabel(QString("File: %1").arg(paramSfoPath));
    pathLabel->setStyleSheet("color: #cccccc; font-size: 11px; margin-bottom: 15px;");
    mainLayout->addWidget(pathLabel);

    // Parse current param.sfo data
    ParamSfoData currentData = parseParamSfo(paramSfoPath);

    // Create form fields
    auto* formLayout = new QFormLayout();

    // Title ID field
    auto* titleIdLabel = new QLabel("Title ID:");
    auto* titleIdEdit = new QLineEdit(currentData.titleId.isEmpty() ? "UNKNOWN" : currentData.titleId);
    titleIdEdit->setMaxLength(12);
    formLayout->addRow(titleIdLabel, titleIdEdit);

    // Title field
    auto* titleLabelField = new QLabel("Title:");
    auto* titleEdit = new QLineEdit(currentData.title.isEmpty() ? "Unknown Game" : currentData.title);
    titleEdit->setMaxLength(128);
    formLayout->addRow(titleLabelField, titleEdit);

    // Version field
    auto* versionLabel = new QLabel("Version:");
    auto* versionEdit = new QLineEdit(currentData.version.isEmpty() ? "01.00" : currentData.version);
    versionEdit->setMaxLength(8);
    formLayout->addRow(versionLabel, versionEdit);

    // Category field
    auto* categoryLabel = new QLabel("Category:");
    auto* categoryEdit = new QLineEdit(currentData.category.isEmpty() ? "gd" : currentData.category);
    categoryEdit->setMaxLength(4);
    formLayout->addRow(categoryLabel, categoryEdit);

    // Content ID field (read-only as it's usually auto-generated)
    auto* contentIdLabel = new QLabel("Content ID:");
    auto* contentIdEdit = new QLineEdit(currentData.contentId.isEmpty() ? "AUTO-GENERATED" : currentData.contentId);
    contentIdEdit->setEnabled(false);
    contentIdEdit->setStyleSheet("background-color: #444444; color: #888888;");
    formLayout->addRow(contentIdLabel, contentIdEdit);

    mainLayout->addLayout(formLayout);

    // Buttons
    auto* buttonsLayout = new QHBoxLayout();

    auto* saveButton = new QPushButton("Save Changes");
    saveButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");
    buttonsLayout->addWidget(saveButton);

    auto* cancelButton = new QPushButton("Cancel");
    cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #da190b; }");
    buttonsLayout->addWidget(cancelButton);

    auto* refreshButton = new QPushButton("Refresh");
    refreshButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #0b7dda; }");
    buttonsLayout->addWidget(refreshButton);

    mainLayout->addLayout(buttonsLayout);

    // Connect button signals
    connect(saveButton, &QPushButton::clicked, editorDialog, [=, &currentData]() {
        // Update the data with form values
        currentData.titleId = titleIdEdit->text();
        currentData.title = titleEdit->text();
        currentData.version = versionEdit->text();
        currentData.category = categoryEdit->text();

        // Save the changes (this would need to be implemented in param_sfo.cpp)
        if (saveParamSfo(paramSfoPath, currentData)) {
            QMessageBox::information(editorDialog, "Success", "param.sfo file has been updated successfully!");
            editorDialog->accept();
        } else {
            QMessageBox::critical(editorDialog, "Error", "Failed to save param.sfo file. Make sure the file is not read-only.");
        }
    });

    connect(cancelButton, &QPushButton::clicked, editorDialog, &QDialog::reject);

    connect(refreshButton, &QPushButton::clicked, editorDialog, [=, &currentData]() {
        // Reload current data from file
        ParamSfoData freshData = parseParamSfo(paramSfoPath);

        titleIdEdit->setText(freshData.titleId.isEmpty() ? "UNKNOWN" : freshData.titleId);
        titleEdit->setText(freshData.title.isEmpty() ? "Unknown Game" : freshData.title);
        versionEdit->setText(freshData.version.isEmpty() ? "01.00" : freshData.version);
        categoryEdit->setText(freshData.category.isEmpty() ? "gd" : freshData.category);
        contentIdEdit->setText(freshData.contentId.isEmpty() ? "AUTO-GENERATED" : freshData.contentId);
    });

    // Apply dialog styling
    editorDialog->setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
        }

        QLabel {
            color: #ffffff;
        }

        QLineEdit {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
            color: #ffffff;
        }

        QLineEdit:focus {
            border-color: #4CAF50;
        }

        QLineEdit:disabled {
            background-color: #2a2a2a;
            color: #666666;
        }
    )");

    editorDialog->exec();
    editorDialog->deleteLater();
}

void GameLibrary::onRenameGame(const QString& gamePath) {
    QFileInfo gameInfo(gamePath);
    QString currentName = gameInfo.baseName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Game",
        "Enter new name for the game:", QLineEdit::Normal,
        currentName, &ok);

    if (ok && !newName.isEmpty() && newName != currentName) {
        QString newPath = gameInfo.absoluteDir().absoluteFilePath(newName);

        QDir gameDir(gamePath);
        if (gameDir.rename(gamePath, newPath)) {
            QMessageBox::information(this, "Game Renamed",
                QString("Game has been renamed from '%1' to '%2'.").arg(currentName).arg(newName));
            refreshLibrary(); // Refresh the library to update the list
        } else {
            QMessageBox::critical(this, "Rename Failed",
                QString("Failed to rename game from '%1' to '%2'.").arg(currentName).arg(newName));
        }
    }
}

void GameLibrary::onDeleteGame(const QString& gamePath) {
    QFileInfo gameInfo(gamePath);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Delete Game",
        QString("Are you sure you want to delete the game '%1'?\n\nThis action cannot be undone.").arg(gameInfo.baseName()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDir gameDir(gamePath);
        if (gameDir.removeRecursively()) {
            QMessageBox::information(this, "Game Deleted",
                QString("Game '%1' has been deleted successfully.").arg(gameInfo.baseName()));
            refreshLibrary(); // Refresh the library to update the list
        } else {
            QMessageBox::critical(this, "Delete Failed",
                QString("Failed to delete game '%1'. Make sure no files are in use.").arg(gameInfo.baseName()));
        }
    }
}

void GameLibrary::onShowInfo(const QString& gamePath) {
    QMessageBox::information(this, "Game Information",
        QString("Game Path: %1\n\nMore detailed game information would be displayed here.").arg(gamePath));
}

void GameLibrary::onCoverImageReady(int coverId, const QString& url, const GameEntry& gameEntry) {
    // Handle cover image download completion
    statusLabel->setText(QString("Cover image downloaded for: %1").arg(gameEntry.gameData.name));

    // Update the game card with the new cover image
    for (GameCard* card : gameCards) {
        if (card && card->getGameEntry().gameData.path == gameEntry.gameData.path) {
            // Update the card's cover image
            card->setCoverImage(url);
            
            // Create updated entry for saving
            GameEntry updatedEntry = gameEntry;
            updatedEntry.gameData.coverUrl = url;
            
            // Save the updated metadata to disk
            saveGameMetadata(updatedEntry);
            break;
        }
    }
}

void GameLibrary::onGameDetailsReady(const IgdbGame& game, const GameEntry& gameEntry) {
    // Handle game details download completion
    statusLabel->setText(QString("Game details updated for: %1").arg(gameEntry.gameData.name));

    // Update the game card with the new details
    for (GameCard* card : gameCards) {
        if (card && card->getGameEntry().gameData.path == gameEntry.gameData.path) {
            // Update the card's game details
            card->updateGameDetails(game);
            
            // Create updated entry for saving
            GameEntry updatedEntry = gameEntry;
            updatedEntry.gameData.name = game.name;
            updatedEntry.gameData.description = game.summary;
            updatedEntry.gameData.igdbId = game.id;
            
            // Save the updated metadata to disk
            saveGameMetadata(updatedEntry);
            break;
        }
    }
}

void GameLibrary::onScreenshotsReady(const QList<QPair<int, QString>>& screenshots, const GameEntry& gameEntry) {
    // Handle screenshots download completion
    statusLabel->setText(QString("Screenshots downloaded for: %1 (%2 images)").arg(gameEntry.gameData.name).arg(screenshots.size()));

    // Update the game card with the new screenshots
    for (GameCard* card : gameCards) {
        if (card && card->getGameEntry().gameData.path == gameEntry.gameData.path) {
            // Update the card's screenshots
            card->updateScreenshots(screenshots);
            
            // Create updated entry for saving
            GameEntry updatedEntry = gameEntry;
            updatedEntry.gameData.screenshotUrls.clear();
            for (const auto& screenshot : screenshots) {
                updatedEntry.gameData.screenshotUrls.append(screenshot.second);
            }
            
            // Save the updated metadata to disk
            saveGameMetadata(updatedEntry);
            break;
        }
    }
}

qint64 GameLibrary::calculateDirectorySize(const QString& path) {
    qint64 totalSize = 0;
    QFileInfo fileInfo(path);

    if (fileInfo.isDir()) {
        // Use a more efficient approach - avoid deep recursion for common cases
        QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            totalSize += it.fileInfo().size();
        }

        // For game directories, also check common subdirectories without full recursion
        QStringList commonSubdirs = {"/sce_sys", "/sce_module"};
        for (const QString& subdir : commonSubdirs) {
            QString subdirPath = path + subdir;
            QFileInfo subdirInfo(subdirPath);
            if (subdirInfo.exists() && subdirInfo.isDir()) {
                QDirIterator subIt(subdirPath, QDir::Files);
                while (subIt.hasNext()) {
                    subIt.next();
                    totalSize += subIt.fileInfo().size();
                }
            }
        }
    } else if (fileInfo.isFile()) {
        totalSize = fileInfo.size();
    }

    return totalSize;
}

QString GameLibrary::findEbootBin(const QString& gamePath) {
    QDirIterator it(gamePath, QStringList{"eboot.bin"}, QDir::Files, QDirIterator::Subdirectories);
    if (it.hasNext()) {
        return it.next();
    }
    return QString();
}

QString GameLibrary::getShadPS4Path() {
    // Try to get from settings first
    Settings& settings = Settings::instance();
    QString path = settings.getShadPS4Path();
    if (!path.isEmpty() && QFile::exists(path)) {
        return path;
    }

    // Try common installation paths
    QStringList commonPaths = {
        "/usr/bin/shadps4",
        "/usr/local/bin/shadps4",
        QStandardPaths::findExecutable("shadps4"),
        QDir::homePath() + "/shadps4/shadps4",
        QDir::homePath() + "/.local/bin/shadps4"
    };

    for (const QString& testPath : commonPaths) {
        if (QFile::exists(testPath)) {
            return testPath;
        }
    }

    return QString();
}

void GameLibrary::saveGameMetadata(const GameEntry& gameEntry) {
    // Validate input
    if (gameEntry.gameData.path.isEmpty()) {
        return;
    }
    
    // Create metadata file path (store in game directory as .igdb_metadata.json)
    QString metadataPath = gameEntry.gameData.path + "/.igdb_metadata.json";
    
    // Ensure directory exists
    QDir dir(gameEntry.gameData.path);
    if (!dir.exists()) {
        return;
    }
    
    QJsonObject metadata;
    metadata["igdbId"] = gameEntry.gameData.igdbId;
    metadata["name"] = gameEntry.gameData.name;
    metadata["description"] = gameEntry.gameData.description;
    metadata["coverUrl"] = gameEntry.gameData.coverUrl;
    metadata["localCoverPath"] = gameEntry.gameData.localCoverPath;
    
    // Save screenshot URLs
    QJsonArray screenshotArray;
    for (const QString& url : gameEntry.gameData.screenshotUrls) {
        screenshotArray.append(url);
    }
    metadata["screenshotUrls"] = screenshotArray;
    
    // Save screenshot IDs
    QJsonArray screenshotIdArray;
    for (int id : gameEntry.gameData.igdbScreenshotIds) {
        screenshotIdArray.append(id);
    }
    metadata["igdbScreenshotIds"] = screenshotIdArray;
    
    // Write to file
    QJsonDocument doc(metadata);
    QFile file(metadataPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void GameLibrary::loadGameMetadata(GameEntry& gameEntry) {
    // Load metadata file if it exists
    QString metadataPath = gameEntry.gameData.path + "/.igdb_metadata.json";
    QFile file(metadataPath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return;
    }

    QJsonObject metadata = doc.object();

    // Load metadata
    if (metadata.contains("igdbId")) {
        gameEntry.gameData.igdbId = metadata["igdbId"].toInt();
    }
    if (metadata.contains("description")) {
        gameEntry.gameData.description = metadata["description"].toString();
    }
    if (metadata.contains("coverUrl")) {
        gameEntry.gameData.coverUrl = metadata["coverUrl"].toString();
    }
    if (metadata.contains("localCoverPath")) {
        gameEntry.gameData.localCoverPath = metadata["localCoverPath"].toString();
    }

    // Load screenshot URLs
    if (metadata.contains("screenshotUrls")) {
        QJsonArray screenshotArray = metadata["screenshotUrls"].toArray();
        gameEntry.gameData.screenshotUrls.clear();
        for (const QJsonValue& value : screenshotArray) {
            gameEntry.gameData.screenshotUrls.append(value.toString());
        }
    }

    // Load screenshot IDs
    if (metadata.contains("igdbScreenshotIds")) {
        QJsonArray screenshotIdArray = metadata["igdbScreenshotIds"].toArray();
        gameEntry.gameData.igdbScreenshotIds.clear();
        for (const QJsonValue& value : screenshotIdArray) {
            gameEntry.gameData.igdbScreenshotIds.append(value.toInt());
        }
    }
}

void GameLibrary::onIgdbCoverImageRequested(const QString& imageUrl, const GameEntry& gameEntry) {
    // For now, we'll use a simple approach to download and display the image
    // In a full implementation, you might want to use QNetworkAccessManager to download the image

    // Since we don't have network access in this context, we'll show a placeholder
    // and indicate that the image URL is available for future loading
    for (GameCard* card : gameCards) {
        if (card->getGameEntry().gameData.path == gameEntry.gameData.path) {
            // Update the card to show that IGDB image is available
            card->setIgdbCoverImage(imageUrl);

            // Update the game entry with the cover URL for future use
            GameEntry updatedEntry = card->getGameEntry();
            updatedEntry.gameData.coverUrl = imageUrl;
            card->updateGameData(updatedEntry);

            // Save metadata
            saveGameMetadata(updatedEntry);
            break;
        }
    }
}
