#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>

// Include new components
#include "settings.h"
#include "settings_page.h"
#include "game_library.h"
#include "downloads_folder.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        applyStyles();
        connectSignals();
        
        // Initialize settings
        Settings& settings = Settings::instance();
        
        // Set window properties
        setWindowTitle("ShadPs4 Manager - Game Library");
        setMinimumSize(1000, 700);
        resize(1200, 800);
    }

private slots:
    void refreshAllData() {
        gameLibrary->refreshLibrary();
        downloadsFolder->refreshDownloads();
    }

    void openSettings() {
        // Create settings dialog
        QDialog* settingsDialog = new QDialog(this);
        settingsDialog->setWindowTitle("Settings");
        settingsDialog->setModal(true);
        settingsDialog->resize(600, 500);
        
        auto* dialogLayout = new QVBoxLayout(settingsDialog);
        
        // Create new settings page for the dialog
        auto* settingsPageDialog = new SettingsPage();
        dialogLayout->addWidget(settingsPageDialog);
        
        // Add close button
        auto* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        auto* closeButton = new QPushButton("Close");
        closeButton->setObjectName("closeButton");
        connect(closeButton, &QPushButton::clicked, settingsDialog, &QDialog::accept);
        buttonLayout->addWidget(closeButton);
        dialogLayout->addLayout(buttonLayout);
        
        // Connect settings changes
        connect(settingsPageDialog, &SettingsPage::settingsChanged, this, &MainWindow::onSettingsChanged);
        
        settingsDialog->exec();
        settingsDialog->deleteLater();
    }

    void openDownloads() {
        // Create downloads dialog  
        QDialog* downloadsDialog = new QDialog(this);
        downloadsDialog->setWindowTitle("Downloads");
        downloadsDialog->setModal(true);
        downloadsDialog->resize(800, 600);
        
        auto* dialogLayout = new QVBoxLayout(downloadsDialog);
        
        // Create new downloads page for the dialog
        auto* downloadsPageDialog = new DownloadsFolder();
        dialogLayout->addWidget(downloadsPageDialog);
        
        // Add close button
        auto* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        auto* closeButton = new QPushButton("Close");
        closeButton->setObjectName("closeButton");
        connect(closeButton, &QPushButton::clicked, downloadsDialog, &QDialog::accept);
        buttonLayout->addWidget(closeButton);
        dialogLayout->addLayout(buttonLayout);
        
        downloadsDialog->exec();
        downloadsDialog->deleteLater();
    }

    void onSettingsChanged() {
        // Refresh library when settings change (like library path)
        gameLibrary->refreshLibrary();
    }

private:
    void setupUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // Header with menu buttons
        auto* headerLayout = new QHBoxLayout();
        auto* titleLabel = new QLabel("ShadPs4 Manager");
        titleLabel->setObjectName("mainTitle");
        
        // Menu buttons
        auto* menuLayout = new QHBoxLayout();
        
        auto* downloadsButton = new QPushButton("Downloads");
        downloadsButton->setObjectName("menuButton");
        downloadsButton->setToolTip("Open Downloads folder manager");
        
        auto* settingsButton = new QPushButton("Settings");
        settingsButton->setObjectName("menuButton");
        settingsButton->setToolTip("Open Settings");
        
        auto* refreshButton = new QPushButton("Refresh");
        refreshButton->setObjectName("refreshButton");
        refreshButton->setToolTip("Refresh game library");
        
        menuLayout->addWidget(downloadsButton);
        menuLayout->addWidget(settingsButton);
        menuLayout->addWidget(refreshButton);
        
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();
        headerLayout->addLayout(menuLayout);
        
        mainLayout->addLayout(headerLayout);

        // Main content - Game Library (no tabs)
        gameLibrary = new GameLibrary();
        mainLayout->addWidget(gameLibrary);
        
        // Initialize downloads folder for dialogs (not displayed in main window)
        downloadsFolder = new DownloadsFolder();
        downloadsFolder->setParent(this);
        downloadsFolder->hide(); // Keep hidden, only used for dialog creation
        
        // Connect buttons
        connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAllData);
        connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
        connect(downloadsButton, &QPushButton::clicked, this, &MainWindow::openDownloads);
    }

    void applyStyles() {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
            }

            #mainTitle {
                font-size: 24px;
                font-weight: bold;
                color: #4CAF50;
                padding: 10px;
            }

            #refreshButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
            }

            #refreshButton:hover {
                background-color: #45a049;
            }

            #refreshButton:pressed {
                background-color: #3d8b40;
            }

            #menuButton {
                background-color: #2196F3;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
                margin-left: 5px;
            }

            #menuButton:hover {
                background-color: #0b7dda;
            }

            #menuButton:pressed {
                background-color: #0969da;
            }

            #closeButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 12px;
                font-weight: bold;
            }

            #closeButton:hover {
                background-color: #45a049;
            }
        )");
    }

    void connectSignals() {
        // Settings changes will be connected per-dialog in openSettings()
    }

private:
    GameLibrary* gameLibrary;
    DownloadsFolder* downloadsFolder; // Keep for dialog creation
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("ShadPs4 Manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ShadPs4");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "working_gui.moc"