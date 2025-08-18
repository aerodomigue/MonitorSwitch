#include <QApplication>
#ifdef Q_OS_MAC
#include <objc/objc-runtime.h>
#endif
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include "ui/mainwindow.h"
#include "core/application.h"
#include "../include/config.h"
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
#ifdef Q_OS_MAC
    // Hide Dock icon, only show in menu bar
    id nsApp = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
    ((void (*)(id, SEL, int))objc_msgSend)(nsApp, sel_registerName("setActivationPolicy:"), 1 /* NSApplicationActivationPolicyAccessory */);
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
        QMessageBox::critical(nullptr, "System Tray",
                             "System tray is not available on this system.");
        return 1;
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
    QIcon appIcon("icons/MonitorSwitch_icon.svg");
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
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
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
            mainWindow.show();
            mainWindow.raise();
            mainWindow.activateWindow();
        }
    });
    
    // Show tray icon
    trayIcon.show();
    
    // Always show the device manager window on startup (Linux/Windows)
    mainWindow.showDeviceManager();

    // Now that UI is ready, load configuration
    coreApplication.startConfiguration();
    
    // Update UI immediately after configuration loading
    mainWindow.updateDeviceList();
    mainWindow.updateStatus();

    std::cout << APP_NAME << " started successfully!" << std::endl;
    
    return app.exec();
}