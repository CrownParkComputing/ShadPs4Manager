#include "welcome_tab.h"
#include "settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QCryptographicHash>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>

WelcomeTab::WelcomeTab(QWidget* parent) : QWidget(parent) {
    systemId = generateSystemId();
    setupUI();
    applyStyles();
}

void WelcomeTab::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Welcome header
    auto* welcomeLabel = new QLabel("🎮 Welcome to ShadPs4 Manager");
    welcomeLabel->setObjectName("welcomeHeader");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(welcomeLabel);

    // Trial notice
    auto* trialNotice = new QLabel("You are using the trial version (limited to 10 items)");
    trialNotice->setObjectName("trialNotice");
    trialNotice->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(trialNotice);

    mainLayout->addSpacing(20);

    // System ID section
    auto* systemIdGroup = new QGroupBox("Step 1: Your System ID");
    systemIdGroup->setObjectName("systemIdGroup");
    auto* systemIdLayout = new QVBoxLayout(systemIdGroup);
    
    auto* systemIdInfo = new QLabel("Copy this unique System ID to complete your purchase:");
    systemIdLayout->addWidget(systemIdInfo);
    
    auto* systemIdRow = new QHBoxLayout();
    systemIdLabel = new QLabel(systemId);
    systemIdLabel->setObjectName("systemIdDisplay");
    systemIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    systemIdLabel->setWordWrap(true);
    
    copySystemIdButton = new QPushButton("📋 Copy System ID");
    copySystemIdButton->setObjectName("copyButton");
    connect(copySystemIdButton, &QPushButton::clicked, this, &WelcomeTab::onCopySystemId);
    
    systemIdRow->addWidget(systemIdLabel, 1);
    systemIdRow->addWidget(copySystemIdButton);
    systemIdLayout->addLayout(systemIdRow);
    
    mainLayout->addWidget(systemIdGroup);

    // PayPal payment section
    auto* paymentGroup = new QGroupBox("Step 2: Complete Payment via PayPal");
    paymentGroup->setObjectName("paymentGroup");
    auto* paymentLayout = new QVBoxLayout(paymentGroup);
    
    instructionsBrowser = new QTextBrowser();
    instructionsBrowser->setObjectName("paymentBrowser");
    instructionsBrowser->setOpenExternalLinks(true);
    instructionsBrowser->setMaximumHeight(220);
    instructionsBrowser->setHtml(
        "<html><body style='font-family: Arial, sans-serif; color: #ffffff; background-color: #2b2b2b;'>"
        "<h3 style='color: #4a9eff;'>Purchase Full Version</h3>"
        "<p>After copying your System ID above, click the button below to purchase your license via PayPal.</p>"
        "<p style='color: #f0ad4e;'><b>Important:</b> Include your System ID in the PayPal payment notes!</p>"
        "<p>After payment is complete, you will receive an unlock code via email to your PayPal registered address.</p>"
        "</body></html>"
    );
    
    paymentLayout->addWidget(instructionsBrowser);
    
    // Direct PayPal payment button
    auto* paypalButton = new QPushButton("💳 Pay with PayPal");
    paypalButton->setObjectName("paypalButton");
    paypalButton->setMinimumHeight(50);
    connect(paypalButton, &QPushButton::clicked, this, [this]() {
        // Open PayPal hosted button page
        QString paypalUrl = "https://www.paypal.com/ncp/payment/2WY8TPHV3KDMS";
        QDesktopServices::openUrl(QUrl(paypalUrl));
        
        QMessageBox::information(this, "Payment Instructions",
            "You will be redirected to PayPal in your browser.\n\n"
            "IMPORTANT: Please include your System ID in the payment notes:\n" + systemId + "\n\n"
            "After completing payment, you will receive an unlock code via email.");
    });
    paymentLayout->addWidget(paypalButton);
    
    mainLayout->addWidget(paymentGroup);

    // License activation section
    auto* activationGroup = new QGroupBox("Step 3: Activate Your License");
    activationGroup->setObjectName("activationGroup");
    auto* activationLayout = new QVBoxLayout(activationGroup);
    
    auto* activationInfo = new QLabel("Enter the unlock code you received via email:");
    activationLayout->addWidget(activationInfo);
    
    auto* activationRow = new QHBoxLayout();
    licenseKeyInput = new QLineEdit();
    licenseKeyInput->setPlaceholderText("Enter your unlock code here");
    licenseKeyInput->setObjectName("licenseInput");
    
    activateButton = new QPushButton("🔓 Activate License");
    activateButton->setObjectName("activateButton");
    connect(activateButton, &QPushButton::clicked, this, &WelcomeTab::onActivateLicense);
    
    activationRow->addWidget(licenseKeyInput, 1);
    activationRow->addWidget(activateButton);
    activationLayout->addLayout(activationRow);
    
    mainLayout->addWidget(activationGroup);

    mainLayout->addStretch();
}

