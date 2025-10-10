#include "settings_page.h"
#include "settings.h"
#include "credential_manager.h"
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

    // Create scroll area for all settings
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    auto* scrollContent = new QWidget();
    auto* contentLayout = new QVBoxLayout(scrollContent);
    scrollContent->setLayout(contentLayout);
    
    // Create 2-column grid layout for settings groups
    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);
    
    // LEFT COLUMN (Column 0)

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

    gridLayout->addWidget(gameLibraryGroup, 0, 0); // Row 0, Column 0

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

    gridLayout->addWidget(downloadsGroup, 1, 0); // Row 1, Column 0

    // DLC Folder Section
    auto* dlcFolderGroup = new QGroupBox("DLC Folder Path");
    auto* dlcFolderLayout = new QVBoxLayout(dlcFolderGroup);

    auto* dlcFolderRow = new QHBoxLayout();
    auto* selectDlcFolderButton = new QPushButton("Browse...");
    auto* createDlcFolderButton = new QPushButton("Create Directory");
    dlcFolderPathLabel = new QLabel();

    dlcFolderRow->addWidget(selectDlcFolderButton);
    dlcFolderRow->addWidget(createDlcFolderButton);
    dlcFolderRow->addWidget(dlcFolderPathLabel, 1);

    dlcFolderLayout->addLayout(dlcFolderRow);
    dlcFolderStatusLabel = new QLabel();
    dlcFolderLayout->addWidget(dlcFolderStatusLabel);

    gridLayout->addWidget(dlcFolderGroup, 2, 0); // Row 2, Column 0
    
    // RIGHT COLUMN (Column 1)

    // ShadPS4 Executable Section
    auto* shadps4Group = new QGroupBox("ShadPS4 Emulator Path");
    auto* shadps4Layout = new QVBoxLayout(shadps4Group);

    // Checkbox for using system shadPS4
    useSystemShadPS4Checkbox = new QCheckBox("Use System-Installed ShadPS4 (from PATH)");
    shadps4Layout->addWidget(useSystemShadPS4Checkbox);

    auto* shadps4Row = new QHBoxLayout();
    auto* selectShadPS4Button = new QPushButton("Browse...");
    shadps4PathLabel = new QLabel();

    shadps4Row->addWidget(selectShadPS4Button);
    shadps4Row->addWidget(shadps4PathLabel, 1);

    shadps4Layout->addLayout(shadps4Row);
    shadps4StatusLabel = new QLabel();
    shadps4Layout->addWidget(shadps4StatusLabel);

    gridLayout->addWidget(shadps4Group, 0, 1); // Row 0, Column 1

    // PKG Extractor Tool Section
    auto* pkgExtractorGroup = new QGroupBox("PKG Extractor Tool Path");
    auto* pkgExtractorLayout = new QVBoxLayout(pkgExtractorGroup);
    
    auto* pkgExtractorInfoLabel = new QLabel(
        "⚠️ Required for installing games."
    );
    pkgExtractorInfoLabel->setWordWrap(true);
    pkgExtractorInfoLabel->setStyleSheet("color: #FF9800; font-size: 10px; padding: 2px;");
    pkgExtractorLayout->addWidget(pkgExtractorInfoLabel);

    auto* pkgExtractorRow = new QHBoxLayout();
    auto* selectPkgExtractorButton = new QPushButton("Browse...");
    auto* resetPkgExtractorButton = new QPushButton("Reset");
    pkgExtractorPathLabel = new QLabel();

    pkgExtractorRow->addWidget(selectPkgExtractorButton);
    pkgExtractorRow->addWidget(resetPkgExtractorButton);
    pkgExtractorRow->addWidget(pkgExtractorPathLabel, 1);

    pkgExtractorLayout->addLayout(pkgExtractorRow);
    pkgExtractorStatusLabel = new QLabel();
    pkgExtractorLayout->addWidget(pkgExtractorStatusLabel);

    gridLayout->addWidget(pkgExtractorGroup, 1, 1); // Row 1, Column 1

    // IGDB API Section (spans both columns)
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

    // IGDB buttons
    auto* igdbButtonLayout = new QHBoxLayout();
    auto* setDefaultCredsButton = new QPushButton("Use Default Credentials");
    auto* clearCredsButton = new QPushButton("Clear Stored Credentials");
    auto* testIgdbButton = new QPushButton("Test Connection");
    
    setDefaultCredsButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; } QPushButton:hover { background-color: #45a049; }");
    clearCredsButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; } QPushButton:hover { background-color: #e68900; }");
    testIgdbButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-size: 11px; } QPushButton:hover { background-color: #0b7dda; }");
    
    igdbButtonLayout->addWidget(setDefaultCredsButton);
    igdbButtonLayout->addWidget(clearCredsButton);
    igdbButtonLayout->addWidget(testIgdbButton);
    igdbButtonLayout->addStretch();
    igdbLayout->addLayout(igdbButtonLayout);

    gridLayout->addWidget(igdbGroup, 3, 0, 1, 2); // Row 3, spans both columns
    
    // Add the grid layout to the content layout
    contentLayout->addLayout(gridLayout);

    // Bottom buttons
    auto* buttonLayout = new QHBoxLayout();

    auto* resetButton = new QPushButton("Reset to Defaults");
    auto* refreshButton = new QPushButton("Refresh");

    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    contentLayout->addLayout(buttonLayout);
    
    // Add stretch at the end of content
    contentLayout->addStretch();
    
    // Set the scroll area content and add it to main layout
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // Connect signals
    connect(selectGameLibraryButton, &QPushButton::clicked, this, &SettingsPage::selectGameLibraryPath);
    connect(createGameLibraryButton, &QPushButton::clicked, this, &SettingsPage::createGameLibraryDirectory);
    connect(selectDownloadsButton, &QPushButton::clicked, this, &SettingsPage::selectDownloadsPath);
    connect(createDownloadsButton, &QPushButton::clicked, this, &SettingsPage::createDownloadsDirectory);
    connect(selectShadPS4Button, &QPushButton::clicked, this, &SettingsPage::selectShadPS4Path);
    connect(useSystemShadPS4Checkbox, &QCheckBox::toggled, this, [this](bool checked) {
        Settings::instance().setUseSystemShadPS4(checked);
        refreshSettings();
    });
    connect(selectDlcFolderButton, &QPushButton::clicked, this, &SettingsPage::selectDlcFolderPath);
    connect(createDlcFolderButton, &QPushButton::clicked, this, &SettingsPage::createDlcFolderDirectory);
    connect(selectPkgExtractorButton, &QPushButton::clicked, this, &SettingsPage::selectPkgExtractorPath);
    connect(resetPkgExtractorButton, &QPushButton::clicked, this, &SettingsPage::resetPkgExtractorPath);
    connect(resetButton, &QPushButton::clicked, this, &SettingsPage::resetToDefaults);
    connect(refreshButton, &QPushButton::clicked, this, &SettingsPage::refreshSettings);

    // Connect IGDB credential editing and testing
    connect(igdbClientIdEdit, &QLineEdit::textChanged, this, &SettingsPage::saveIgdbCredentials);
    connect(igdbClientSecretEdit, &QLineEdit::textChanged, this, &SettingsPage::saveIgdbCredentials);
    connect(setDefaultCredsButton, &QPushButton::clicked, this, &SettingsPage::setDefaultIgdbCredentials);
    connect(clearCredsButton, &QPushButton::clicked, this, [this]() {
        CredentialManager::instance().clearAllCredentials();
        refreshSettings();
        QMessageBox::information(this, "Credentials Cleared", 
            "Stored IGDB credentials have been cleared.\nDefault credentials will be used.");
    });
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
    shadps4Path = settings.getShadPS4Path();
    dlcFolderPath = settings.getDlcFolderPath();

    // Load use system shadPS4 checkbox
    useSystemShadPS4Checkbox->setChecked(settings.getUseSystemShadPS4());

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

    // Update ShadPS4 path display
    shadps4PathLabel->setText(shadps4Path.isEmpty() ? "Not configured" : shadps4Path);
    QFileInfo shadps4File(shadps4Path);
    if (shadps4File.exists() && shadps4File.isExecutable()) {
        shadps4StatusLabel->setText("✓ Executable found");
        shadps4StatusLabel->setObjectName("statusValid");
    } else {
        shadps4StatusLabel->setText("✗ Executable not found or not executable");
        shadps4StatusLabel->setObjectName("statusInvalid");
    }

    // Update DLC folder path display
    dlcFolderPathLabel->setText(dlcFolderPath);
    if (settings.isDlcFolderPathValid()) {
        dlcFolderStatusLabel->setText("✓ Directory exists and is writable");
        dlcFolderStatusLabel->setObjectName("statusValid");
    } else {
        dlcFolderStatusLabel->setText("✗ Directory does not exist or is not writable");
        dlcFolderStatusLabel->setObjectName("statusInvalid");
    }

    // Update PKG Extractor path display
    QString pkgExtractorPath = settings.getPkgExtractorPath();
    pkgExtractorPathLabel->setText(pkgExtractorPath.isEmpty() ? "Not configured" : pkgExtractorPath);
    QFileInfo pkgExtractorFile(pkgExtractorPath);
    if (pkgExtractorFile.exists() && pkgExtractorFile.isExecutable()) {
        pkgExtractorStatusLabel->setText("✓ Extractor tool found and executable");
        pkgExtractorStatusLabel->setObjectName("statusValid");
    } else {
        pkgExtractorStatusLabel->setText("✗ Extractor tool not found - PKG extraction will NOT work!");
        pkgExtractorStatusLabel->setObjectName("statusInvalid");
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

void SettingsPage::selectShadPS4Path() {
    QString file = QFileDialog::getOpenFileName(
        this,
        "Select ShadPS4 Executable",
        Settings::instance().getShadPS4Path(),
        "Executables (*);;All Files (*.*)"
    );

    if (!file.isEmpty()) {
        Settings::instance().setShadPS4Path(file);
        refreshSettings();
    }
}

void SettingsPage::selectDlcFolderPath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select DLC Folder Directory",
        Settings::instance().getDlcFolderPath()
    );

    if (!dir.isEmpty()) {
        Settings::instance().setDlcFolderPath(dir);
        refreshSettings();
    }
}

