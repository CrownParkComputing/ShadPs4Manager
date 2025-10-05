#ifndef IGDBSERVICE_H
#define IGDBSERVICE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPair>

#include "common_types.h"

struct IgdbCover {
    int id = 0;
    QString imageId;
    QString url;
};

struct IgdbScreenshot {
    int id = 0;
    QString imageId;
    QString url;
};

class IgdbService : public QObject {
    Q_OBJECT

public:
    explicit IgdbService(QObject* parent = nullptr);

    void setCredentials(const QString& clientId, const QString& clientSecret);

    void searchGames(const QString& query);
    void fetchGameDetails(int gameId);
    void fetchCoverImage(int coverId);
    void fetchScreenshots(const QList<int>& screenshotIds);

    // New method for automatic metadata downloading
    void downloadMetadataForGame(const QString& gameName);

    // Test method to verify IGDB API integration
    void testIgdbConnection();

signals:
    void searchResultsReady(const QList<IgdbGame>& games);
    void gameDetailsReady(const IgdbGame& game);
    void coverImageReady(int coverId, const QString& url);
    void screenshotsReady(const QList<QPair<int, QString>>& screenshots);
    void errorOccurred(const QString& error);
    void testResultsReady(const QString& testResult);

private slots:
    void onTokenReceived();
    void onSearchFinished();
    void onGameDetailsFinished();
    void onCoverImageFinished();
    void onScreenshotsFinished();
    void onAutoSearchFinished();
    void onTestFinished();

private:
    QString getAccessToken();
    void requestAccessToken();
    void performGameSearch(const QString& gameName);
    void performTestSearch();

    QString m_clientId;
    QString m_clientSecret;
    QString m_accessToken;
    QDateTime m_tokenExpiry;
    QString m_pendingGameName;

    QNetworkAccessManager* m_networkManager;
};

#endif // IGDBSERVICE_H
