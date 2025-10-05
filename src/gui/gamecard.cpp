#include "gamecard.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFile>
#include <QDir>
#include <QStyle>
#include <QApplication>
#include <QFont>
#include <QBrush>
#include <QLinearGradient>
#include <QPainter>
#include <QRegularExpression>

GameCard::GameCard(const GameEntry& game, QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_launchState(GameLaunchState::Idle)
    , m_networkManager(nullptr)
{
    setupUI();
    setupStyle();
    updateUI();
}

void GameCard::setupUI() {
    m_mainContainer = new QWidget(this);
    m_mainContainer->setObjectName("mainContainer");

    // Main horizontal layout - cover and buttons directly adjacent
    m_mainLayout = new QHBoxLayout(m_mainContainer);
    m_mainLayout->setSpacing(0); // No spacing at all
    m_mainLayout->setContentsMargins(0, 0, 0, 0); // No margins

    // Cover image (left side)
    m_coverImage = new QLabel(m_mainContainer);
    m_coverImage->setObjectName("coverImage");
    m_coverImage->setMinimumSize(160, 200);
    m_coverImage->setMaximumSize(160, 200);
    m_coverImage->setAlignment(Qt::AlignCenter);

    // Buttons container (right side - directly adjacent)
    QWidget* buttonContainer = new QWidget(m_mainContainer);
    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(1);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // Create icon-only buttons (smaller width, compact design)
    m_launchButton = new QPushButton(buttonContainer);
    m_launchButton->setObjectName("launchButton");
    m_launchButton->setFixedSize(16, 24);
    m_launchButton->setToolTip("Launch Game");
    connect(m_launchButton, &QPushButton::clicked, this, &GameCard::onLaunchButtonClicked);

    m_stopButton = new QPushButton(buttonContainer);
    m_stopButton->setObjectName("stopButton");
    m_stopButton->setFixedSize(16, 24);
    m_stopButton->setToolTip("Stop Game");
    connect(m_stopButton, &QPushButton::clicked, this, &GameCard::onStopButtonClicked);

    m_killButton = new QPushButton(buttonContainer);
    m_killButton->setObjectName("killButton");
    m_killButton->setFixedSize(16, 24);
    m_killButton->setToolTip("Force Kill Game");
    m_killButton->setVisible(false); // Initially hidden
    connect(m_killButton, &QPushButton::clicked, this, &GameCard::onKillButtonClicked);

    m_settingsButton = new QPushButton(buttonContainer);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setFixedSize(16, 21);
    m_settingsButton->setToolTip("Game Settings");
    connect(m_settingsButton, &QPushButton::clicked, this, &GameCard::onSettingsButtonClicked);

    m_infoButton = new QPushButton(buttonContainer);
    m_infoButton->setObjectName("infoButton");
    m_infoButton->setFixedSize(16, 21);
    m_infoButton->setToolTip("Game Details");
    connect(m_infoButton, &QPushButton::clicked, this, &GameCard::onInfoButtonClicked);

    m_deleteButton = new QPushButton(buttonContainer);
    m_deleteButton->setObjectName("deleteButton");
    m_deleteButton->setFixedSize(16, 21);
    m_deleteButton->setToolTip("Delete Game");
    connect(m_deleteButton, &QPushButton::clicked, this, &GameCard::onDeleteButtonClicked);

    m_refreshButton = new QPushButton(buttonContainer);
    m_refreshButton->setObjectName("refreshButton");
    m_refreshButton->setFixedSize(16, 21);
    m_refreshButton->setToolTip("Refresh IGDB Data");
    connect(m_refreshButton, &QPushButton::clicked, this, &GameCard::onRefreshButtonClicked);

    buttonLayout->addWidget(m_launchButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_killButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_infoButton);
    buttonLayout->addWidget(m_settingsButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();

    // Info panel (bottom)
    QWidget* infoPanel = new QWidget(m_mainContainer);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(2, 2, 2, 2);

    m_titleLabel = new QLabel(infoPanel);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(40);

    m_statusLabel = new QLabel(infoPanel);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setVisible(false);

    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_statusLabel);
    infoLayout->addStretch();

    // Add to main layout: cover directly next to buttons, then info below
    m_mainLayout->addWidget(m_coverImage, 0); // No stretching
    m_mainLayout->addWidget(buttonContainer, 0); // No stretching
    m_mainLayout->addWidget(infoPanel, 1); // Takes remaining space

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0); // No outer margins
    mainLayout->setSpacing(0); // No spacing

    setFixedSize(320, 240); // Adjusted size for side-by-side layout
}

