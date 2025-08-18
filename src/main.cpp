#include <QApplication>
#ifdef Q_OS_MAC
#include <objc/objc-runtime.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <QStyle>
#include <QDir>
#include <QFile>
#ifdef _WIN32
#include <QImage>
#endif
#include "ui/mainwindow.h"
#include "core/application.h"
#include "../include/config.h"
#include <iostream>

#ifdef _WIN32
// Helper function to get the application icon on Windows
QIcon getWindowsAppIcon() {
    // Try to get the icon from the executable
    HICON hIcon = ExtractIcon(GetModuleHandle(nullptr), 
                              reinterpret_cast<LPCWSTR>(QCoreApplication::applicationFilePath().utf16()), 0);
    if (hIcon && hIcon != (HICON)1) {
        // Convert to QIcon using Qt's built-in conversion
        QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(hIcon));
        DestroyIcon(hIcon);
        if (!pixmap.isNull()) {
            return QIcon(pixmap);
        }
    }
    return QIcon();
}
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

#ifdef Q_OS_MAC
    // On macOS, we'll manage the dock icon dynamically
    // Don't hide it at startup to avoid issues
#endif
    
    // Set application properties
    app.setApplicationName(QString::fromStdString(APP_NAME));
    app.setApplicationVersion("1.0");
    app.setOrganizationName(QString::fromStdString(APP_VENDOR));
    
    // Create the core application
    Application coreApplication;
    
    // Initialize the core application
    if (!coreApplication.initialize()) {
        QMessageBox::critical(nullptr, "Initialization Error",
                             "Failed to initialize the application.");
        return 1;
    }
    
    // Create the main window (hidden by default for tray application)
    MainWindow mainWindow;
    mainWindow.setApplication(&coreApplication);
    
    // Check if system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
#ifdef Q_OS_MAC
        // On macOS, if tray is not available, fall back to classic window mode
        qWarning() << "System tray not available on macOS, showing window instead";
        mainWindow.showDeviceManager();
#else
        // On Windows/Linux, if tray is not available, show window and continue
        qWarning() << "System tray not available, showing window instead";
        mainWindow.showDeviceManager();
#endif
    }
    
    // Set application and window icon
#ifdef Q_OS_MAC
    // Correct path for macOS bundle
    QString iconPath = QCoreApplication::applicationDirPath() + "/../Resources/MonitorSwitch.png";
    QIcon appIcon(iconPath);
    if (appIcon.isNull()) {
        // Fallback: try the app directory
        iconPath = QCoreApplication::applicationDirPath() + "/icons/MonitorSwitch.png";
        appIcon = QIcon(iconPath);
        if (appIcon.isNull()) {
            // Final fallback: relative path
            appIcon = QIcon("icons/MonitorSwitch.png");
            if (appIcon.isNull()) {
                qWarning() << "Could not load icon file from any path";
            }
        }
    }
    qDebug() << "Trying icon path:" << iconPath;
#else
    // Windows and Linux icon loading with multiple fallbacks
    QIcon appIcon;
    
    // Debug: print current directory and paths
    qDebug() << "Current working directory:" << QDir::currentPath();
    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
    
#ifdef _WIN32
    // First, try to get the icon from the Windows executable
    appIcon = getWindowsAppIcon();
    if (!appIcon.isNull()) {
        qDebug() << "Successfully loaded icon from Windows executable";
    }