void WelcomeTab::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #welcomeHeader {
            font-size: 24px;
            font-weight: bold;
            color: #4a9eff;
            padding: 20px;
        }

        #trialNotice {
            font-size: 14px;
            color: #f0ad4e;
            font-weight: bold;
            padding: 10px;
            background-color: #3a3a3a;
            border-radius: 5px;
        }

        QGroupBox {
            background-color: #1e1e1e;
            border: 2px solid #4a9eff;
            border-radius: 8px;
            margin-top: 15px;
            padding-top: 20px;
            font-weight: bold;
            color: #4a9eff;
            font-size: 14px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 5px 15px;
            background-color: #2b2b2b;
            border-radius: 4px;
        }

        #systemIdDisplay {
            background-color: #3a3a3a;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 10px;
            font-family: 'Courier New', monospace;
            font-size: 12px;
            color: #5cb85c;
        }

        #licenseInput {
            background-color: #3a3a3a;
            border: 2px solid #555;
            border-radius: 4px;
            padding: 8px;
            color: #ffffff;
            font-size: 13px;
        }

        #licenseInput:focus {
            border-color: #4a9eff;
        }

        QPushButton {
            background-color: #4a9eff;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 10px 20px;
            font-weight: bold;
            font-size: 13px;
        }

        QPushButton:hover {
            background-color: #5dbaff;
        }

        QPushButton:pressed {
            background-color: #3a8edf;
        }

        #copyButton {
            background-color: #5cb85c;
        }

        #copyButton:hover {
            background-color: #6dc86d;
        }

        #activateButton {
            background-color: #d9534f;
        }

            #activateButton:hover {
                background-color: #e96460;
            }

            #paypalButton {
                background-color: #0070ba;
                font-size: 16px;
                padding: 15px;
            }

            #paypalButton:hover {
                background-color: #003087;
            }

            #paymentBrowser {
                background-color: #2b2b2b;
                border: 1px solid #555;
                border-radius: 4px;
            }
        )");
}QString WelcomeTab::generateSystemId() {
    // Generate unique system identifier based on hardware
    QStringList identifiers;
    
    // Add machine unique ID
    identifiers << QSysInfo::machineUniqueId();
    
    // Add hostname
    identifiers << QSysInfo::machineHostName();
    
    // Add MAC addresses
    for (const QNetworkInterface& interface : QNetworkInterface::allInterfaces()) {
        if (!interface.hardwareAddress().isEmpty() && 
            interface.hardwareAddress() != "00:00:00:00:00:00") {
            identifiers << interface.hardwareAddress();
        }
    }
    
    // Add machine info
    identifiers << QSysInfo::productType();
    identifiers << QSysInfo::productVersion();
    
    // Create hash
    QString combined = identifiers.join("|");
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    
    return hash.toHex().toUpper();
}

void WelcomeTab::onCopySystemId() {
    QApplication::clipboard()->setText(systemId);
    QMessageBox::information(this, "System ID Copied", 
        "Your System ID has been copied to clipboard!\n\n"
        "Please include this ID when making your PayPal payment.");
}

void WelcomeTab::onActivateLicense() {
    QString enteredKey = licenseKeyInput->text().trimmed();
    
    if (enteredKey.isEmpty()) {
        QMessageBox::warning(this, "Empty License Key", 
            "Please enter the unlock code you received via email.");
        return;
    }
    
    // Generate expected license key using system ID + secret
    QString secret = "AstroCity_EGRET_II";
    QString combined = systemId + secret;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    QString expectedKey = hash.toHex().toUpper();
    
    if (enteredKey.toUpper() == expectedKey) {
        // Valid license!
        Settings& settings = Settings::instance();
        settings.setValue("license_key", enteredKey);
        settings.setValue("is_registered", true);
        settings.setValue("system_id", systemId);
        
        QMessageBox::information(this, "License Activated!", 
            "🎉 Thank you for registering ShadPs4 Manager!\n\n"
            "All features are now unlocked.\n"
            "The application will now restart.");
        
        emit licenseActivated();
    } else {
        QMessageBox::critical(this, "Invalid License Key",
            "The unlock code you entered is invalid.\n\n"
            "Please check:\n"
            "• The code was copied correctly from your email\n"
            "• You're using the correct System ID\n"
            "• No extra spaces or characters\n\n"
            "If you continue to have issues, please contact support.");
    }
}
