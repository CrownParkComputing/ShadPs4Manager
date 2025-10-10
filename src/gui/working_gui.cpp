// ShadPs4 Manager - GUI Application
// 
// ARCHITECTURE:
// This GUI application acts as a frontend for PS4 PKG management and extraction.
// It uses a separate CLI tool (shadps4-pkg-extractor) for all PKG extraction operations.
// 
// DEPENDENCIES:
// - shadps4-pkg-extractor: CLI binary that performs actual PKG extraction
//   Must be present in the same bin/ folder as this GUI executable
// 
// EXTRACTION FLOW:
// 1. GUI queues extraction requests from user actions
// 2. GUI spawns shadps4-pkg-extractor process via QProcess
// 3. CLI tool performs extraction and outputs progress to stdout/stderr
// 4. GUI captures output streams and displays in real-time log
// 5. On completion, GUI processes next item in queue
// 
// This separation provides:
// - Process isolation (CLI crashes don't crash GUI)
// - Easier debugging (can test CLI independently)
// - Simpler threading model (each extraction is independent process)
// - Better error recovery (failed extractions don't corrupt GUI state)

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QRegularExpression>
#include <QPointer>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QProcess>
#include <QDialog>
#include <QTextEdit>
#include <QDebug>
#include <QQueue>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QThread>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QAudioDevice>
#include <chrono>

#include "settings.h"
#include "settings_page.h"
#include "game_library.h"
#include "downloads_folder.h"
#include "installation_folder.h"

// Custom animated title widget with dancing letters
class AnimatedTitleWidget : public QWidget {
    Q_OBJECT
private:
    QString titleText = "ShadPs4 Manager";
    int animationFrame = 0;
    bool isAnimating = false;
    QTimer* animationTimer = nullptr;
    
public:
    AnimatedTitleWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(80);
        setMaximumHeight(80);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // Expand horizontally, fixed height
        
        animationTimer = new QTimer(this);
        connect(animationTimer, &QTimer::timeout, this, [this]() {
            animationFrame++;
            update(); // Trigger paintEvent
        });
        animationTimer->start(30); // 30ms = ~33 FPS for smooth animation
    }
    
    void setAnimating(bool animate) {
        isAnimating = animate;
        update();
    }
    
protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        
        QFont font = painter.font();
        font.setPointSize(24);
        font.setBold(true);
        painter.setFont(font);
        
        QFontMetrics fm(font);
        int totalWidth = fm.horizontalAdvance(titleText);
        int startX = (width() - totalWidth) / 2;
        int baseY = height() / 2 + fm.height() / 4;
        
        int currentX = startX;
        
        for (int i = 0; i < titleText.length(); i++) {
            QChar letter = titleText[i];
            QString letterStr = QString(letter);
            int letterWidth = fm.horizontalAdvance(letterStr);
            
            painter.save();
            
            if (isAnimating) {
                // Calculate animation offsets for each letter
                float time = animationFrame * 0.05f;
                float letterPhase = i * 0.3f; // Phase offset for wave effect
                
                // Vertical dancing motion (sine wave)
                float verticalOffset = sin(time + letterPhase) * 15.0f;
                
                // Horizontal wave motion
                float horizontalOffset = cos(time * 0.7f + letterPhase) * 5.0f;
                
                // Rotation
                float rotation = sin(time * 1.2f + letterPhase * 0.5f) * 10.0f;
                
                // Scale pulsing
                float scale = 1.0f + sin(time * 1.5f + letterPhase) * 0.15f;
                
                // Color cycling through rainbow
                int hue = (int)(time * 50 + i * 15) % 360;
                QColor color = QColor::fromHsv(hue, 255, 255);
                
                // Apply transformations
                int centerX = currentX + letterWidth / 2;
                int centerY = baseY;
                
                painter.translate(centerX + horizontalOffset, centerY + verticalOffset);
                painter.rotate(rotation);
                painter.scale(scale, scale);
                painter.translate(-letterWidth / 2, 0);
                
                painter.setPen(color);
                
                // Add glow effect
                QPen glowPen(color);
                glowPen.setWidth(2);
                painter.setPen(glowPen);
            } else {
                // Static display
                painter.translate(currentX, baseY);
                painter.setPen(QColor(106, 90, 205)); // SlateBlue
            }
            
            painter.drawText(0, 0, letterStr);
            painter.restore();
            
            currentX += letterWidth;
        }
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

