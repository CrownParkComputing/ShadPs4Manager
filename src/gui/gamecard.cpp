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

    // Main vertical layout - title at top with more space, cover and icons at bottom
    m_mainLayout = new QVBoxLayout(m_mainContainer);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);

    // Play buttons removed - cover image will be clickable for launch

    // Title label (at top with more space)
    m_titleLabel = new QLabel(m_mainContainer);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMinimumHeight(60);  // Increased from 40
    m_titleLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Status info panel (below title)
    QWidget* statusPanel = new QWidget(m_mainContainer);
    QVBoxLayout* statusPanelLayout = new QVBoxLayout(statusPanel);
    statusPanelLayout->setSpacing(2);
    statusPanelLayout->setContentsMargins(2, 2, 2, 2);

    m_statusLabel = new QLabel(statusPanel);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setVisible(false);

    statusPanelLayout->addWidget(m_statusLabel);
    statusPanelLayout->addStretch();

    // Cover image (at bottom)
    m_coverImage = new QLabel(m_mainContainer);
    m_coverImage->setObjectName("coverImage");
    m_coverImage->setMinimumSize(160, 200);
    m_coverImage->setMaximumSize(160, 200);
    m_coverImage->setAlignment(Qt::AlignCenter);
    m_coverImage->setCursor(Qt::PointingHandCursor);
    m_coverImage->setToolTip("Click to launch game");
    m_coverImage->setStyleSheet("QLabel:hover { border: 2px solid #2196F3; }");
    
    // Make cover image clickable
    m_coverImage->installEventFilter(this);

    // Info buttons container (bottom - under cover image)
    QWidget* infoButtonContainer = new QWidget(m_mainContainer);
    QHBoxLayout* infoButtonLayout = new QHBoxLayout(infoButtonContainer);
    infoButtonLayout->setContentsMargins(0, 0, 0, 0);
    infoButtonLayout->setSpacing(2); // Reduced spacing to fit all icons

    // Create small info buttons (11px wide to prevent cutoff)
    m_settingsButton = new QPushButton(infoButtonContainer);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setFixedSize(11, 21);
    m_settingsButton->setToolTip("Game Settings");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    connect(m_settingsButton, &QPushButton::clicked, this, &GameCard::onSettingsButtonClicked);

    m_infoButton = new QPushButton(infoButtonContainer);
    m_infoButton->setObjectName("infoButton");
    m_infoButton->setFixedSize(11, 21);
    m_infoButton->setToolTip("Game Information");
    m_infoButton->setCursor(Qt::PointingHandCursor);
    connect(m_infoButton, &QPushButton::clicked, this, &GameCard::onInfoButtonClicked);

    m_deleteButton = new QPushButton(infoButtonContainer);
    m_deleteButton->setObjectName("deleteButton");
    m_deleteButton->setFixedSize(11, 21);
    m_deleteButton->setToolTip("Delete Game");
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    connect(m_deleteButton, &QPushButton::clicked, this, &GameCard::onDeleteButtonClicked);

    m_refreshButton = new QPushButton(infoButtonContainer);
    m_refreshButton->setObjectName("refreshButton");
    m_refreshButton->setFixedSize(11, 21);
    m_refreshButton->setToolTip("Refresh IGDB Data");
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    connect(m_refreshButton, &QPushButton::clicked, this, &GameCard::onRefreshButtonClicked);

    // Add info buttons horizontally to fit under cover
    infoButtonLayout->addWidget(m_settingsButton);
    infoButtonLayout->addWidget(m_infoButton);
    infoButtonLayout->addWidget(m_deleteButton);
    infoButtonLayout->addWidget(m_refreshButton);

    // Add all components to main vertical layout (title at top, cover/icons at bottom)
    m_mainLayout->addWidget(m_titleLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    m_mainLayout->addWidget(statusPanel);
    m_mainLayout->addStretch();  // This pushes cover and icons to bottom
    m_mainLayout->addWidget(m_coverImage, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(infoButtonContainer, 0, Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setFixedSize(180, 320); // Adjusted size for vertical layout
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
            font-size: 14px;
            font-weight: bold;
        }

        #launchButton:hover {
            background-color: #45a049;
        }

        #stopButton {
            background-color: #f44336;
            border: none;
            border-radius: 8px;
            color: white;
            font-size: 14px;
            font-weight: bold;
        }

        #stopButton:hover {
            background-color: #da190b;
        }

        #killButton {
            background-color: #ff5722;
            border: none;
            border-radius: 8px;
            color: white;
            font-size: 12px;
            font-weight: bold;
        }

        #killButton:hover {
            background-color: #e64a19;
        }

        #settingsButton, #infoButton, #deleteButton, #refreshButton {
            background-color: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 4px;
            color: white;
            font-size: 10px;
            max-width: 11px;
            min-width: 11px;
        }

        #settingsButton:hover, #infoButton:hover, #refreshButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }

        #deleteButton:hover {
            background-color: #f44336;
        }
    )";

    setStyleSheet(style);
}

void GameCard::updateUI() {
    m_titleLabel->setText(m_game.gameData.name);

    // Update cover image appearance based on state and compressed status
    QString coverTooltip;
    QString coverBorderColor = "#2196F3"; // Default blue border on hover
    
    switch (m_launchState) {
        case GameLaunchState::Idle:
            coverTooltip = m_game.gameData.isCompressed ?
                "Click to extract & launch game" : "Click to launch game";
            coverBorderColor = "#4CAF50"; // Green for ready to launch
            break;

        case GameLaunchState::Launching:
            coverTooltip = "Game is launching...";
            coverBorderColor = "#FF9800"; // Orange for launching
            break;

        case GameLaunchState::Running:
            coverTooltip = "Game is currently running";
            coverBorderColor = "#F44336"; // Red for running
            break;

        case GameLaunchState::Failed:
            coverTooltip = "Launch failed - Click to retry";
            coverBorderColor = "#9C27B0"; // Purple for failed
            break;
    }
    
    m_coverImage->setToolTip(coverTooltip);
    
    // Update cover hover border color based on state
    QString hoverStyle = QString("QLabel:hover { border: 2px solid %1; }").arg(coverBorderColor);
    m_coverImage->setStyleSheet(hoverStyle);

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

bool GameCard::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_coverImage && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Handle cover image click - launch the game
            emit launchRequested(m_game);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// Launch functionality now handled through cover image click
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
