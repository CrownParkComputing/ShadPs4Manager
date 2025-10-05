#ifndef DOWNLOADS_FOLDER_H
#define DOWNLOADS_FOLDER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QMenu>
#include <QAction>

struct DownloadInfo {
    QString fileName;
    QString path;
    qint64 size;
    QString titleId;
    QString contentId;
};

class DownloadsFolder : public QWidget {
    Q_OBJECT

public:
    DownloadsFolder(QWidget* parent = nullptr);

signals:
    void pkgSelected(const QString& pkgPath);
    void extractionRequested(const QString& pkgPath, const QString& outputPath);

public slots:
    void refreshDownloads();
    void setDownloadsPath(const QString& path);

private slots:
    void onPkgDoubleClicked(QListWidgetItem* item);
    void onPkgRightClicked(const QPoint& pos);
    void extractPkg();
    void showPkgInfo();
    void extractAllPkgs();

private:
    void setupUI();
    void applyStyles();
    void loadPkgs();
    void clearPkgs();
    DownloadInfo parsePkgInfo(const QString& pkgPath);

    QListWidget* pkgListWidget;
    QLabel* statusLabel;
    QPushButton* refreshButton;
    QPushButton* extractAllButton;
    QString downloadsPath;
    QList<DownloadInfo> downloads;
};

#endif // DOWNLOADS_FOLDER_H
