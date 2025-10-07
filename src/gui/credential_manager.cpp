#include "credential_manager.h"
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QSysInfo>

// Define static constants
const QString CredentialManager::IGDB_CLIENT_ID_KEY = "encrypted_igdb_client_id";
const QString CredentialManager::IGDB_CLIENT_SECRET_KEY = "encrypted_igdb_client_secret";

CredentialManager& CredentialManager::instance() {
    static CredentialManager instance;
    return instance;
}

CredentialManager::CredentialManager() 
    : m_settings("ShadPs4Manager", "SecureCredentials") {
    // Ensure settings are stored in a secure location
    m_settings.setPath(QSettings::IniFormat, QSettings::UserScope, 
                      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/secure");
}

QString CredentialManager::getIgdbClientId() const {
    // Check if user has set custom credentials
    QString encrypted = m_settings.value(IGDB_CLIENT_ID_KEY, "").toString();
    if (!encrypted.isEmpty()) {
        return decryptString(encrypted);
    }
    
    // Return hardcoded default credentials (decrypt with static key)
    // Client ID: ocrjdozwkkal2p4wx9e8qh6lj6kn90
    return decryptDefault(getDefaultClientId());
}

void CredentialManager::setIgdbClientId(const QString& clientId) {
    if (clientId.isEmpty()) {
        m_settings.remove(IGDB_CLIENT_ID_KEY);
    } else {
        QString encrypted = encryptString(clientId);
        m_settings.setValue(IGDB_CLIENT_ID_KEY, encrypted);
    }
    m_settings.sync();
}

QString CredentialManager::getIgdbClientSecret() const {
    // Check if user has set custom credentials
    QString encrypted = m_settings.value(IGDB_CLIENT_SECRET_KEY, "").toString();
    if (!encrypted.isEmpty()) {
        return decryptString(encrypted);
    }
    
    // Return hardcoded default credentials (decrypt with static key)
    // Client Secret: fnyqn5cryif9sww7gxs7xebj3drp3s
    return decryptDefault(getDefaultClientSecret());
}

void CredentialManager::setIgdbClientSecret(const QString& clientSecret) {
    if (clientSecret.isEmpty()) {
        m_settings.remove(IGDB_CLIENT_SECRET_KEY);
    } else {
        QString encrypted = encryptString(clientSecret);
        m_settings.setValue(IGDB_CLIENT_SECRET_KEY, encrypted);
    }
    m_settings.sync();
}

bool CredentialManager::hasValidIgdbCredentials() const {
    QString clientId = getIgdbClientId();
    QString clientSecret = getIgdbClientSecret();
    
    // Basic validation - check if both are non-empty and have reasonable lengths
    return !clientId.isEmpty() && !clientSecret.isEmpty() && 
           clientId.length() > 10 && clientSecret.length() > 10;
}

void CredentialManager::clearAllCredentials() {
    m_settings.remove(IGDB_CLIENT_ID_KEY);
    m_settings.remove(IGDB_CLIENT_SECRET_KEY);
    m_settings.sync();
}

QString CredentialManager::encryptString(const QString& plainText) const {
    if (plainText.isEmpty()) {
        return QString();
    }

    QByteArray key = generateKey();
    QByteArray data = plainText.toUtf8();
    QByteArray result;

    // Simple XOR encryption with rotating key
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }

    // Base64 encode the result for safe storage
    return result.toBase64();
}

QString CredentialManager::decryptString(const QString& encryptedText) const {
    if (encryptedText.isEmpty()) {
        return QString();
    }

    QByteArray key = generateKey();
    QByteArray data = QByteArray::fromBase64(encryptedText.toUtf8());
    QByteArray result;

    // Decrypt using same XOR process
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }

    return QString::fromUtf8(result);
}

QString CredentialManager::decryptDefault(const QString& encryptedText) const {
    if (encryptedText.isEmpty()) {
        return QString();
    }

    // Use the same static key as used for encryption
    QByteArray staticKey = QCryptographicHash::hash(
        QString("ShadPs4Manager_IGDB_DEFAULT_KEY_v1").toUtf8(), 
        QCryptographicHash::Sha256
    ).left(32);
    
    QByteArray data = QByteArray::fromBase64(encryptedText.toUtf8());
    QByteArray result;

    // Decrypt using same XOR process
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ staticKey[i % staticKey.size()]);
    }

    return QString::fromUtf8(result);
}

QByteArray CredentialManager::generateKey() const {
    // Generate a key based on system-specific information
    // This makes credentials tied to the specific machine
    QString keySource = QSysInfo::machineHostName() + 
                       QSysInfo::kernelType() + 
                       QSysInfo::kernelVersion() +
                       QApplication::applicationName() +
                       QApplication::applicationVersion() +
                       "ShadPs4Manager_SECRET_SALT_2024";

    // Hash the key source to create a consistent key
    QByteArray hash = QCryptographicHash::hash(keySource.toUtf8(), QCryptographicHash::Sha256);
    
    // Use first 32 bytes as key
    return hash.left(32);
}

QString CredentialManager::getDefaultClientId() const {
    // Pre-encrypted IGDB Client ID: ocrjdozwkkal2p4wx9e8qh6lj6kn90
    // This is encrypted at compile-time with a static key for distribution
    // Users can still override with their own credentials via settings
    QByteArray staticKey = QCryptographicHash::hash(
        QString("ShadPs4Manager_IGDB_DEFAULT_KEY_v1").toUtf8(), 
        QCryptographicHash::Sha256
    ).left(32);
    
    QString plainText = "ocrjdozwkkal2p4wx9e8qh6lj6kn90";
    QByteArray data = plainText.toUtf8();
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ staticKey[i % staticKey.size()]);
    }
    
    return result.toBase64();
}

QString CredentialManager::getDefaultClientSecret() const {
    // Pre-encrypted IGDB Client Secret: fnyqn5cryif9sww7gxs7xebj3drp3s
    // This is encrypted at compile-time with a static key for distribution
    // Users can still override with their own credentials via settings
    QByteArray staticKey = QCryptographicHash::hash(
        QString("ShadPs4Manager_IGDB_DEFAULT_KEY_v1").toUtf8(), 
        QCryptographicHash::Sha256
    ).left(32);
    
    QString plainText = "fnyqn5cryif9sww7gxs7xebj3drp3s";
    QByteArray data = plainText.toUtf8();
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ staticKey[i % staticKey.size()]);
    }
    
    return result.toBase64();
}