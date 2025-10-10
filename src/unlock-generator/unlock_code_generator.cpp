#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QMessageBox>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTabWidget>

class UnlockCodeGenerator : public QWidget {
    Q_OBJECT

public:
    UnlockCodeGenerator(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        applyStyles();
        loadRecords();
    }
    
    ~UnlockCodeGenerator() {
        saveRecords();
    }

private slots:
    void generateCode() {
        QString systemId = systemIdInput->text().trimmed();
        QString email = emailInput->text().trimmed();
        
        if (systemId.isEmpty()) {
            QMessageBox::warning(this, "Missing System ID", 
                "Please enter the customer's System ID.");
            return;
        }
        
        if (email.isEmpty()) {
            QMessageBox::warning(this, "Missing Email", 
                "Please enter the customer's email address.");
            return;
        }
        
        // Validate email format
        if (!email.contains("@") || !email.contains(".")) {
            QMessageBox::warning(this, "Invalid Email", 
                "Please enter a valid email address.");
            return;
        }
        
        // Validate System ID format (should be 64 character hex string)
        if (systemId.length() != 64) {
            QMessageBox::warning(this, "Invalid System ID", 
                "System ID should be 64 characters long.\n"
                "Please verify you copied the complete ID.");
            return;
        }
        
        // Generate unlock code using the same algorithm as the main app
        QString secret = "AstroCity_EGRET_II";
        QString combined = systemId.toUpper() + secret;
        QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
        QString unlockCode = hash.toHex().toUpper();
        
        // Display the unlock code
        unlockCodeDisplay->setPlainText(unlockCode);
        
        // Save record
        saveGenerationRecord(systemId, email, unlockCode);
        
        // Add to history
        QString historyEntry = QString("System ID: %1\nEmail: %2\nUnlock Code: %3\n%4\n\n")
            .arg(systemId)
            .arg(email)
            .arg(unlockCode)
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        historyLog->append(historyEntry);
        
        copyButton->setEnabled(true);
    }
    
    void copyCode() {
        QString unlockCode = unlockCodeDisplay->toPlainText();
        if (!unlockCode.isEmpty()) {
            QApplication::clipboard()->setText(unlockCode);
            QMessageBox::information(this, "Code Copied", 
                "The unlock code has been copied to clipboard!\n\n"
                "You can now email this code to the customer.");
        }
    }
    
    void clearForm() {
        systemIdInput->clear();
        emailInput->clear();
        unlockCodeDisplay->clear();
        copyButton->setEnabled(false);
        systemIdInput->setFocus();
    }
    
    void viewRecords() {
        // Show all records in a dialog
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("All Generation Records");
        dialog->setMinimumSize(900, 600);
        
        auto* layout = new QVBoxLayout(dialog);
        
        auto* recordsText = new QTextEdit();
        recordsText->setReadOnly(true);
        recordsText->setFont(QFont("Courier New", 10));
        
        // Load and decrypt records
        QString recordsContent;
        for (const auto& record : generationRecords) {
            recordsContent += QString("=== Record ===\n");
            recordsContent += QString("Date: %1\n").arg(record.timestamp);
            recordsContent += QString("System ID: %1\n").arg(record.systemId);
            recordsContent += QString("Email: %1\n").arg(decryptString(record.encryptedEmail));
            recordsContent += QString("Unlock Code: %1\n\n").arg(record.unlockCode);
        }
        
        if (recordsContent.isEmpty()) {
            recordsContent = "No records found.";
        }
        
        recordsText->setPlainText(recordsContent);
        layout->addWidget(recordsText);
        
        auto* closeButton = new QPushButton("Close");
        connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
        layout->addWidget(closeButton);
        
        dialog->exec();
    }

private:
    struct GenerationRecord {
        QString timestamp;
        QString systemId;
        QString encryptedEmail;  // Base64 encoded
        QString unlockCode;
    };
    
    QString encryptString(const QString& plainText) {
        // Simple XOR encryption with Base64 encoding
        QByteArray key = "AstroCity_EGRET_II_AdminKey";
        QByteArray data = plainText.toUtf8();
        QByteArray encrypted;
        
        for (int i = 0; i < data.size(); ++i) {
            encrypted.append(data[i] ^ key[i % key.size()]);
        }
        
        return encrypted.toBase64();
    }
    
    QString decryptString(const QString& encryptedBase64) {
        // Decrypt Base64 encoded XOR encrypted string
        QByteArray key = "AstroCity_EGRET_II_AdminKey";
        QByteArray encrypted = QByteArray::fromBase64(encryptedBase64.toUtf8());
        QByteArray decrypted;
        
        for (int i = 0; i < encrypted.size(); ++i) {
            decrypted.append(encrypted[i] ^ key[i % key.size()]);
        }
        
        return QString::fromUtf8(decrypted);
    }
    