void GameCard::setupStyle() {
    QString style = R"(
        #mainContainer {
            background-color: #2d2d2d;
            border: 2px solid #404040;
            border-radius: 8px;
        }

        #coverImage {
            background-color: #1a1a1a;
            border-radius: 6px;
            color: white;
            font-size: 24px;
            font-weight: bold;
        }

        #titleLabel {
            color: white;
            font-size: 12px;
            font-weight: bold;
            padding: 4px;
            text-align: center;
        }

        #statusLabel {
            color: #888;
            font-size: 10px;
            padding: 2px 4px;
            text-align: center;
        }

        #launchButton {
            background-color: #4CAF50;
            border: none;
            border-radius: 8px;
            color: white;
            max-width: 16px;
            min-width: 16px;
        }

        #launchButton:hover {
            background-color: #45a049;
        }

        #stopButton {
            background-color: #f44336;
            border: none;
            border-radius: 8px;
            color: white;
            max-width: 16px;
            min-width: 16px;
        }

        #stopButton:hover {
            background-color: #da190b;
        }

        #settingsButton, #infoButton, #deleteButton {
            background-color: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            color: white;
            max-width: 16px;
            min-width: 16px;
        }

        #settingsButton:hover, #infoButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }

        #deleteButton:hover {
            background-color: #f44336;
        }

        #refreshButton {
            background-color: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            color: white;
            max-width: 16px;
            min-width: 16px;
        }

        #refreshButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )";

    setStyleSheet(style);
}

void GameCard::updateUI() {
    m_titleLabel->setText(m_game.gameData.name);

    // Update launch button based on state and compressed status
    switch (m_launchState) {
        case GameLaunchState::Idle:
            m_launchButton->setToolTip(m_game.gameData.isCompressed ?
                "Extract & Launch Game" : "Launch Game");
            m_launchButton->setText("▶");
            m_launchButton->setVisible(true);
            m_stopButton->setVisible(false);
            m_killButton->setVisible(false);
            break;

        case GameLaunchState::Launching:
            m_launchButton->setToolTip("Launching...");
            m_launchButton->setText("⟳");
            m_launchButton->setVisible(true);
            m_stopButton->setVisible(false);
            m_killButton->setVisible(false);
            break;

        case GameLaunchState::Running:
            m_launchButton->setVisible(false);
            m_stopButton->setVisible(true);
            m_stopButton->setToolTip("Stop Game");
            m_stopButton->setText("⏹");
            m_killButton->setVisible(true);
            m_killButton->setToolTip("Force Kill Game");
            m_killButton->setText("💀");
            break;

        case GameLaunchState::Failed:
            m_launchButton->setToolTip("Launch Failed (Click to Retry)");
            m_launchButton->setText("⚠");
            m_launchButton->setVisible(true);
            m_stopButton->setVisible(false);
            m_killButton->setVisible(false);
            break;
    }

    // Update button text/icons
    m_settingsButton->setText("⚙");
    m_settingsButton->setToolTip("Game Settings");

    m_infoButton->setText("ℹ");
    m_infoButton->setToolTip("Game Details");

    m_deleteButton->setText("🗑");
    m_deleteButton->setToolTip("Delete Game");

    m_refreshButton->setText("🔄");
    m_refreshButton->setToolTip("Refresh IGDB Data");

    // Show compressed indicator if needed
    if (m_game.gameData.isCompressed) {
        m_statusLabel->setText("ZIP");
        m_statusLabel->setStyleSheet("color: #2196F3; font-weight: bold;");
        m_statusLabel->setVisible(true);
    } else {
        m_statusLabel->setVisible(false);
    }

    loadCoverImage();
}

void GameCard::loadCoverImage() {
    QString imagePath;

    // Priority: local file > cached file > IGDB URL > fallback
    if (!m_game.gameData.localCoverPath.isEmpty() && QFile::exists(m_game.gameData.localCoverPath)) {
        imagePath = m_game.gameData.localCoverPath;
    } else {
        // Check for cached cover in game directory with unique filename
        QString gameDir = QFileInfo(m_game.gameData.path).absolutePath();
        QString gameName = QFileInfo(m_game.gameData.path).baseName();
        QString safeGameName = gameName;
        safeGameName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
        QString cacheFile = gameDir + "/" + safeGameName + "_cover_cache.jpg";
        if (QFile::exists(cacheFile)) {
            imagePath = cacheFile;
            m_game.gameData.localCoverPath = cacheFile; // Update the local path
        } else if (!m_game.gameData.coverUrl.isEmpty()) {
            // Try to load IGDB cover image
            loadIgdbCoverImage();
            return; // Exit early since we're handling IGDB loading asynchronously
        }
    }

    if (!imagePath.isEmpty()) {
        // Load local file
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            m_coverImage->setPixmap(pixmap.scaled(160, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            createFallbackCover();
        }
    } else {
        createFallbackCover();
    }
}

void GameCard::createFallbackCover() {
    QString initials = m_game.gameData.name.length() >= 2 ?
        m_game.gameData.name.left(2).toUpper() : m_game.gameData.name.toUpper();

    QPixmap fallback(160, 200);
    fallback.fill(QColor("#1a1a1a"));

    QPainter painter(&fallback);
    painter.setPen(QPen(Qt::white, 2));
    QFont font = painter.font();
    font.setPixelSize(24);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(fallback.rect(), Qt::AlignCenter, initials);

    m_coverImage->setPixmap(fallback);
}

void GameCard::loadIgdbCoverImage() {
    // Show loading indicator
    m_coverImage->setText("Loading...");
    m_coverImage->setStyleSheet("background-color: #1a1a1a; border-radius: 6px; color: #888; font-size: 12px;");

    // Emit signal to request cover image download from parent
    emit igdbCoverImageRequested(m_game.gameData.coverUrl, m_game);
}

void GameCard::setLaunchState(GameLaunchState state) {
    if (m_launchState != state) {
        m_launchState = state;
        updateUI();
    }
}

void GameCard::updateGameData(const GameEntry& game) {
    m_game = game;
    updateUI();
}

void GameCard::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_mainContainer->resize(size());
}

