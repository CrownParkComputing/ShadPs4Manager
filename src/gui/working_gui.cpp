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
        settingsPage->refreshSettings();
    }

    void onSettingsChanged() {
        // Refresh library when settings change (like library path)
        gameLibrary->refreshLibrary();
    }

private:
    void setupUI() {
        auto* mainLayout = new QVBoxLayout(this);

        // Header
        auto* headerLayout = new QHBoxLayout();
        auto* titleLabel = new QLabel("ShadPs4 Manager");
        titleLabel->setObjectName("mainTitle");
        
        auto* refreshButton = new QPushButton("Refresh All");
        refreshButton->setObjectName("refreshButton");
        refreshButton->setToolTip("Refresh all data in all tabs");
        
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(refreshButton);
        
        mainLayout->addLayout(headerLayout);

        // Tab widget
        tabWidget = new QTabWidget();
        
        // Game Library tab
        gameLibrary = new GameLibrary();
        tabWidget->addTab(gameLibrary, "Game Library");
        
        // Downloads tab  
        downloadsFolder = new DownloadsFolder();
        tabWidget->addTab(downloadsFolder, "Downloads");
        
        // Settings tab
        settingsPage = new SettingsPage();
        tabWidget->addTab(settingsPage, "Settings");
        
        mainLayout->addWidget(tabWidget);
        
        // Connect the refresh button
        connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAllData);
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

            QTabWidget::pane {
                border: 1px solid #555555;
                background-color: #353535;
            }

            QTabBar::tab {
                background-color: #404040;
                color: #ffffff;
                padding: 8px 16px;
                margin-right: 2px;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
            }

            QTabBar::tab:selected {
                background-color: #4CAF50;
                color: #ffffff;
            }

            QTabBar::tab:hover {
                background-color: #505050;
            }

            QTabBar::tab:!selected {
                margin-top: 2px;
            }
        )");
    }

    void connectSignals() {
        // Connect settings changes to refresh library
        connect(settingsPage, &SettingsPage::settingsChanged, this, &MainWindow::onSettingsChanged);
    }

private:
    QTabWidget* tabWidget;
    GameLibrary* gameLibrary;
    DownloadsFolder* downloadsFolder;
    SettingsPage* settingsPage;
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