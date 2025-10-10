#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QProcess process;
    process.setWorkingDirectory("/home/jon/ShadPs4Manager/build/bin");
    // Don't set any channel mode - use Qt default (MergedChannels)
    
    QStringList args;
    args << "/home/jon/Downloads/Capcom Arcade Stadium - Complete Bundle [CUSA26271] FIXED 9.00+/CUSA26271_CAPCOM_ARCADE_STADIUM_UPDATEv1.05_FXD_[FW900].pkg";
    args << "/tmp/test-qprocess-update";
    
    qDebug() << "Starting process...";
    process.start("/home/jon/ShadPs4Manager/build/bin/shadps4-pkg-extractor", args);
    
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start:" << process.errorString();
        return 1;
    }
    
    qDebug() << "Process started, waiting for finish...";
    process.waitForFinished(-1);
    
    qDebug() << "Exit code:" << process.exitCode();
    qDebug() << "Exit status:" << process.exitStatus();
    
    return 0;
}