void SettingsPage::selectPkgExtractorPath() {
    QString file = QFileDialog::getOpenFileName(
        this,
        "Select PKG Extractor Executable",
        Settings::instance().getPkgExtractorPath(),
        "Executables (*)"
    );

    if (!file.isEmpty()) {
        Settings::instance().setPkgExtractorPath(file);
        refreshSettings();
        emit settingsChanged();
    }
}

void SettingsPage::resetPkgExtractorPath() {
    Settings::instance().setPkgExtractorPath(""); // Clear custom path
    refreshSettings();
    emit settingsChanged();
    QMessageBox::information(this, "Reset",
        "PKG Extractor path has been reset to default location.");
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

void SettingsPage::createDlcFolderDirectory() {
    if (Settings::instance().createDlcFolderDirectory()) {
        QMessageBox::information(this, "Success",
            "DLC Folder directory created successfully!");
        refreshSettings();
    } else {
        QMessageBox::critical(this, "Error",
            "Failed to create DLC Folder directory!");
    }
}

void SettingsPage::resetToDefaults() {
    Settings& settings = Settings::instance();

    settings.setGameLibraryPath(settings.getDefaultGameLibraryPath());
    settings.setDownloadsPath(settings.getDefaultDownloadsPath());
    settings.setDlcFolderPath(settings.getDefaultDlcFolderPath());
    settings.setShadPS4Path(settings.getDefaultShadPS4Path());

    refreshSettings();

    QMessageBox::information(this, "Reset Complete",
        "Settings have been reset to default paths.");
}

void SettingsPage::saveIgdbCredentials() {
    Settings& settings = Settings::instance();
    settings.setIgdbClientId(igdbClientIdEdit->text());
    settings.setIgdbClientSecret(igdbClientSecretEdit->text());
}

void SettingsPage::setDefaultIgdbCredentials() {
    // Set the provided IGDB credentials
    const QString defaultClientId = "ocrjdozwkkal2p4wx9e8qh6lj6kn90";
    const QString defaultClientSecret = "brj8c9yzc2y92rh22266ikxslpvft9";
    
    igdbClientIdEdit->setText(defaultClientId);
    igdbClientSecretEdit->setText(defaultClientSecret);
    
    // Save the credentials automatically
    saveIgdbCredentials();
    
    QMessageBox::information(this, "Default Credentials Set",
        "Default IGDB credentials have been set and saved securely.\n\n"
        "You can now use IGDB features to download game metadata, cover images, and screenshots.\n\n"
        "Click 'Test Connection' to verify the credentials are working.");
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
