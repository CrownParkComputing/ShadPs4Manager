#include "downloads_folder.h"
#include "settings.h"
#include "param_sfo.h"

DownloadsFolder::DownloadsFolder(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyStyles();
    refreshDownloads();
}

void DownloadsFolder::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* headerLabel = new QLabel("Downloads Folder");
    headerLabel->setObjectName("downloadsHeader");
    refreshButton = new QPushButton("Refresh");
    extractAllButton = new QPushButton("Extract All");
    headerLayout->addWidget(headerLabel, 1);
    headerLayout->addWidget(refreshButton);
    headerLayout->addWidget(extractAllButton);
    mainLayout->addLayout(headerLayout);

    // PKG list
    pkgListWidget = new QListWidget();
    pkgListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(pkgListWidget);

    // Status label
    statusLabel = new QLabel("Ready");
    mainLayout->addWidget(statusLabel);

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &DownloadsFolder::refreshDownloads);
    connect(extractAllButton, &QPushButton::clicked, this, &DownloadsFolder::extractAllPkgs);
    connect(pkgListWidget, &QListWidget::itemDoubleClicked, this, &DownloadsFolder::onPkgDoubleClicked);
    connect(pkgListWidget, &QListWidget::customContextMenuRequested, this, &DownloadsFolder::onPkgRightClicked);
}

void DownloadsFolder::applyStyles() {
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        #downloadsHeader {
            font-size: 16px;
            font-weight: bold;
            color: #2196F3;
            padding: 5px;
        }

        QListWidget {
            background-color: #1e1e1e;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
            outline: none;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #444444;
            background-color: #353535;
            margin: 2px;
            border-radius: 3px;
        }

        QListWidget::item:selected {
            background-color: #2196F3;
            color: #ffffff;
        }

        QListWidget::item:hover {
            background-color: #404040;
        }

        QPushButton {
            background-color: #404040;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
            min-width: 80px;
        }

        QPushButton:hover {
            background-color: #505050;
            border-color: #777777;
        }

        QLabel {
            color: #cccccc;
            font-size: 12px;
            padding: 5px;
        }
    )");
}

void DownloadsFolder::refreshDownloads() {
    Settings& settings = Settings::instance();
    setDownloadsPath(settings.getDownloadsPath());
}

void DownloadsFolder::setDownloadsPath(const QString& path) {
    downloadsPath = path;
    loadPkgs();
}

void DownloadsFolder::loadPkgs() {
    clearPkgs();

    if (downloadsPath.isEmpty()) {
        statusLabel->setText("No downloads path configured");
        return;
    }

    QDir downloadsDir(downloadsPath);
    if (!downloadsDir.exists()) {
        statusLabel->setText("Downloads directory does not exist");
        return;
    }

    // Look for PKG files
    QStringList pkgFiles = downloadsDir.entryList(QStringList{"*.pkg"}, QDir::Files);

    if (pkgFiles.isEmpty()) {
        statusLabel->setText("No PKG files found in downloads");
        return;
    }

    for (const QString& pkgFile : pkgFiles) {
        QString pkgPath = downloadsDir.absoluteFilePath(pkgFile);
        DownloadInfo pkgInfo = parsePkgInfo(pkgPath);

        downloads.append(pkgInfo);

        QListWidgetItem* item = new QListWidgetItem();
        QString displayText = QString("%1 - %2 MB")
                            .arg(pkgInfo.fileName)
                            .arg(pkgInfo.size / (1024.0 * 1024.0), 0, 'f', 1);

        item->setText(displayText);
        item->setData(Qt::UserRole, pkgPath);
        pkgListWidget->addItem(item);
    }

    statusLabel->setText(QString("Found %1 PKG files").arg(downloads.size()));
}

void DownloadsFolder::clearPkgs() {
    pkgListWidget->clear();
    downloads.clear();
}

DownloadInfo DownloadsFolder::parsePkgInfo(const QString& pkgPath) {
    DownloadInfo info;
    QFileInfo fileInfo(pkgPath);

    info.fileName = fileInfo.fileName();
    info.path = pkgPath;
    info.size = fileInfo.size();

    // For now, set placeholder values
    // In a real implementation, you'd read the PKG metadata
    info.titleId = "CUSA00000";
    info.contentId = "UNKNOWN";

    return info;
}

void DownloadsFolder::onPkgDoubleClicked(QListWidgetItem* item) {
    QString pkgPath = item->data(Qt::UserRole).toString();
    emit pkgSelected(pkgPath);
}

void DownloadsFolder::onPkgRightClicked(const QPoint& pos) {
    QListWidgetItem* item = pkgListWidget->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);
    QAction* extractAction = contextMenu.addAction("Extract PKG");
    QAction* infoAction = contextMenu.addAction("PKG Info");

    QAction* selectedAction = contextMenu.exec(pkgListWidget->mapToGlobal(pos));
    if (!selectedAction) return;

    if (selectedAction == extractAction) {
        extractPkg();
    } else if (selectedAction == infoAction) {
        showPkgInfo();
    }
}

void DownloadsFolder::extractPkg() {
    QListWidgetItem* currentItem = pkgListWidget->currentItem();
    if (!currentItem) return;

    QString pkgPath = currentItem->data(Qt::UserRole).toString();
    QString gameLibraryPath = Settings::instance().getGameLibraryPath();

    if (gameLibraryPath.isEmpty()) {
        QMessageBox::warning(this, "No Library Path",
            "Please configure the Game Library path in Settings first.");
        return;
    }

    // Generate output directory name from PKG filename
    QFileInfo pkgInfo(pkgPath);
    QString outputDir = gameLibraryPath + "/" + pkgInfo.baseName() + "_extracted";

    emit extractionRequested(pkgPath, outputDir);
}

void DownloadsFolder::showPkgInfo() {
    QListWidgetItem* currentItem = pkgListWidget->currentItem();
    if (!currentItem) return;

    QString pkgPath = currentItem->data(Qt::UserRole).toString();

    QMessageBox::information(this, "PKG Information",
        QString("PKG Path: %1\n\nMore detailed PKG information would be displayed here.").arg(pkgPath));
}

void DownloadsFolder::extractAllPkgs() {
    if (downloads.isEmpty()) {
        QMessageBox::information(this, "No PKGs", "No PKG files found to extract.");
        return;
    }

    QString gameLibraryPath = Settings::instance().getGameLibraryPath();
    if (gameLibraryPath.isEmpty()) {
        QMessageBox::warning(this, "No Library Path",
            "Please configure the Game Library path in Settings first.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Extract All PKGs",
        QString("Extract all %1 PKG files to Game Library?").arg(downloads.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (const DownloadInfo& download : downloads) {
            QFileInfo pkgInfo(download.path);
            QString outputDir = gameLibraryPath + "/" + pkgInfo.baseName() + "_extracted";
            emit extractionRequested(download.path, outputDir);
        }
    }
}