    QString getRecordsFilePath() {
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(dataPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        return dir.filePath("unlock_code_records.json");
    }
    
    void saveGenerationRecord(const QString& systemId, const QString& email, const QString& unlockCode) {
        GenerationRecord record;
        record.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        record.systemId = systemId;
        record.encryptedEmail = encryptString(email);
        record.unlockCode = unlockCode;
        
        generationRecords.append(record);
        saveRecords();
    }
    
    void saveRecords() {
        QJsonArray recordsArray;
        
        for (const auto& record : generationRecords) {
            QJsonObject obj;
            obj["timestamp"] = record.timestamp;
            obj["systemId"] = record.systemId;
            obj["encryptedEmail"] = record.encryptedEmail;
            obj["unlockCode"] = record.unlockCode;
            recordsArray.append(obj);
        }
        
        QJsonDocument doc(recordsArray);
        
        QFile file(getRecordsFilePath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }
    
    void loadRecords() {
        QFile file(getRecordsFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            return; // No records file yet
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) {
            return;
        }
        
        QJsonArray recordsArray = doc.array();
        for (const QJsonValue& value : recordsArray) {
            if (!value.isObject()) continue;
            
            QJsonObject obj = value.toObject();
            GenerationRecord record;
            record.timestamp = obj["timestamp"].toString();
            record.systemId = obj["systemId"].toString();
            record.encryptedEmail = obj["encryptedEmail"].toString();
            record.unlockCode = obj["unlockCode"].toString();
            
            generationRecords.append(record);
        }
        
        // Update history log with loaded records
        QString historyText;
        for (const auto& record : generationRecords) {
            historyText += QString("System ID: %1\nEmail: %2\nUnlock Code: %3\n%4\n\n")
                .arg(record.systemId)
                .arg(decryptString(record.encryptedEmail))
                .arg(record.unlockCode)
                .arg(record.timestamp);
        }
        if (!historyText.isEmpty()) {
            historyLog->setPlainText(historyText);
        }
    }
    
    void setupUI() {
        setWindowTitle("ShadPs4 Manager - Unlock Code Generator (Admin)");
        setMinimumSize(700, 600);
        
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(20);
        mainLayout->setContentsMargins(30, 30, 30, 30);
        
        // Title
        auto* titleLabel = new QLabel("🔐 Unlock Code Generator");
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        auto* subtitleLabel = new QLabel("Administrator Tool for ShadPs4 Manager License Generation");
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(subtitleLabel);
        
        mainLayout->addSpacing(10);
        
        // Tab widget
        QTabWidget* tabWidget = new QTabWidget();
        
        // === GENERATE TAB ===
        QWidget* generateTab = new QWidget();
        auto* generateLayout = new QVBoxLayout(generateTab);
        generateLayout->setSpacing(15);
        
        // Input section
        auto* inputGroup = new QGroupBox("Generate Unlock Code");
        inputGroup->setObjectName("inputGroup");
        auto* inputLayout = new QVBoxLayout(inputGroup);
        
        auto* instructionLabel = new QLabel(
            "1. Customer sends their System ID after PayPal payment\n"
            "2. Paste the System ID and customer's email below\n"
            "3. Click Generate to create unlock code\n"
            "4. Copy the code and email it to the customer"
        );
        instructionLabel->setWordWrap(true);
        inputLayout->addWidget(instructionLabel);
        
        inputLayout->addSpacing(10);
        
        auto* systemIdLabel = new QLabel("Customer's System ID:");
        systemIdLabel->setObjectName("fieldLabel");
        inputLayout->addWidget(systemIdLabel);
        
        systemIdInput = new QLineEdit();
        systemIdInput->setPlaceholderText("Paste 64-character System ID here...");
        systemIdInput->setObjectName("systemIdInput");
        inputLayout->addWidget(systemIdInput);
        
        inputLayout->addSpacing(10);
        
        auto* emailLabel = new QLabel("Customer's Email Address:");
        emailLabel->setObjectName("fieldLabel");
        inputLayout->addWidget(emailLabel);
        
        emailInput = new QLineEdit();
        emailInput->setPlaceholderText("customer@example.com");
        emailInput->setObjectName("emailInput");
        inputLayout->addWidget(emailInput);
        
        auto* buttonRow = new QHBoxLayout();
        auto* generateButton = new QPushButton("🔑 Generate Unlock Code");
        generateButton->setObjectName("generateButton");
        connect(generateButton, &QPushButton::clicked, this, &UnlockCodeGenerator::generateCode);
        
        auto* clearButton = new QPushButton("🗑️ Clear");
        clearButton->setObjectName("clearButton");
        connect(clearButton, &QPushButton::clicked, this, &UnlockCodeGenerator::clearForm);
        
        buttonRow->addWidget(generateButton);
        buttonRow->addWidget(clearButton);
        inputLayout->addLayout(buttonRow);
        
        generateLayout->addWidget(inputGroup);
        
        // Output section
        auto* outputGroup = new QGroupBox("Generated Unlock Code");
        outputGroup->setObjectName("outputGroup");
        auto* outputLayout = new QVBoxLayout(outputGroup);
        
        auto* outputLabel = new QLabel("Send this code to the customer:");
        outputLabel->setObjectName("fieldLabel");
        outputLayout->addWidget(outputLabel);
        
        unlockCodeDisplay = new QTextEdit();
        unlockCodeDisplay->setReadOnly(true);
        unlockCodeDisplay->setMaximumHeight(60);
        unlockCodeDisplay->setObjectName("unlockCodeDisplay");
        unlockCodeDisplay->setPlaceholderText("Unlock code will appear here...");
        outputLayout->addWidget(unlockCodeDisplay);
        
        copyButton = new QPushButton("📋 Copy Unlock Code");
        copyButton->setObjectName("copyButton");
        copyButton->setEnabled(false);
        connect(copyButton, &QPushButton::clicked, this, &UnlockCodeGenerator::copyCode);
        outputLayout->addWidget(copyButton);
        
        generateLayout->addWidget(outputGroup);
        generateLayout->addStretch();
        
        tabWidget->addTab(generateTab, "Generate Code");
        
        // === HISTORY TAB ===
        QWidget* historyTab = new QWidget();
        auto* historyLayout = new QVBoxLayout(historyTab);
        
        auto* historyGroup = new QGroupBox("Generation History (Current Session)");
        historyGroup->setObjectName("historyGroup");
        auto* historyGroupLayout = new QVBoxLayout(historyGroup);
        
        historyLog = new QTextEdit();
        historyLog->setReadOnly(true);
        historyLog->setObjectName("historyLog");
        historyLog->setPlaceholderText("Generation history will appear here...");
        historyGroupLayout->addWidget(historyLog);
        
        historyLayout->addWidget(historyGroup);
        
        auto* viewAllButton = new QPushButton("📋 View All Records (All Time)");
        viewAllButton->setObjectName("viewRecordsButton");
        connect(viewAllButton, &QPushButton::clicked, this, &UnlockCodeGenerator::viewRecords);
        historyLayout->addWidget(viewAllButton);
        
        tabWidget->addTab(historyTab, "History");
        
        mainLayout->addWidget(tabWidget);
    }
    
    void applyStyles() {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 13px;
            }
            
            #titleLabel {
                font-size: 26px;
                font-weight: bold;
                color: #4a9eff;
                padding: 10px;
            }
            
            #subtitleLabel {
                font-size: 14px;
                color: #aaaaaa;
                padding: 5px;
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
            
            #fieldLabel {
                font-weight: bold;
                color: #cccccc;
            }
            
