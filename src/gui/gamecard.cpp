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

GameCard::GameCard(const GameEntry& game, QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_launchState(GameLaunchState::Idle)
{
    setupUI();
    setupStyle();
    updateUI();
}

void GameCard::setupUI() {
    m_mainContainer = new QWidget(this);
    m_mainContainer->setObjectName("mainContainer");

    // Main horizontal layout for image and buttons side by side
    m_mainLayout = new QHBoxLayout(m_mainContainer);

    // Left side - game image and info
    QWidget* leftPanel = new QWidget(m_mainContainer);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_coverImage = new QLabel(leftPanel);
    m_coverImage->setObjectName("coverImage");
    m_coverImage->setMinimumSize(160, 200);
    m_coverImage->setMaximumSize(160, 200);
    m_coverImage->setAlignment(Qt::AlignCenter);

    m_titleLabel = new QLabel(leftPanel);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(40);

    m_statusLabel = new QLabel(leftPanel);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setVisible(false);

    leftLayout->addWidget(m_coverImage);
    leftLayout->addWidget(m_titleLabel);
    leftLayout->addWidget(m_statusLabel);
    leftLayout->addStretch();

    // Right side - buttons
    QWidget* rightPanel = new QWidget(m_mainContainer);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    // Create icon-only buttons (50% width reduction, 50% height increase)
    m_launchButton = new QPushButton(rightPanel);
    m_launchButton->setObjectName("launchButton");
    m_launchButton->setFixedSize(8, 24);
    m_launchButton->setToolTip("Launch Game");
    connect(m_launchButton, &QPushButton::clicked, this, &GameCard::onLaunchButtonClicked);

    m_stopButton = new QPushButton(rightPanel);
    m_stopButton->setObjectName("stopButton");
    m_stopButton->setFixedSize(8, 24);
    m_stopButton->setToolTip("Stop Game");
    connect(m_stopButton, &QPushButton::clicked, this, &GameCard::onStopButtonClicked);

    m_settingsButton = new QPushButton(rightPanel);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setFixedSize(7, 21);
    m_settingsButton->setToolTip("Game Settings");
    connect(m_settingsButton, &QPushButton::clicked, this, &GameCard::onSettingsButtonClicked);

    m_infoButton = new QPushButton(rightPanel);
    m_infoButton->setObjectName("infoButton");
    m_infoButton->setFixedSize(7, 21);
    m_infoButton->setToolTip("Game Details");
    connect(m_infoButton, &QPushButton::clicked, this, &GameCard::onInfoButtonClicked);

    m_deleteButton = new QPushButton(rightPanel);
    m_deleteButton->setObjectName("deleteButton");
    m_deleteButton->setFixedSize(7, 21);
    m_deleteButton->setToolTip("Delete Game");
    connect(m_deleteButton, &QPushButton::clicked, this, &GameCard::onDeleteButtonClicked);

    m_refreshButton = new QPushButton(rightPanel);
    m_refreshButton->setObjectName("refreshButton");
    m_refreshButton->setFixedSize(7, 21);
    m_refreshButton->setToolTip("Refresh IGDB Data");
    connect(m_refreshButton, &QPushButton::clicked, this, &GameCard::onRefreshButtonClicked);

    rightLayout->addWidget(m_launchButton);
    rightLayout->addWidget(m_stopButton);
    rightLayout->addStretch();
    rightLayout->addWidget(m_infoButton);
    rightLayout->addWidget(m_settingsButton);
    rightLayout->addWidget(m_deleteButton);
    rightLayout->addWidget(m_refreshButton);
    rightLayout->addStretch();

    // Add panels to main layout
    m_mainLayout->addWidget(leftPanel, 1);
    m_mainLayout->addWidget(rightPanel, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);

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
            border-radius: 16px;
            color: white;
        }

        #launchButton:hover {
            background-color: #45a049;
        }

        #stopButton {
            background-color: #f44336;
            border: none;
            border-radius: 16px;
            color: white;
        }

        #stopButton:hover {
            background-color: #da190b;
        }

        #settingsButton, #infoButton, #deleteButton {
            background-color: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 12px;
            color: white;
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
            border-radius: 12px;
            color: white;
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
            break;

        case GameLaunchState::Launching:
            m_launchButton->setToolTip("Launching...");
            m_launchButton->setText("⟳");
            m_launchButton->setVisible(true);
            m_stopButton->setVisible(false);
            break;

        case GameLaunchState::Running:
            m_launchButton->setVisible(false);
            m_stopButton->setVisible(true);
            m_stopButton->setToolTip("Stop Game");
            m_stopButton->setText("⏹");
            break;

        case GameLaunchState::Failed:
            m_launchButton->setToolTip("Launch Failed (Click to Retry)");
            m_launchButton->setText("⚠");
            m_launchButton->setVisible(true);
            m_stopButton->setVisible(false);
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

    // Priority: local file > IGDB URL > fallback
    if (!m_game.gameData.localCoverPath.isEmpty() && QFile::exists(m_game.gameData.localCoverPath)) {
        imagePath = m_game.gameData.localCoverPath;
    } else if (!m_game.gameData.coverUrl.isEmpty()) {
        // Try to load IGDB cover image
        loadIgdbCoverImage();
        return; // Exit early since we're handling IGDB loading asynchronously
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
    // For now, we'll show a placeholder indicating IGDB image is available
    // In a full implementation, this would load the actual image from the URL

    // Create a placeholder that indicates IGDB image availability
    QPixmap igdbPlaceholder(160, 200);
    igdbPlaceholder.fill(QColor("#1a4a1a")); // Dark green to indicate IGDB

    QPainter painter(&igdbPlaceholder);
    painter.setPen(QPen(Qt::white, 2));
    QFont font = painter.font();
    font.setPixelSize(14);
    font.setBold(true);
    painter.setFont(font);

    // Show that IGDB image is available
    QString displayText = "IGDB\nCover";
    painter.drawText(igdbPlaceholder.rect(), Qt::AlignCenter, displayText);

    m_coverImage->setPixmap(igdbPlaceholder);

    // Store the URL for potential future loading
    m_game.gameData.coverUrl = imageUrl;
}
