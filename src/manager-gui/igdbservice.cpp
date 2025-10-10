#include "igdbservice.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QTimer>
#include <QEventLoop>



IgdbService::IgdbService(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &IgdbService::onTokenReceived);
}

void IgdbService::setCredentials(const QString& clientId, const QString& clientSecret) {
    m_clientId = clientId;
    m_clientSecret = clientSecret;
}

void IgdbService::searchGames(const QString& query) {
    QString token = getAccessToken();
    if (token.isEmpty()) {
        // Store the query to retry after getting token
        m_pendingGameName = query;
        requestAccessToken();
        return;
    }

    QUrl url("https://api.igdb.com/v4/games");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    QString queryBody = QString("search \"%1\"; fields name,cover,screenshots,videos,summary; where platforms = (48); limit 20;").arg(query);

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onSearchFinished);
}

void IgdbService::fetchGameDetails(int gameId) {
    QString token = getAccessToken();
    if (token.isEmpty()) {
        requestAccessToken();
        return;
    }

    QUrl url("https://api.igdb.com/v4/games");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    QString queryBody = QString("fields name,cover,screenshots,videos,summary; where id = %1;").arg(gameId);

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onGameDetailsFinished);
}

void IgdbService::fetchCoverImage(int coverId) {
    QString token = getAccessToken();
    if (token.isEmpty()) {
        requestAccessToken();
        return;
    }

    QUrl url("https://api.igdb.com/v4/covers");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    QString queryBody = QString("fields image_id, url; where id = %1;").arg(coverId);

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onCoverImageFinished);
}

void IgdbService::fetchScreenshots(const QList<int>& screenshotIds) {
    QString token = getAccessToken();
    if (token.isEmpty()) {
        requestAccessToken();
        return;
    }

    QUrl url("https://api.igdb.com/v4/screenshots");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    QStringList idStrings;
    for (int id : screenshotIds) {
        idStrings.append(QString::number(id));
    }

    QString queryBody = QString("fields image_id, url; where id = (%1); limit %2;")
                              .arg(idStrings.join(","))
                              .arg(screenshotIds.size());

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onScreenshotsFinished);
}

QString IgdbService::getAccessToken() {
    if (m_accessToken.isEmpty() || m_tokenExpiry < QDateTime::currentDateTime()) {
        return QString();
    }
    return m_accessToken;
}

void IgdbService::requestAccessToken() {
    QUrl url("https://id.twitch.tv/oauth2/token");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString data = QString("client_id=%1&client_secret=%2&grant_type=client_credentials")
                          .arg(m_clientId)
                          .arg(m_clientSecret);

    QNetworkReply* reply = m_networkManager->post(request, data.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onTokenReceived);
}

void IgdbService::onTokenReceived() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Token request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();

    m_accessToken = obj["access_token"].toString();
    int expiresIn = obj["expires_in"].toInt();

    if (!m_accessToken.isEmpty()) {
        m_tokenExpiry = QDateTime::currentDateTime().addSecs(expiresIn);

        // If we have a pending game name, process it now that we have a token
        if (!m_pendingGameName.isEmpty()) {
            QString gameName = m_pendingGameName;
            m_pendingGameName.clear();

            // Handle test connection differently
            if (gameName == "__TEST_CONNECTION__") {
                performTestSearch();
            } else {
                // Re-call searchGames now that we have a token
                searchGames(gameName);
            }
        }
    }
}

void IgdbService::onSearchFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Search request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray array = doc.array();

    QList<IgdbGame> games;
    for (const QJsonValue& value : array) {
        QJsonObject gameObj = value.toObject();
        games.append(IgdbGame::fromJson(gameObj));
    }

    emit searchResultsReady(games);
}

void IgdbService::onGameDetailsFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Game details request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray array = doc.array();

    if (!array.isEmpty()) {
        QJsonObject gameObj = array.first().toObject();
        IgdbGame game = IgdbGame::fromJson(gameObj);
        emit gameDetailsReady(game);
    }
}

void IgdbService::onCoverImageFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Cover image request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray array = doc.array();

    if (!array.isEmpty()) {
        QJsonObject coverObj = array.first().toObject();
        QString imageId = coverObj["image_id"].toString();
        QString url = coverObj["url"].toString();

        if (url.startsWith("//")) {
            url = "https:" + url;
        }
        url = url.replace("/t_thumb/", "/t_cover_big/");

        emit coverImageReady(coverObj["id"].toInt(), url);
    }
}