void GameCard::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);
    // Emit launch request when clicking on the card (but not on buttons)
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        if (m_coverImage->geometry().contains(pos) || m_titleLabel->geometry().contains(pos)) {
            emit launchRequested(m_game);
        }
    }
}



void GameCard::onLaunchButtonClicked() {
    emit launchRequested(m_game);
}

void GameCard::onStopButtonClicked() {
    emit stopRequested(m_game);
}

void GameCard::onKillButtonClicked() {
    emit killRequested(m_game);
}

void GameCard::onSettingsButtonClicked() {
    emit settingsRequested(m_game);
}

void GameCard::onInfoButtonClicked() {
    emit infoRequested(m_game);
}

void GameCard::onDeleteButtonClicked() {
    emit deleteRequested(m_game);
}

void GameCard::onRefreshButtonClicked() {
    emit refreshIgdbData(m_game);
}

void GameCard::setCoverImage(const QString& imageUrl) {
    m_game.gameData.coverUrl = imageUrl;
    loadCoverImage();
}

void GameCard::updateGameDetails(const IgdbGame& game) {
    // Update game data with IGDB information
    m_game.gameData.name = game.name;
    m_game.gameData.description = game.summary;
    m_game.gameData.igdbId = game.id;

    // Update UI
    updateUI();
}

void GameCard::updateScreenshots(const QList<QPair<int, QString>>& screenshots) {
    // Update screenshot data
    m_game.gameData.screenshotUrls.clear();
    for (const auto& screenshot : screenshots) {
        m_game.gameData.screenshotUrls.append(screenshot.second);
    }
}

void GameCard::setIgdbCoverImage(const QString& imageUrl) {
    if (imageUrl.isEmpty()) {
        createFallbackCover();
        return;
    }

    // Store the URL
    m_game.gameData.coverUrl = imageUrl;
    
    // Show loading placeholder
    QPixmap loadingPlaceholder(160, 200);
    loadingPlaceholder.fill(QColor("#1a4a1a")); // Dark green to indicate loading

    QPainter painter(&loadingPlaceholder);
    painter.setPen(QPen(Qt::white, 2));
    QFont font = painter.font();
    font.setPixelSize(12);
    font.setBold(true);
    painter.setFont(font);

    QString displayText = "Loading\nIGDB Cover...";
    painter.drawText(loadingPlaceholder.rect(), Qt::AlignCenter, displayText);

    m_coverImage->setPixmap(loadingPlaceholder);

    // Start downloading the actual image
    downloadCoverImage(imageUrl);
}

void GameCard::downloadCoverImage(const QString& imageUrl) {
    if (imageUrl.isEmpty()) return;

    // Create network manager if it doesn't exist
    if (!m_networkManager) {
        m_networkManager = new QNetworkAccessManager(this);
    }

    // Convert IGDB URL to high resolution
    QString highResUrl = imageUrl;
    if (highResUrl.contains("thumb")) {
        highResUrl.replace("thumb", "cover_big");
    }
    
    QUrl url(highResUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::UserAgentHeader, "ShadPs4Manager/1.0");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                // Scale and set the image
                QPixmap scaledPixmap = pixmap.scaled(160, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_coverImage->setPixmap(scaledPixmap);
                
                // Cache the image locally with unique filename
                QString gameDir = QFileInfo(m_game.gameData.path).absolutePath();
                QString gameName = QFileInfo(m_game.gameData.path).baseName();
                QString safeGameName = gameName;
                safeGameName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
                QString cacheFile = gameDir + "/" + safeGameName + "_cover_cache.jpg";
                pixmap.save(cacheFile, "JPG", 85);
                
                // Update local cover path
                m_game.gameData.localCoverPath = cacheFile;
            } else {
                createFallbackCover();
            }
        } else {
            // Download failed, show fallback
            createFallbackCover();
        }
    });
}
