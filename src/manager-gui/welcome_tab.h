#ifndef WELCOME_TAB_H
#define WELCOME_TAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QGroupBox>

class WelcomeTab : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeTab(QWidget* parent = nullptr);

signals:
    void licenseActivated();

private slots:
    void onActivateLicense();
    void onCopySystemId();

private:
    void setupUI();
    void applyStyles();
    QString generateSystemId();
    
    QLabel* systemIdLabel;
    QTextBrowser* instructionsBrowser;
    QLineEdit* licenseKeyInput;
    QPushButton* activateButton;
    QPushButton* copySystemIdButton;
    QString systemId;
};

#endif // WELCOME_TAB_H
