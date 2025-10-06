#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QSettings>
#include <QDir>

class Settings {
public:
    static Settings& instance();

    // Game library settings
    QString getGameLibraryPath() const;
    void setGameLibraryPath(const QString& path);

    // Downloads folder settings
    QString getDownloadsPath() const;
    void setDownloadsPath(const QString& path);

    // ShadPS4 executable settings
    QString getShadPS4Path() const;
    void setShadPS4Path(const QString& path);

    // Default paths
    QString getDefaultGameLibraryPath() const;
    QString getDefaultDownloadsPath() const;
    QString getDefaultShadPS4Path() const;

    // Validation
    bool isGameLibraryPathValid() const;
    bool isDownloadsPathValid() const;

    // Directory creation
    bool createGameLibraryDirectory();
    bool createDownloadsDirectory();

    // IGDB API credentials (now handled through encrypted storage)
    QString getIgdbClientId() const;
    void setIgdbClientId(const QString& clientId);
    QString getIgdbClientSecret() const;
    void setIgdbClientSecret(const QString& clientSecret);
    bool hasValidIgdbCredentials() const;

private:
    Settings();
    ~Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    QSettings m_settings;
    QString m_gameLibraryPath;
    QString m_downloadsPath;
    // IGDB credentials removed - now handled by CredentialManager

    void loadSettings();
    void saveSettings();
    QString getDefaultPath(const QString& subfolder) const;
};

#endif // SETTINGS_H