signals:
    void extractionFailed(const QString& pkgPath, const QString& errorMessage, PkgType pkgType);

private:
    struct ExtractionRequest {
        QString pkgPath;
        QString outputPath;
        PkgType pkgType;
    };
    void setupUI();
    void connectSignals();
    void onSettingsChanged();
    
    QQueue<ExtractionRequest> extractionQueue;
    bool isExtracting = false;
    QProcess* currentExtractionProcess = nullptr;
    QTextEdit* extractionLogWidget = nullptr;  // Log output area
    QWidget* extractionTabWidget = nullptr;  // Extraction tab container
    QTabWidget* mainTabWidget = nullptr;  // Reference to switch tabs
    
    // Music player
    QMediaPlayer* musicPlayer = nullptr;
    QAudioOutput* audioOutput = nullptr;
    QProcess* shadps4Process = nullptr;  // Track running emulator process
    QStringList musicPlaylist;
    int currentTrackIndex = 0;
    AnimatedTitleWidget* titleWidget = nullptr;
    QLabel* trackLabel = nullptr;

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        connectSignals();
        
        // Load settings on startup
        onSettingsChanged();
        
        // Initialize music player
        setupMusicPlayer();
    }

    ~MainWindow() {
        // Clean up
        if (musicPlayer) {
            musicPlayer->stop();
            delete musicPlayer;
            musicPlayer = nullptr;
        }
        if (audioOutput) {
            delete audioOutput;
            audioOutput = nullptr;
        }
    }
    
    void setupMusicPlayer() {
        musicPlayer = new QMediaPlayer(this);
        audioOutput = new QAudioOutput(this);
        
        // Set HDMI output as default
        const QAudioDevice hdmiDevice = findHdmiAudioDevice();
        if (!hdmiDevice.isNull()) {
            audioOutput->setDevice(hdmiDevice);
        }
        
        musicPlayer->setAudioOutput(audioOutput);
        
        // Load playlist from jukebox folder
        QDir jukeboxDir("/home/jon/ShadPs4Manager/src/jukebox");
        QStringList filters;
        filters << "*.mp3";
        musicPlaylist = jukeboxDir.entryList(filters, QDir::Files, QDir::Name);
        
        if (!musicPlaylist.isEmpty()) {
            // Convert to full paths
            for (QString& track : musicPlaylist) {
                track = jukeboxDir.absoluteFilePath(track);
            }
            
            currentTrackIndex = 0;
            audioOutput->setVolume(0.5);
            
            // Connect track finished signal to play next
            connect(musicPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    playNextTrack();
                }
            });
            
            // Connect playback state changes to control title animation
            connect(musicPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
                if (titleWidget) {
                    titleWidget->setAnimating(state == QMediaPlayer::PlayingState);
                }
            });
            
            playCurrentTrack();
        }
    }
    
    void playCurrentTrack() {
        if (!musicPlaylist.isEmpty() && currentTrackIndex >= 0 && currentTrackIndex < musicPlaylist.size()) {
            musicPlayer->setSource(QUrl::fromLocalFile(musicPlaylist[currentTrackIndex]));
            musicPlayer->play();
            
            // Update track label
            if (trackLabel) {
                QFileInfo fileInfo(musicPlaylist[currentTrackIndex]);
                QString trackName = fileInfo.completeBaseName();
                trackLabel->setText("♫ " + trackName);
            }
        }
    }
    
    void playNextTrack() {
        if (!musicPlaylist.isEmpty()) {
            currentTrackIndex = (currentTrackIndex + 1) % musicPlaylist.size();
            playCurrentTrack();
        }
    }
    
    void playPreviousTrack() {
        if (!musicPlaylist.isEmpty()) {
            currentTrackIndex--;
            if (currentTrackIndex < 0) {
                currentTrackIndex = musicPlaylist.size() - 1;
            }
            playCurrentTrack();
        }
    }
    
    void togglePlayPause() {
        if (musicPlayer) {
            if (musicPlayer->playbackState() == QMediaPlayer::PlayingState) {
                musicPlayer->pause();
            } else {
                musicPlayer->play();
            }
        }
    }
    
    void stopMusic() {
        if (musicPlayer) {
            musicPlayer->stop();
        }
    }
    
    QAudioDevice findHdmiAudioDevice() {
        QMediaDevices mediaDevices;
        
        qDebug() << "=== Audio Device Selection ===";
        qDebug() << "Available audio output devices:";
        const QList<QAudioDevice> audioDevices = mediaDevices.audioOutputs();
        for (const QAudioDevice &device : audioDevices) {
            qDebug() << "  -" << device.description();
        }
        
        // Check if default device is already HDMI/digital
        QAudioDevice defaultDevice = mediaDevices.defaultAudioOutput();
        QString defaultName = defaultDevice.description().toLower();
        qDebug() << "System default device:" << defaultDevice.description();
        
        if (defaultName.contains("hdmi") || defaultName.contains("digital")) {
            qDebug() << "✓ Default device is already HDMI/digital, using it!";
            return defaultDevice;
        }
        
        // Default isn't HDMI, search for HDMI device explicitly
        qDebug() << "Default device is not HDMI, searching for HDMI device...";
        for (const QAudioDevice &device : audioDevices) {
            QString deviceName = device.description().toLower();
            if (deviceName.contains("hdmi") || deviceName.contains("digital")) {
                qDebug() << "✓ Found HDMI audio device:" << device.description();
                return device;
            }
        }
        
        // If no HDMI found, return default device
        qDebug() << "⚠ No HDMI device found, using system default:" << defaultDevice.description();
        return defaultDevice;
    }

