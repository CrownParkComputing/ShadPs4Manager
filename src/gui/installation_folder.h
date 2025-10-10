#ifndef INSTALLATION_FOLDER_H
#define INSTALLATION_FOLDER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTreeWidget>

class InstallationFolder : public QWidget {
    Q_OBJECT

public:
    InstallationFolder(QWidget* parent = nullptr);

public slots:
    void refreshView();

private:
    void setupUI();
    void applyStyles();
    void loadInstalledGames();

    QLabel* gameLibPathLabel;
    QLabel* dlcPathLabel;
    QLabel* baseGamesCountLabel;
    QLabel* updatesCountLabel;
    QLabel* dlcCountValueLabel;
    QTreeWidget* installedGamesTree;
    QPushButton* refreshButton;
};

#endif // INSTALLATION_FOLDER_H
