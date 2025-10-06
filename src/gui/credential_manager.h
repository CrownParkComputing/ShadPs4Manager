#ifndef CREDENTIAL_MANAGER_H
#define CREDENTIAL_MANAGER_H

#include <QString>
#include <QByteArray>
#include <QSettings>

class CredentialManager {
public:
    static CredentialManager& instance();

    // IGDB credential management
    QString getIgdbClientId() const;
    void setIgdbClientId(const QString& clientId);
    
    QString getIgdbClientSecret() const;
    void setIgdbClientSecret(const QString& clientSecret);

    // Check if credentials are properly configured
    bool hasValidIgdbCredentials() const;

    // Clear all stored credentials
    void clearAllCredentials();

private:
    CredentialManager();
    ~CredentialManager() = default;
    CredentialManager(const CredentialManager&) = delete;
    CredentialManager& operator=(const CredentialManager&) = delete;

    // Encryption/Decryption methods
    QString encryptString(const QString& plainText) const;
    QString decryptString(const QString& encryptedText) const;
    
    // Generate encryption key based on system info
    QByteArray generateKey() const;
    
    QSettings m_settings;
    static const QString IGDB_CLIENT_ID_KEY;
    static const QString IGDB_CLIENT_SECRET_KEY;
};

#endif // CREDENTIAL_MANAGER_H