public slots:
    void refreshAllData() {
        gameLibrary->refreshLibrary();
    }

    void launchEmulator() {
        QString shadps4Path = Settings::instance().getShadPS4Path();
        
        if (shadps4Path.isEmpty()) {
            QMessageBox::warning(this, "No Emulator Path", 
                "Please set the ShadPS4 emulator path in Settings before launching.");
            return;
        }
        
        // Stop music when launching game
        if (musicPlayer) {
            musicPlayer->pause();
        }
        
        // Launch ShadPS4 and track the process
        shadps4Process = new QProcess(this);
        connect(shadps4Process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MainWindow::onEmulatorFinished);
        shadps4Process->start(shadps4Path);
    }
    
    void onEmulatorFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        // Resume music when emulator closes
        if (musicPlayer) {
            musicPlayer->play();
        }
        
        if (shadps4Process) {
            shadps4Process->deleteLater();
            shadps4Process = nullptr;
        }
    }

    void killShadPS4() {
        // Kill all ShadPS4 processes (force terminate)
        QProcess killProcess;
        killProcess.start("pkill", QStringList() << "-9" << "-f" << "shadps4");
        killProcess.waitForFinished(3000); // Wait up to 3 seconds
        
        // Resume music after killing
        if (musicPlayer) {
            musicPlayer->play();
        }
        
        if (killProcess.exitCode() == 0 || killProcess.exitCode() == 1) {
            // Exit code 0 means processes were killed
            // Exit code 1 means no matching processes found (also ok)
            QMessageBox::information(this, "Kill ShadPS4", 
                "All ShadPS4 processes have been terminated.");
        } else {
            QMessageBox::warning(this, "Kill Failed", 
                QString("Failed to kill ShadPS4 processes.\nYou may need to terminate them manually.\nExit code: %1").arg(killProcess.exitCode()));
        }
    }

    void extractPkgFile(const QString& pkgPath, const QString& outputPath) {
        // Determine PKG type from filename
        QFileInfo fileInfo(pkgPath);
        QString baseName = fileInfo.completeBaseName();
        PkgType pkgType = detectPkgType(baseName);
        
        // Add to extraction queue
        // Deduplicate: skip if same path already queued or currently extracting
        bool duplicate = false;
        if (isExtracting && currentExtractionProcess) {
            // currentExtractionProcess arguments handled separately
        }
        for (const auto &req : extractionQueue) {
            if (req.pkgPath == pkgPath) { duplicate = true; break; }
        }
        if (duplicate) {
            qDebug() << "Duplicate PKG already in queue, skipping:" << pkgPath;
            return;
        }
        ExtractionRequest request{pkgPath, outputPath, pkgType};
        extractionQueue.enqueue(request);
        
        qDebug() << "Added to queue:" << pkgPath << "Type:" << static_cast<int>(pkgType);
        
        // Switch to extraction log tab when starting extraction
        if (mainTabWidget && extractionTabWidget) {
            int logTabIndex = mainTabWidget->indexOf(extractionTabWidget);
            if (logTabIndex >= 0) {
                mainTabWidget->setCurrentIndex(logTabIndex);
            }
        }
        
        // Process queue if not already extracting
        processExtractionQueue();
    }