#endif
    
    // If no executable icon found, try file paths
    if (appIcon.isNull()) {
        // Try different icon paths in order of preference
        QStringList iconPaths = {
            QCoreApplication::applicationDirPath() + "/icons/MonitorSwitch.ico",  // Windows ICO in app dir
            QCoreApplication::applicationDirPath() + "/icons/MonitorSwitch.png",  // PNG in app dir
            QCoreApplication::applicationDirPath() + "/icons/MonitorSwitch_icon.svg", // SVG in app dir
            "icons/MonitorSwitch.ico",     // Relative ICO
            "icons/MonitorSwitch.png",     // Relative PNG  
            "icons/MonitorSwitch_icon.svg" // Relative SVG
        };
        
        for (const QString& path : iconPaths) {
            qDebug() << "Trying icon path:" << path;
            qDebug() << "File exists:" << QFile::exists(path);
            appIcon = QIcon(path);
            if (!appIcon.isNull()) {
                qDebug() << "Successfully loaded icon from:" << path;
                break;
            } else {
                qDebug() << "Failed to load icon from:" << path;
            }
        }
    }
    
    if (appIcon.isNull()) {
        qWarning() << "Could not load application icon from any path";
        // Create a default icon as fallback
        appIcon = qApp->style()->standardIcon(QStyle::SP_ComputerIcon);
        qDebug() << "Using default system icon as fallback";
    }
#endif
    app.setWindowIcon(appIcon);
    mainWindow.setWindowIcon(appIcon);
    // Create system tray icon (must stay alive for the app lifetime)
    static QSystemTrayIcon trayIcon;
    trayIcon.setIcon(appIcon);
    trayIcon.setToolTip(QString::fromStdString(APP_NAME));
    
#ifdef Q_OS_MAC
    // Debug: check that the icon is loaded
    if (appIcon.isNull()) {
        qWarning() << "Icon is null!";
    } else {
        qDebug() << "Icon loaded successfully";
    }
    
    // Check that tray is supported
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray is not available!";
    } else {
        qDebug() << "System tray is available";
    }
#endif
    
    // Create tray menu
    QMenu trayMenu;
    
    QAction *showAction = trayMenu.addAction("Show");
    QObject::connect(showAction, &QAction::triggered, [&mainWindow]() {
        // First restore the window if it's minimized
        if (mainWindow.isMinimized()) {
            mainWindow.setWindowState(mainWindow.windowState() & ~Qt::WindowMinimized);
        }
        
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
        
#ifdef Q_OS_WIN
        // On Windows, force the window to the foreground
        mainWindow.setWindowState(Qt::WindowActive);
#endif
    });
    
    trayMenu.addSeparator();
    
    QAction *devicesAction = trayMenu.addAction("Manage Devices");
    QObject::connect(devicesAction, &QAction::triggered, [&mainWindow]() {
        mainWindow.showDeviceManager();
    });
    
    QAction *settingsAction = trayMenu.addAction("Settings");
    QObject::connect(settingsAction, &QAction::triggered, [&mainWindow]() {
        mainWindow.showSettings();
    });
    
    trayMenu.addSeparator();
    
    QAction *quitAction = trayMenu.addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, [&]() {
        coreApplication.shutdown();
        app.quit();
    });
    
    trayIcon.setContextMenu(&trayMenu);
    
    // Handle tray icon activation
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, 
                     [&mainWindow](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            // First restore the window if it's minimized
            if (mainWindow.isMinimized()) {
                mainWindow.setWindowState(mainWindow.windowState() & ~Qt::WindowMinimized);
            }
            
            mainWindow.show();
            mainWindow.raise();
            mainWindow.activateWindow();
            
#ifdef Q_OS_WIN
            // On Windows, force the window to the foreground
            mainWindow.setWindowState(Qt::WindowActive);
#endif
        }
    });
    
    // Show tray icon
    trayIcon.show();
    
    // Load configuration first to check startup preferences
    coreApplication.startConfiguration();

    // Show the device manager window on startup unless start minimized is enabled
    // On macOS, the app starts hidden in system tray
    if (!coreApplication.isStartMinimizedEnabled()) {
        mainWindow.showDeviceManager();
    }

    
    // Update UI immediately after configuration loading
    mainWindow.updateDeviceList();
    mainWindow.updateStatus();

    std::cout << APP_NAME << " started successfully!" << std::endl;
    
    return app.exec();
}