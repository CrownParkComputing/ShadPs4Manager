#ifndef GAME_LIBRARY_H
#define GAME_LIBRARY_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QDialog>
#include <QFrame>
#include <QDirIterator>
#include <QApplication>
#include <QPixmap>
#include <QPainter>
#include <QFrame>
#include <QDialog>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "gamecard.h"
#include "common_types.h"

class IgdbService;

struct GameInfo {
    QString name;
    QString path;
    QString titleId;
    qint64 size;
    QString version;
    QString paramSfoPath;
    QString iconPath;
};

class GameLibrary : public QWidget {
    Q_OBJECT

public:
    GameLibrary(QWidget* parent = nullptr);

signals:
    void gameSelected(const QString& gamePath);
    void extractionRequested(const QString& pkgPath, const QString& outputPath);
    void gameLaunched();
    void gameFinished();

public slots:
    void refreshLibrary();
    void setLibraryPath(const QString& path);

private slots:
    void extractGame();
    void showGameInfo();
    void launchGame();
    void deleteGame();
    void renameGame();
    void viewParamSfo();
    void onLaunchGame(const QString& gamePath);
    void onKillGame(const QString& gamePath);
    void onViewSfo(const QString& gamePath);
    void onRenameGame(const QString& gamePath);
    void onDeleteGame(const QString& gamePath);
    void onShowGameInfo(const GameEntry& gameEntry);
    void onRefreshIgdbData(const GameEntry& gameEntry);
    void performIgdbSearch(const GameEntry& gameEntry, const QString& searchQuery);
    void showIgdbSearchResults(const QList<IgdbGame>& games, const GameEntry& gameEntry);
    void onGameDetailsReady(const IgdbGame& game, const GameEntry& gameEntry);
    void onCoverImageReady(int coverId, const QString& url, const GameEntry& gameEntry);
    void onScreenshotsReady(const QList<QPair<int, QString>>& screenshots, const GameEntry& gameEntry);
    void onIgdbCoverImageRequested(const QString& imageUrl, const GameEntry& gameEntry);

private:
    void showParamSfoEditor(const QString& paramSfoPath, const QString& gamePath);

private:
    void setupUI();
    void applyStyles();
    void loadGames();
    void clearGames();
    void arrangeGameCards();
    GameInfo parseGameInfo(const QString& gamePath);
    QString findEbootBin(const QString& gamePath);
    QString getShadPS4Path();
    QString findParamSfoInDirectory(const QString& gamePath);
    qint64 calculateDirectorySize(const QString& path);
    void saveGameMetadata(const GameEntry& gameEntry);
    void loadGameMetadata(GameEntry& gameEntry);
    QString formatFileSize(qint64 size);

    QScrollArea* scrollArea;
    QWidget* cardsContainer;
    QGridLayout* cardsLayout;
    QLabel* statusLabel;
    QString libraryPath;
    QList<GameInfo> games;
    QList<GameCard*> gameCards;
    
    IgdbService* m_igdbService;
    GameEntry m_currentSearchGameEntry; // Store current search context
};

#endif // GAME_LIBRARY_H