private slots:
    void processExtractionQueue() {
        // If already extracting or queue is empty, return
        if (isExtracting || extractionQueue.isEmpty()) {
            return;
        }
        
        // Mark as extracting and process next item
        isExtracting = true;
        ExtractionRequest request = extractionQueue.dequeue();
        
        qDebug() << "=== PKG EXTRACTION STARTED (CLI-BASED) ===";
        qDebug() << "PKG Path:" << request.pkgPath;
        qDebug() << "Output Path:" << request.outputPath;
        qDebug() << "PKG Type:" << static_cast<int>(request.pkgType);
        qDebug() << "Remaining in queue:" << extractionQueue.size();
        
        // Log to the extraction log widget
        if (extractionLogWidget) {
            QString pkgTypeName;
            switch (request.pkgType) {
                case PkgType::BaseGame: pkgTypeName = "Base Game"; break;
                case PkgType::Update: pkgTypeName = "Update"; break;
                case PkgType::DLC: pkgTypeName = "DLC"; break;
                default: pkgTypeName = "Package"; break;
            }
            
            extractionLogWidget->append(QString("\n=== EXTRACTION STARTED ==="));
            extractionLogWidget->append(QString("Type: %1").arg(pkgTypeName));
            extractionLogWidget->append(QString("File: %1").arg(QFileInfo(request.pkgPath).fileName()));
            extractionLogWidget->append(QString("Output: %1").arg(request.outputPath));
            extractionLogWidget->append(QString("Queue: %1 remaining\n").arg(extractionQueue.size()));
            
            // Auto-scroll to bottom
            QTextCursor cursor = extractionLogWidget->textCursor();
            cursor.movePosition(QTextCursor::End);
            extractionLogWidget->setTextCursor(cursor);
        }
        
        // Get the path to our CLI pkg extractor executable
        QString extractorPath = Settings::instance().getPkgExtractorPath();
        
        if (!QFile::exists(extractorPath)) {
            qDebug() << "CRITICAL ERROR: CLI pkg extractor not found at:" << extractorPath;
            if (extractionLogWidget) {
                extractionLogWidget->append(QString("<span style='color:red;'>CRITICAL ERROR: CLI extractor missing!</span>"));
                extractionLogWidget->append(QString("<span style='color:red;'>Expected location: %1</span>").arg(extractorPath));
                extractionLogWidget->append(QString("<span style='color:red;'>The GUI requires the shadps4-pkg-extractor CLI tool to extract PKG files.</span>"));
                extractionLogWidget->append(QString("<span style='color:red;'>Please set the correct path in Settings or ensure both binaries are in the same directory.</span>"));
            }
            isExtracting = false;
            processExtractionQueue(); // Try next item
            return;
        }
        
    // Create and configure the extraction process
    currentExtractionProcess = new QProcess(this);
    QElapsedTimer *durationTimer = new QElapsedTimer();
    durationTimer->start();
    QElapsedTimer *lastOutputTimer = new QElapsedTimer();
    lastOutputTimer->start();
        
        // IMPORTANT: Do NOT set ForwardedChannels - it causes crashes!
        // Use default Qt channel mode (MergedChannels) instead
        
        // Streaming output handlers for real-time feedback
        connect(currentExtractionProcess, &QProcess::readyReadStandardOutput, this, [this, lastOutputTimer]() {
            if (currentExtractionProcess && extractionLogWidget) {
                QString chunk = QString::fromLocal8Bit(currentExtractionProcess->readAllStandardOutput());
                if (!chunk.isEmpty()) {
                    lastOutputTimer->restart();
                    extractionLogWidget->append(chunk.trimmed());
                    QTextCursor c = extractionLogWidget->textCursor();
                    c.movePosition(QTextCursor::End); extractionLogWidget->setTextCursor(c);
                }
            }
        });
        connect(currentExtractionProcess, &QProcess::readyReadStandardError, this, [this, lastOutputTimer]() {
            if (currentExtractionProcess && extractionLogWidget) {
                QString chunk = QString::fromLocal8Bit(currentExtractionProcess->readAllStandardError());
                if (!chunk.trimmed().isEmpty()) {
                    lastOutputTimer->restart();
                    extractionLogWidget->append(QString("<span style='color:red;'>%1</span>").arg(chunk.trimmed()));
                    QTextCursor c = extractionLogWidget->textCursor();
                    c.movePosition(QTextCursor::End); extractionLogWidget->setTextCursor(c);
                }
            }
        });
        // Process error diagnostics
        connect(currentExtractionProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err){
            if (extractionLogWidget) {
                extractionLogWidget->append(QString("<span style='color:red;'>[process error] code=%1</span>").arg(static_cast<int>(err)));
            }
        });

        // Heartbeat timer to reassure user during long periods of silence
        QTimer *heartbeat = new QTimer(this);
        heartbeat->setInterval(5000);
        connect(heartbeat, &QTimer::timeout, this, [this, durationTimer]() {
            if (extractionLogWidget) {
                extractionLogWidget->append(QString("[heartbeat] still extracting... %1s elapsed")
                                              .arg(durationTimer->elapsed()/1000));
            }
        });
        heartbeat->start();
        
    connect(currentExtractionProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, request, durationTimer, heartbeat, lastOutputTimer](int exitCode, QProcess::ExitStatus exitStatus) {
            qDebug() << "Extraction process finished. Exit code:" << exitCode << "Status:" << exitStatus;

            // Read any remaining buffered output
            QString stdOut, stdErr;
            if (currentExtractionProcess) {
                stdOut = QString::fromLocal8Bit(currentExtractionProcess->readAllStandardOutput());
                stdErr = QString::fromLocal8Bit(currentExtractionProcess->readAllStandardError());
            }
            if (extractionLogWidget) {
                if (!stdOut.trimmed().isEmpty()) {
                    extractionLogWidget->append(stdOut.trimmed());
                }
                if (!stdErr.trimmed().isEmpty()) {
                    extractionLogWidget->append(QString("<span style='color:red;'>%1</span>").arg(stdErr.trimmed()));
                }
            }
            
            // Log completion status
            if (extractionLogWidget) {
                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    extractionLogWidget->append("\n✓ EXTRACTION COMPLETED SUCCESSFULLY\n");
                } else {
                    extractionLogWidget->append(QString("\n✗ EXTRACTION FAILED - Exit code: %1\n").arg(exitCode));
                }
                QTextCursor cursor = extractionLogWidget->textCursor();
                cursor.movePosition(QTextCursor::End);
                extractionLogWidget->setTextCursor(cursor);
            }
            
            // Clean up process and heartbeat
            if (currentExtractionProcess) { currentExtractionProcess->deleteLater(); currentExtractionProcess = nullptr; }
            if (heartbeat) { heartbeat->stop(); heartbeat->deleteLater(); }
            qint64 ms = durationTimer ? durationTimer->elapsed() : 0;
            delete durationTimer;
            delete lastOutputTimer;
            
            // Check result
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                qDebug() << "✓ PKG extraction completed successfully:" << request.pkgPath;
                qDebug() << "  Extracted to:" << request.outputPath;
                
                // NO popup on success - just continue silently
                if (extractionLogWidget) {
                    extractionLogWidget->append(QString("Finished in %1s").arg(ms/1000.0, 0, 'f', 1));
                }
                // Process next item in queue
                onExtractionComplete();
            } else {
                // ERROR occurred - log and skip to next
                qDebug() << "✗ CLI extractor FAILED:" << request.pkgPath;
                qDebug() << "  Exit code:" << exitCode << "Status:" << exitStatus;
                if (extractionLogWidget) {
                    extractionLogWidget->append(QString("<span style='color:red;'>PKG extraction failed. Skipping to next in queue...</span>"));
                }
                
                // Emit failure signal
                QString errorMsg = QString("Exit code: %1").arg(exitCode);
                emit extractionFailed(request.pkgPath, errorMsg, request.pkgType);
                
                // Continue with next item
                onExtractionComplete();
            }
        });
        
        // Start the extraction process
        QStringList arguments;
        arguments << request.pkgPath << request.outputPath;
        
        // Set working directory to the extractor's directory
        QString extractorDir = QFileInfo(extractorPath).absolutePath();
        currentExtractionProcess->setWorkingDirectory(extractorDir);
        
        // DEBUG: Print environment
        qDebug() << "QProcess environment:";
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        qDebug() << "LD_LIBRARY_PATH:" << env.value("LD_LIBRARY_PATH", "(not set)");
        qDebug() << "PATH:" << env.value("PATH");
        
        qDebug() << "Starting extractor:" << extractorPath << "with args:" << arguments;
        qDebug() << "Working directory:" << extractorDir;
        currentExtractionProcess->start(extractorPath, arguments);
        
        if (!currentExtractionProcess->waitForStarted(5000)) {
            qDebug() << "ERROR: Failed to start extraction process";
            QString errorMsg = currentExtractionProcess->errorString();
            if (extractionLogWidget) {
                extractionLogWidget->append(QString("<span style='color:red;'>ERROR: Failed to start process - %1</span>")
                    .arg(errorMsg));
            }
            
            // Emit failure signal
            emit extractionFailed(request.pkgPath, errorMsg, request.pkgType);
            
            currentExtractionProcess->deleteLater();
            currentExtractionProcess = nullptr;
            isExtracting = false;
            processExtractionQueue(); // Try next item
        }
    }
    
    void onExtractionComplete() {
        qDebug() << "onExtractionComplete() called";
        
        // Mark extraction as complete
        isExtracting = false;
        
        // Check if there are more items to process
        if (!extractionQueue.isEmpty()) {
            qDebug() << "Processing next extraction - queue size:" << extractionQueue.size();
            // Small delay to ensure process cleanup is complete
            QTimer::singleShot(500, this, &MainWindow::processExtractionQueue);
        } else {
            qDebug() << "All extractions completed - queue is empty";
            
            // Refresh game library after all extractions complete
            if (gameLibrary) {
                QTimer::singleShot(200, gameLibrary, &GameLibrary::refreshLibrary);
            }
            
            // Log completion message
            if (extractionLogWidget) {
                extractionLogWidget->append("\n=== ALL EXTRACTIONS COMPLETE ===\n");
            }
        }
    }

