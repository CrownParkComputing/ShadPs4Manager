#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>

// Common data structures used across GUI components

struct IgdbGame {
    int id = 0;
    QString name;
    QString summary;
    int cover = 0;
    QList<int> screenshots;
    QList<int> videos;

    static IgdbGame fromJson(const QJsonObject& json) {
        IgdbGame game;
        game.id = json["id"].toInt();
        game.name = json["name"].toString();
        game.summary = json["summary"].toString();
        game.cover = json["cover"].toInt();

        // Parse screenshots array
        if (json.contains("screenshots") && json["screenshots"].isArray()) {
            QJsonArray screenshotsArray = json["screenshots"].toArray();
            for (const QJsonValue& value : screenshotsArray) {
                game.screenshots.append(value.toInt());
            }
        }

        return game;
    }
};

// Forward declaration for GameEntry (defined in gamecard.h)
struct GameEntry;

#endif // COMMON_TYPES_H
