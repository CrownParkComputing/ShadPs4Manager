#include "settings_page.h"
#include "settings.h"
#include "igdbservice.h"

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {
    // Initialize IGDB service
    m_igdbService = new IgdbService(this);
    connect(m_igdbService, &IgdbService::testResultsReady, this, &SettingsPage::showIgdbTestResults);
    
    setupUI();
    applyStyles();
    refreshSettings();
}

void SettingsPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Header
    auto* headerLabel = new QLabel("Settings - Configure Paths");
    headerLabel->setObjectName("settingsHeader");
    mainLayout->addWidget(headerLabel);

    // Game Library Section
    auto* gameLibraryGroup = new QGroupBox("Game Library Path");
    auto* gameLibraryLayout = new QVBoxLayout(gameLibraryGroup);

    auto* gameLibraryRow = new QHBoxLayout();
    auto* selectGameLibraryButton = new QPushButton("Browse...");
    auto* createGameLibraryButton = new QPushButton("Create Directory");
    gameLibraryPathLabel = new QLabel();

    gameLibraryRow->addWidget(selectGameLibraryButton);
    gameLibraryRow->addWidget(createGameLibraryButton);
    gameLibraryRow->addWidget(gameLibraryPathLabel, 1);

    gameLibraryLayout->addLayout(gameLibraryRow);
    gameLibraryStatusLabel = new QLabel();
    gameLibraryLayout->addWidget(gameLibraryStatusLabel);

    mainLayout->addWidget(gameLibraryGroup);

    // Downloads Section
    auto* downloadsGroup = new QGroupBox("Downloads Folder Path");
    auto* downloadsLayout = new QVBoxLayout(downloadsGroup);

    auto* downloadsRow = new QHBoxLayout();
    auto* selectDownloadsButton = new QPushButton("Browse...");
    auto* createDownloadsButton = new QPushButton("Create Directory");
    downloadsPathLabel = new QLabel();

    downloadsRow->addWidget(selectDownloadsButton);
    downloadsRow->addWidget(createDownloadsButton);
    downloadsRow->addWidget(downloadsPathLabel, 1);

    downloadsLayout->addLayout(downloadsRow);
    downloadsStatusLabel = new QLabel();
    downloadsLayout->addWidget(downloadsStatusLabel);

    mainLayout->addWidget(downloadsGroup);

    // IGDB API Section
    auto* igdbGroup = new QGroupBox("IGDB API Configuration");
    auto* igdbLayout = new QVBoxLayout(igdbGroup);

    // Client ID field
    auto* clientIdLayout = new QHBoxLayout();
    auto* clientIdLabel = new QLabel("Client ID:");
    igdbClientIdEdit = new QLineEdit();
    igdbClientIdEdit->setPlaceholderText("Enter your IGDB Client ID...");
    clientIdLayout->addWidget(clientIdLabel);
    clientIdLayout->addWidget(igdbClientIdEdit, 1);

    igdbLayout->addLayout(clientIdLayout);

    // Client Secret field
    auto* clientSecretLayout = new QHBoxLayout();
    auto* clientSecretLabel = new QLabel("Client Secret:");
    igdbClientSecretEdit = new QLineEdit();
    igdbClientSecretEdit->setPlaceholderText("Enter your IGDB Client Secret...");
    igdbClientSecretEdit->setEchoMode(QLineEdit::Password); // Hide the secret
    clientSecretLayout->addWidget(clientSecretLabel);
    clientSecretLayout->addWidget(igdbClientSecretEdit, 1);

    igdbLayout->addLayout(clientSecretLayout);

    // IGDB info
    auto* igdbInfoLabel = new QLabel("Get your IGDB API credentials from: https://api.igdb.com/");
    igdbInfoLabel->setStyleSheet("color: #888888; font-size: 11px;");
    igdbLayout->addWidget(igdbInfoLabel);

    // IGDB test button
    auto* testIgdbButton = new QPushButton("Test Connection");
    testIgdbButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; } QPushButton:hover { background-color: #0b7dda; }");
    igdbLayout->addWidget(testIgdbButton);

    mainLayout->addWidget(igdbGroup);

    // Bottom buttons
    auto* buttonLayout = new QHBoxLayout();

    auto* resetButton = new QPushButton("Reset to Defaults");
    auto* refreshButton = new QPushButton("Refresh");

    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(selectGameLibraryButton, &QPushButton::clicked, this, &SettingsPage::selectGameLibraryPath);
    connect(createGameLibraryButton, &QPushButton::clicked, this, &SettingsPage::createGameLibraryDirectory);
    connect(selectDownloadsButton, &QPushButton::clicked, this, &SettingsPage::selectDownloadsPath);
    connect(createDownloadsButton, &QPushButton::clicked, this, &SettingsPage::createDownloadsDirectory);
    connect(resetButton, &QPushButton::clicked, this, &SettingsPage::resetToDefaults);
    connect(refreshButton, &QPushButton::clicked, this, &SettingsPage::refreshSettings);

    // Connect IGDB credential editing and testing
    connect(igdbClientIdEdit, &QLineEdit::textChanged, this, &SettingsPage::saveIgdbCredentials);
    connect(igdbClientSecretEdit, &QLineEdit::textChanged, this, &SettingsPage::saveIgdbCredentials);
    connect(testIgdbButton, &QPushButton::clicked, this, &SettingsPage::testIgdbConnection);
}

