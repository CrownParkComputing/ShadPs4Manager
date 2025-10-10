#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>

class IgdbService;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    SettingsPage(QWidget* parent = nullptr);

signals:
    void settingsChanged();

public slots:
    void refreshSettings();

private slots:
    void selectGameLibraryPath();
    void selectDownloadsPath();
    void selectShadPS4Path();
    void selectDlcFolderPath();
    void selectPkgExtractorPath();
    void createGameLibraryDirectory();
    void createDownloadsDirectory();
    void createDlcFolderDirectory();
    void resetToDefaults();
    void resetPkgExtractorPath();
    void saveIgdbCredentials();
    void setDefaultIgdbCredentials();
    void testIgdbConnection();
    void showIgdbTestResults(const QString& result);

private:
    void setupUI();
    void updatePathDisplay();
    void applyStyles();

    QLabel* gameLibraryPathLabel;
    QLabel* downloadsPathLabel;
    QLabel* shadps4PathLabel;
    QLabel* dlcFolderPathLabel;
    QLabel* pkgExtractorPathLabel;
    QLabel* gameLibraryStatusLabel;
    QLabel* downloadsStatusLabel;
    QLabel* shadps4StatusLabel;
    QLabel* dlcFolderStatusLabel;
    QLabel* pkgExtractorStatusLabel;

    QCheckBox* useSystemShadPS4Checkbox;

    QLineEdit* igdbClientIdEdit;
    QLineEdit* igdbClientSecretEdit;

    QString gameLibraryPath;
    QString downloadsPath;
    QString shadps4Path;
    QString dlcFolderPath;
    
    IgdbService* m_igdbService;
};

#endif // SETTINGS_PAGE_H