void IgdbService::onScreenshotsFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Screenshots request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray array = doc.array();

    QList<QPair<int, QString>> screenshots;
    for (const QJsonValue& value : array) {
        QJsonObject screenshotObj = value.toObject();
        QString imageId = screenshotObj["image_id"].toString();
        QString url = screenshotObj["url"].toString();

        if (url.startsWith("//")) {
            url = "https:" + url;
        }
        url = url.replace("/t_thumb/", "/t_screenshot_big/");

        screenshots.append(qMakePair(screenshotObj["id"].toInt(), url));
    }

    emit screenshotsReady(screenshots);
}

void IgdbService::downloadMetadataForGame(const QString& gameName) {
    // First search for the game
    QString token = getAccessToken();
    if (token.isEmpty()) {
        requestAccessToken();
        // Store the game name for later use after token is received
        m_pendingGameName = gameName;
        return;
    }

    performGameSearch(gameName);
}

void IgdbService::performGameSearch(const QString& gameName) {
    QUrl url("https://api.igdb.com/v4/games");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());

    // Search query with platform filter for PS4 (platform ID 48)
    QString queryBody = QString("search \"%1\"; fields name,cover,screenshots,videos,summary; where platforms = (48); limit 5;").arg(gameName);

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onAutoSearchFinished);
}

void IgdbService::onAutoSearchFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Auto search request failed: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray array = doc.array();

    if (array.isEmpty()) {
        emit errorOccurred("No games found for the given name");
        return;
    }

    // Get the first (best match) game
    QJsonObject gameObj = array.first().toObject();
    IgdbGame game = IgdbGame::fromJson(gameObj);

    // Emit the game details for the UI to handle
    emit gameDetailsReady(game);

    // If the game has a cover image, fetch it
    if (game.cover > 0) {
        fetchCoverImage(game.cover);
    }

    // If the game has screenshots, fetch them
    if (!game.screenshots.isEmpty()) {
        fetchScreenshots(game.screenshots);
    }
}

void IgdbService::testIgdbConnection() {
    if (m_clientId.isEmpty() || m_clientSecret.isEmpty()) {
        emit testResultsReady("❌ IGDB credentials not configured. Please set Client ID and Client Secret in Settings.");
        return;
    }

    QString token = getAccessToken();
    if (token.isEmpty()) {
        // Try to get a new token for testing
        requestAccessToken();
        // Store that we're testing so we can show results after token is received
        m_pendingGameName = "__TEST_CONNECTION__";
        return;
    }

    // Perform a simple test search
    performTestSearch();
}

void IgdbService::performTestSearch() {
    QUrl url("https://api.igdb.com/v4/games");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-ID", m_clientId.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());

    // Simple test query - search for a common game that should exist
    QString queryBody = QString("fields name,cover; search \"The Witcher\"; where platforms = (48); limit 1;");

    QNetworkReply* reply = m_networkManager->post(request, queryBody.toUtf8());
    connect(reply, &QNetworkReply::finished, this, &IgdbService::onTestFinished);
}

void IgdbService::onTestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    QString testResult;

    if (reply->error() != QNetworkReply::NoError) {
        testResult = QString("❌ IGDB API Test Failed:\nError: %1\n\nTroubleshooting:\n• Check your Client ID and Client Secret\n• Verify your credentials are correct\n• Ensure you have a valid IGDB account\n• Check your internet connection").arg(reply->errorString());
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray array = doc.array();

        if (array.isEmpty()) {
            testResult = "⚠️ IGDB API Test Warning:\nNo games found in test search.\n\nThis might indicate:\n• Your credentials are valid but no PS4 games found\n• The search query returned no results\n• API rate limiting (try again later)";
        } else {
            QJsonObject gameObj = array.first().toObject();
            QString gameName = gameObj["name"].toString();
            testResult = QString("✅ IGDB API Test Successful!\n\nFound game: %1\n\nYour credentials are working correctly and you can:\n• Search for games\n• Download cover images\n• Fetch screenshots\n• Update game metadata").arg(gameName);
        }
    }

    emit testResultsReady(testResult);
}
