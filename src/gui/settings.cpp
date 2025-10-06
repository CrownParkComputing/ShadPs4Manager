#include "settings.h"
#include "credential_manager.h"
#include <QStandardPaths>
#include <QDir>

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

Settings::Settings() : m_settings("ShadPs4Manager", "Settings") {
    loadSettings();
}

void Settings::loadSettings() {
    m_gameLibraryPath = m_settings.value("paths/gameLibrary", getDefaultGameLibraryPath()).toString();
    m_downloadsPath = m_settings.value("paths/downloads", getDefaultDownloadsPath()).toString();
    // IGDB credentials now loaded from CredentialManager (encrypted storage)
}

void Settings::saveSettings() {
    m_settings.setValue("paths/gameLibrary", m_gameLibraryPath);
    m_settings.setValue("paths/downloads", m_downloadsPath);
    m_settings.setValue("paths/shadps4", getShadPS4Path());
    // IGDB credentials now saved through CredentialManager (encrypted storage)
    m_settings.sync();
}

QString Settings::getGameLibraryPath() const {
    return m_gameLibraryPath;
}

void Settings::setGameLibraryPath(const QString& path) {
    m_gameLibraryPath = path;
    saveSettings();
}

QString Settings::getDownloadsPath() const {
    return m_downloadsPath;
}

void Settings::setDownloadsPath(const QString& path) {
    m_downloadsPath = path;
    saveSettings();
}

QString Settings::getShadPS4Path() const {
    QString path = m_settings.value("paths/shadps4", getDefaultShadPS4Path()).toString();
    return path;
}

void Settings::setShadPS4Path(const QString& path) {
    m_settings.setValue("paths/shadps4", path);
    m_settings.sync();
}

QString Settings::getDefaultGameLibraryPath() const {
    return getDefaultPath("GameLibrary");
}

QString Settings::getDefaultDownloadsPath() const {
    return getDefaultPath("Downloads");
}

QString Settings::getDefaultShadPS4Path() const {
    // Try common installation paths for ShadPS4
    QStringList commonPaths = {
        "/usr/bin/shadps4",
        "/usr/local/bin/shadps4",
        QStandardPaths::findExecutable("shadps4"),
        QDir::homePath() + "/shadps4/shadps4",
        QDir::homePath() + "/.local/bin/shadps4"
    };

    for (const QString& path : commonPaths) {
        QFileInfo info(path);
        if (info.exists() && info.isExecutable()) {
            return path;
        }
    }

    return QString(); // Return empty if not found
}

QString Settings::getDefaultPath(const QString& subfolder) const {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return QDir(documentsPath).filePath("ShadPs4Manager/" + subfolder);
}

bool Settings::isGameLibraryPathValid() const {
    QFileInfo info(m_gameLibraryPath);
    return info.exists() && info.isDir() && info.isWritable();
}

bool Settings::isDownloadsPathValid() const {
    QFileInfo info(m_downloadsPath);
    return info.exists() && info.isDir() && info.isWritable();
}

bool Settings::createGameLibraryDirectory() {
    QDir dir;
    if (!dir.mkpath(m_gameLibraryPath)) {
        return false;
    }
    saveSettings();
    return true;
}

bool Settings::createDownloadsDirectory() {
    QDir dir;
    if (!dir.mkpath(m_downloadsPath)) {
        return false;
    }
    saveSettings();
    return true;
}

QString Settings::getIgdbClientId() const {
    return CredentialManager::instance().getIgdbClientId();
}

void Settings::setIgdbClientId(const QString& clientId) {
    CredentialManager::instance().setIgdbClientId(clientId);
}

QString Settings::getIgdbClientSecret() const {
    return CredentialManager::instance().getIgdbClientSecret();
}

void Settings::setIgdbClientSecret(const QString& clientSecret) {
    CredentialManager::instance().setIgdbClientSecret(clientSecret);
}

bool Settings::hasValidIgdbCredentials() const {
    return CredentialManager::instance().hasValidIgdbCredentials();
}
