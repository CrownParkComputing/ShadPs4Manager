#ifndef GAMECARD_H
#define GAMECARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "common_types.h"

struct GameEntry {
    QString exe;
    struct {
        QString name;
        QString path;
        bool isCompressed = false;
        QString localCoverPath;
        QString coverUrl;
        int igdbId = 0;
        QString description;
        QList<int> igdbScreenshotIds;
        QList<QString> screenshotUrls;
        QString prefixName;
    } gameData;
};

enum class GameLaunchState {
    Idle,
    Launching,
    Running,
    Failed
};

class GameCard : public QWidget {
    Q_OBJECT

public:
    explicit GameCard(const GameEntry& game, QWidget* parent = nullptr);

    void setLaunchState(GameLaunchState state);
    void updateGameData(const GameEntry& game);

    // New methods for metadata integration
    QString getGamePath() const { return m_game.gameData.path; }
    GameEntry getGameEntry() const { return m_game; }

    // Metadata update methods
    void setCoverImage(const QString& imageUrl);
    void updateGameDetails(const struct IgdbGame& game);
    void updateScreenshots(const QList<QPair<int, QString>>& screenshots);
    void setIgdbCoverImage(const QString& imageUrl);

signals:
    void launchRequested(const GameEntry& game);
    void stopRequested(const GameEntry& game);
    void killRequested(const GameEntry& game);
    void settingsRequested(const GameEntry& game);
    void infoRequested(const GameEntry& game);
    void deleteRequested(const GameEntry& game);
    void refreshIgdbData(const GameEntry& game);
    void igdbCoverImageRequested(const QString& imageUrl, const GameEntry& game);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    // Launch functionality now handled through cover image click
    void onSettingsButtonClicked();
    void onInfoButtonClicked();
    void onDeleteButtonClicked();
    void onRefreshButtonClicked();

private:
    void setupUI();
    void setupStyle();
    void updateUI();
    void loadCoverImage();
    void loadIgdbCoverImage();
    void createFallbackCover();
    void downloadCoverImage(const QString& imageUrl);

    GameEntry m_game;
    GameLaunchState m_launchState;

    QWidget* m_mainContainer;
    QLabel* m_coverImage;
    QLabel* m_titleLabel;
    QLabel* m_statusLabel;
    // Launch buttons removed - cover image is now clickable
    QPushButton* m_settingsButton;
    QPushButton* m_infoButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;

    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_buttonLayout;

    // Network manager for image downloading
    QNetworkAccessManager* m_networkManager;
};

#endif // GAMECARD_H