void SettingsPage::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #settingsHeader {
            font-size: 16px;
            font-weight: bold;
            color: #4CAF50;
            padding: 10px;
            margin-bottom: 15px;
        }

        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #353535;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #4CAF50;
            background-color: #353535;
        }

        QPushButton {
            background-color: #404040;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 13px;
            min-width: 100px;
        }

        QPushButton:hover {
            background-color: #505050;
            border-color: #777777;
        }

        QPushButton:pressed {
            background-color: #353535;
        }

        QLabel {
            color: #cccccc;
            font-size: 12px;
            padding: 5px;
        }

        #statusValid {
            color: #4CAF50;
        }

        #statusInvalid {
            color: #f44336;
        }
    )");
}

void SettingsPage::refreshSettings() {
    Settings& settings = Settings::instance();

    gameLibraryPath = settings.getGameLibraryPath();
    downloadsPath = settings.getDownloadsPath();

    // Load IGDB credentials
    igdbClientIdEdit->setText(settings.getIgdbClientId());
    igdbClientSecretEdit->setText(settings.getIgdbClientSecret());

    updatePathDisplay();
    emit settingsChanged();
}

void SettingsPage::updatePathDisplay() {
    Settings& settings = Settings::instance();

    // Update game library path display
    gameLibraryPathLabel->setText(gameLibraryPath);
    if (settings.isGameLibraryPathValid()) {
        gameLibraryStatusLabel->setText("✓ Directory exists and is writable");
        gameLibraryStatusLabel->setObjectName("statusValid");
    } else {
        gameLibraryStatusLabel->setText("✗ Directory does not exist or is not writable");
        gameLibraryStatusLabel->setObjectName("statusInvalid");
    }

    // Update downloads path display
    downloadsPathLabel->setText(downloadsPath);
    if (settings.isDownloadsPathValid()) {
        downloadsStatusLabel->setText("✓ Directory exists and is writable");
        downloadsStatusLabel->setObjectName("statusValid");
    } else {
        downloadsStatusLabel->setText("✗ Directory does not exist or is not writable");
        downloadsStatusLabel->setObjectName("statusInvalid");
    }

    // Reapply styles to update colors
    applyStyles();
}

void SettingsPage::selectGameLibraryPath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Game Library Directory",
        Settings::instance().getGameLibraryPath()
    );

    if (!dir.isEmpty()) {
        Settings::instance().setGameLibraryPath(dir);
        refreshSettings();
    }
}

void SettingsPage::selectDownloadsPath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Downloads Directory",
        Settings::instance().getDownloadsPath()
    );

    if (!dir.isEmpty()) {
        Settings::instance().setDownloadsPath(dir);
        refreshSettings();
    }
}

void SettingsPage::createGameLibraryDirectory() {
    if (Settings::instance().createGameLibraryDirectory()) {
        QMessageBox::information(this, "Success",
            "Game Library directory created successfully!");
        refreshSettings();
    } else {
        QMessageBox::critical(this, "Error",
            "Failed to create Game Library directory!");
    }
}

void SettingsPage::createDownloadsDirectory() {
    if (Settings::instance().createDownloadsDirectory()) {
        QMessageBox::information(this, "Success",
            "Downloads directory created successfully!");
        refreshSettings();
    } else {
        QMessageBox::critical(this, "Error",
            "Failed to create Downloads directory!");
    }
}

void SettingsPage::resetToDefaults() {
    Settings& settings = Settings::instance();

    settings.setGameLibraryPath(settings.getDefaultGameLibraryPath());
    settings.setDownloadsPath(settings.getDefaultDownloadsPath());

    refreshSettings();

    QMessageBox::information(this, "Reset Complete",
        "Settings have been reset to default paths.");
}

void SettingsPage::saveIgdbCredentials() {
    Settings& settings = Settings::instance();
    settings.setIgdbClientId(igdbClientIdEdit->text());
    settings.setIgdbClientSecret(igdbClientSecretEdit->text());
}

void SettingsPage::testIgdbConnection() {
    // Update credentials first
    saveIgdbCredentials();

    Settings& settings = Settings::instance();
    QString clientId = settings.getIgdbClientId();
    QString clientSecret = settings.getIgdbClientSecret();

    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        QMessageBox::warning(this, "IGDB Test",
            "Please enter both Client ID and Client Secret before testing the connection.");
        return;
    }

    // Disable the test button temporarily
    QPushButton* testButton = qobject_cast<QPushButton*>(sender());
    if (testButton) {
        testButton->setText("Testing...");
        testButton->setEnabled(false);
    }

    // Set credentials and start the test
    m_igdbService->setCredentials(clientId, clientSecret);
    m_igdbService->testIgdbConnection();
}

void SettingsPage::showIgdbTestResults(const QString& result) {
    // Re-enable the test button
    QList<QPushButton*> widgets = findChildren<QPushButton*>();
    for (QPushButton* widget : widgets) {
        QPushButton* button = qobject_cast<QPushButton*>(widget);
        if (button && button->text() == "Testing...") {
            button->setText("Test Connection");
            button->setEnabled(true);
        }
    }

    // Show the test results in a message box
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("IGDB Connection Test Results");
    msgBox.setText(result);
    msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: #2b2b2b;
            color: #ffffff;
        }

        QPushButton {
            background-color: #404040;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 8px 16px;
            color: #ffffff;
        }

        QPushButton:hover {
            background-color: #505050;
        }
    )");

    msgBox.exec();
}