private:

    // Detect PKG type from filename using the same logic as downloads_folder
    PkgType detectPkgType(const QString& baseName) {
        // Convert to lowercase for case-insensitive matching
        QString lowerBaseName = baseName.toLower();
        QString originalBaseName = baseName; // Keep original case for regex matching
        
        // First check for DLC patterns (most specific)
        if (lowerBaseName.contains("dlc") ||
            lowerBaseName.contains("addon") ||
            lowerBaseName.contains("season") ||
            lowerBaseName.contains("expansion") ||
            lowerBaseName.contains("-ac") || lowerBaseName.contains("_ac") ||
            // Only consider _fxd as DLC if it explicitly contains "DLC" in the name
            (lowerBaseName.contains("_fxd") && lowerBaseName.contains("dlc")) ||
            QRegularExpression("dlc\\d+", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch()) {
            return PkgType::DLC;
        }
        
        // Then check for update/patch patterns (be more specific about updates)
        if (originalBaseName.contains("PATCH", Qt::CaseInsensitive) ||
            originalBaseName.contains("UPDATE", Qt::CaseInsensitive) ||
            lowerBaseName.contains("_update") ||
            lowerBaseName.contains("-update") ||
            lowerBaseName.contains("backport") ||
            // Only consider versions > 1.00 as updates, not v1.00 which is usually base
            QRegularExpression("v([2-9]\\d*\\.\\d+|1\\.[1-9]\\d*|1\\.0[1-9])", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch() ||
            // A0101 and higher (A0100 is base, A0101+ are updates)
            QRegularExpression("A0(10[1-9]|1[1-9]\\d|[2-9]\\d\\d)", QRegularExpression::CaseInsensitiveOption).match(originalBaseName).hasMatch()) {
            return PkgType::Update;
        }
        
        // Default to base game
        return PkgType::BaseGame;
    }

    GameLibrary* gameLibrary;
};

// ---- MainWindow method definitions ----
void MainWindow::onSettingsChanged() {
    if (gameLibrary) {
        gameLibrary->refreshLibrary();
    }
}

void MainWindow::connectSignals() {
    // Connect game launch signals to music control
    connect(gameLibrary, &GameLibrary::gameLaunched, this, [this]() {
        if (musicPlayer) {
            musicPlayer->pause();
        }
    });
    
    connect(gameLibrary, &GameLibrary::gameFinished, this, [this]() {
        if (musicPlayer) {
            musicPlayer->play();
        }
    });
}

void MainWindow::setupUI() {
    setWindowTitle("ShadPs4 Manager");
    setMinimumSize(1200, 800);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Exit button row in top right
    QHBoxLayout* topButtonLayout = new QHBoxLayout();
    topButtonLayout->addStretch();
    QPushButton* exitButton = new QPushButton("✕ Exit", this);
    exitButton->setStyleSheet("QPushButton { background-color: #DC143C; color: white; font-weight: bold; padding: 8px 16px; border-radius: 4px; } QPushButton:hover { background-color: #B22222; }");
    exitButton->setFixedWidth(100);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);
    topButtonLayout->addWidget(exitButton);
    mainLayout->addLayout(topButtonLayout);

    // Animated title header with dancing letters (full width)
    titleWidget = new AnimatedTitleWidget(this);
    mainLayout->addWidget(titleWidget);

    // Tabs
    QTabWidget* tabWidget = new QTabWidget(this);
    mainTabWidget = tabWidget;  // Store reference for tab switching
    gameLibrary = new GameLibrary();
    tabWidget->addTab(gameLibrary, "Game Library");
    DownloadsFolder* downloadsFolder = new DownloadsFolder();
    tabWidget->addTab(downloadsFolder, "Downloads Folder");
    InstallationFolder* installationFolder = new InstallationFolder();
    tabWidget->addTab(installationFolder, "Installed Games");
    
    // Extraction Log tab with both extraction output and failure log
    extractionTabWidget = new QWidget();
    QVBoxLayout* extractionTabLayout = new QVBoxLayout(extractionTabWidget);
    
    // Extraction output (real-time log)
    auto* extractionLabel = new QLabel("<b>📝 Extraction Output</b>");
    extractionTabLayout->addWidget(extractionLabel);
    
    extractionLogWidget = new QTextEdit();
    extractionLogWidget->setReadOnly(true);
    extractionLogWidget->setFont(QFont("Monospace", 9));
    extractionLogWidget->setPlaceholderText("Extraction output will appear here...");
    extractionTabLayout->addWidget(extractionLogWidget, 2); // Give it more space
    
    // Failure log section
    auto* failureLogGroup = new QGroupBox("⚠️ Installation Failures");
    auto* failureLogLayout = new QVBoxLayout(failureLogGroup);
    
    auto* failureLogWidget = new QListWidget();
    failureLogWidget->setMaximumHeight(150);
    failureLogWidget->setAlternatingRowColors(true);
    failureLogWidget->setStyleSheet(
        "QListWidget { background-color: #3a2a2a; color: #ff6b6b; font-family: monospace; font-size: 11px; }"
        "QListWidget::item { padding: 4px; border-bottom: 1px solid #4a3a3a; }"
    );
    
    auto* clearFailuresButton = new QPushButton("Clear Failures");
    clearFailuresButton->setMaximumWidth(120);
    connect(clearFailuresButton, &QPushButton::clicked, this, [failureLogWidget]() {
        failureLogWidget->clear();
    });
    
    failureLogLayout->addWidget(failureLogWidget);
    failureLogLayout->addWidget(clearFailuresButton, 0, Qt::AlignLeft);
    extractionTabLayout->addWidget(failureLogGroup, 1); // Give it less space
    
    tabWidget->addTab(extractionTabWidget, "Extraction Log");
    
    // Connect failure signal to the new failure log widget in extraction tab
    auto addFailureLog = [failureLogWidget](const QString& pkgPath, const QString& errorMessage, PkgType pkgType) {
        QFileInfo pkgInfo(pkgPath);
        QString fileName = pkgInfo.fileName();
        
        // Determine type label
        QString typeLabel;
        switch(pkgType) {
            case PkgType::BaseGame: typeLabel = "Base Game"; break;
            case PkgType::Update: typeLabel = "Update"; break;
            case PkgType::DLC: typeLabel = "DLC"; break;
            default: typeLabel = "Unknown"; break;
        }
        
        QString failureMsg = QString("[%1] [%2] %3: %4")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(typeLabel)
            .arg(fileName)
            .arg(errorMessage);
        failureLogWidget->addItem(failureMsg);
    };
    connect(this, &MainWindow::extractionFailed, this, addFailureLog);
    
    // Add Settings as a tab
    SettingsPage* settingsPage = new SettingsPage();
    tabWidget->addTab(settingsPage, "Settings");
    connect(settingsPage, &SettingsPage::settingsChanged, this, &MainWindow::onSettingsChanged);
    
    mainLayout->addWidget(tabWidget);

    // Bottom controls layout
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    
    // Left side - Emulator controls (Launch and Kill only)
    QHBoxLayout* emulatorControls = new QHBoxLayout();
    QPushButton* launchButton = new QPushButton("Launch Emulator", this);
    launchButton->setStyleSheet("QPushButton { background-color: #6A5ACD; color: white; font-weight: bold; padding: 8px; }");
    connect(launchButton, &QPushButton::clicked, this, &MainWindow::launchEmulator);
    emulatorControls->addWidget(launchButton);
    
    QPushButton* killButton = new QPushButton("Kill ShadPS4", this);
    killButton->setStyleSheet("QPushButton { background-color: #DC143C; color: white; font-weight: bold; padding: 8px; }");
    connect(killButton, &QPushButton::clicked, this, &MainWindow::killShadPS4);
    emulatorControls->addWidget(killButton);
    
    bottomLayout->addLayout(emulatorControls);
    bottomLayout->addStretch();
    
    // Right side - Music controls
    QHBoxLayout* musicControls = new QHBoxLayout();
    
    trackLabel = new QLabel("♫ No track", this);
    trackLabel->setStyleSheet("QLabel { color: #888; font-style: italic; }");
    trackLabel->setMinimumWidth(250);
    musicControls->addWidget(trackLabel);
    
    QPushButton* prevButton = new QPushButton("⏮", this);
    prevButton->setMaximumWidth(40);
    connect(prevButton, &QPushButton::clicked, this, &MainWindow::playPreviousTrack);
    musicControls->addWidget(prevButton);
    
    QPushButton* playPauseButton = new QPushButton("⏯", this);
    playPauseButton->setMaximumWidth(40);
    connect(playPauseButton, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    musicControls->addWidget(playPauseButton);
    
    QPushButton* stopButton = new QPushButton("⏹", this);
    stopButton->setMaximumWidth(40);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopMusic);
    musicControls->addWidget(stopButton);
    
    QPushButton* nextButton = new QPushButton("⏭", this);
    nextButton->setMaximumWidth(40);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::playNextTrack);
    musicControls->addWidget(nextButton);
    
    bottomLayout->addLayout(musicControls);
    
    mainLayout->addLayout(bottomLayout);
    connect(downloadsFolder, &DownloadsFolder::extractionRequested, this, &MainWindow::extractPkgFile);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("ShadPs4 Manager");
    app.setApplicationVersion("1.0.1");
    app.setOrganizationName("ShadPs4");

    // Create splash screen
    QPixmap splashPixmap(400, 200);
    splashPixmap.fill(QColor(40, 40, 50));
    
    QPainter painter(&splashPixmap);
    painter.setPen(Qt::white);
    
    // Title
    QFont titleFont = painter.font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(splashPixmap.rect(), Qt::AlignCenter, "ShadPs4 Manager");
    
    // Version
    QFont versionFont = painter.font();
    versionFont.setPointSize(10);
    versionFont.setBold(false);
    painter.setFont(versionFont);
    painter.drawText(QRect(0, 120, 400, 30), Qt::AlignCenter, "Version 1.0.1");
    
    // Info text
    painter.drawText(QRect(0, 150, 400, 30), Qt::AlignCenter, "Loading application...");
    painter.end();
    
    QSplashScreen splash(splashPixmap);
    splash.show();
    app.processEvents();
    
    // Verify CLI extractor exists
    QString extractorPath = Settings::instance().getPkgExtractorPath();
    splash.showMessage("Checking dependencies...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();
    
    if (!QFile::exists(extractorPath)) {
        splash.close();
        QMessageBox::critical(nullptr, "Missing Dependency",
            QString("Critical: CLI extraction tool not found!\n\n"
                    "Expected location: %1\n\n"
                    "The GUI requires the shadps4-pkg-extractor CLI tool.\n"
                    "Please set the correct path in Settings > PKG Extractor Tool Path.\n\n"
                    "If you haven't built it yet, please rebuild the project completely.").arg(extractorPath));
        // Don't exit - let user configure path in Settings
    }
    
    splash.showMessage("Initializing UI...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();
    QThread::msleep(300);

    MainWindow window;
    window.show();
    splash.finish(&window);

    return app.exec();
}

#include "working_gui.moc"