            QLineEdit, QTextEdit {
                background-color: #3a3a3a;
                border: 2px solid #555;
                border-radius: 4px;
                padding: 8px;
                color: #ffffff;
                font-size: 13px;
            }
            
            QLineEdit:focus, QTextEdit:focus {
                border-color: #4a9eff;
            }
            
            #systemIdInput {
                font-family: 'Courier New', monospace;
            }
            
            #emailInput {
                font-family: Arial, sans-serif;
            }
            
            #unlockCodeDisplay {
                font-family: 'Courier New', monospace;
                color: #5cb85c;
                font-weight: bold;
            }
            
            #historyLog {
                font-family: 'Courier New', monospace;
                font-size: 11px;
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
            
            QPushButton:disabled {
                background-color: #555555;
                color: #888888;
            }
            
            #generateButton {
                background-color: #5cb85c;
            }
            
            #generateButton:hover {
                background-color: #6dc86d;
            }
            
            #copyButton {
                background-color: #f0ad4e;
            }
            
            #copyButton:hover {
                background-color: #f5bc6c;
            }
            
            #clearButton {
                background-color: #d9534f;
            }
            
            #clearButton:hover {
                background-color: #e96460;
            }
            
            #viewRecordsButton {
                background-color: #f0ad4e;
            }
            
            #viewRecordsButton:hover {
                background-color: #f5bc6c;
            }
        )");
    }
    
    QLineEdit* systemIdInput;
    QLineEdit* emailInput;
    QTextEdit* unlockCodeDisplay;
    QTextEdit* historyLog;
    QPushButton* copyButton;
    QList<GenerationRecord> generationRecords;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ShadPs4 Manager Unlock Code Generator");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ShadPs4");
    
    UnlockCodeGenerator generator;
    generator.show();
    
    return app.exec();
}

#include "unlock_code_generator.moc"
