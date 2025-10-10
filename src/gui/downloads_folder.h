#ifndef DOWNLOADS_FOLDER_H
#define DOWNLOADS_FOLDER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QProgressBar>
#include <QInputDialog>
#include <QElapsedTimer>

enum class PkgType {
    BaseGame,
    Update,
    DLC,
    Unknown
};

struct DownloadInfo {
    QString fileName;
    QString path;
    qint64 size;
    QString titleId;
    QString contentId;
    PkgType pkgType;
    QString gameName;
    QString version;
    int installOrder; // 0 = base game, 1+ = updates/DLC in order
};

struct GameGroup {
    QString gameName;
    QString titleId;
    QList<DownloadInfo> packages;
    bool hasBaseGame = false;
    int updateCount = 0;
    int dlcCount = 0;
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
    void onGameDoubleClicked(QTreeWidgetItem* item, int column);
    void onGameRightClicked(const QPoint& pos);
    void extractGame();
    void extractArchive();
    void extractSelectedPkgs();
    void showGameInfo();
    void installGameInOrder();
    void installSinglePackage(const DownloadInfo& pkg);
    void installGameGroup(const GameGroup& group);
    void extractArchiveFile(const QString& archivePath);
    void deletePackage(const DownloadInfo& pkg, QTreeWidgetItem* item);

private:
    void setupUI();
    void applyStyles();
    void loadPkgs();
    void clearPkgs();
    void groupGamesByTitle();
    void updateGameTree();
    DownloadInfo parsePkgInfo(const QString& pkgPath);
    PkgType detectPkgType(const QString& contentId, const QString& titleId);
    QString extractGameName(const QString& pkgPath);
    QString formatFileSize(qint64 size);
    bool checkBatchDiskSpace(const QList<DownloadInfo>& packages, const QString& outputBasePath, QString& errorMessage);
    QString getProperDirectoryName(const QString& pkgPath);
    QString getInstallPath(const DownloadInfo& pkg);
    QString formatBytes(uint64_t bytes);

    // Debounce & progress tracking
    qint64 lastScanMS = 0;          // Epoch ms of last completed scan
    bool scanInProgress = false;    // Guard against re-entrancy
    int lastFileCount = 0;          // For quick status comparisons
    QElapsedTimer scanTimer;        // Measure scan duration

    QTreeWidget* gameTreeWidget;
    QLabel* statusLabel;
    QPushButton* refreshButton;
    QPushButton* installSelectedButton;
    QPushButton* installAllButton;
    QString downloadsPath;
    QList<DownloadInfo> downloads;
    QList<GameGroup> gameGroups;
};

#endif // DOWNLOADS_FOLDER_H
