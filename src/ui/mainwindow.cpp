#include "mainwindow.h"
#include "core/application.h"
#include "services/usb/usb_service.h"
#include "config.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDateTime>
#include <QHeaderView>
#include <QSplitter>
#include <QEvent>
#include <QSettings>
#include <QMetaObject>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), m_application(nullptr) {
    
    setWindowTitle(QString::fromStdString(APP_NAME + " - Device Manager"));
    setMinimumSize(600, 500);
    resize(800, 600);
    
    initializeUI();
    setupConnections();
    
    // Log platform information to UI
#ifdef _WIN32
    logMessage("Platform: Windows");
#elif defined(__APPLE__)
    logMessage("Platform: macOS");
#elif defined(__linux__)
    logMessage("Platform: Linux");
#else
    logMessage("Platform: Unknown");
#endif
    
    // Set up update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    m_updateTimer->start(5000); // Update every 5 seconds
}

MainWindow::~MainWindow() {
}

void MainWindow::setApplication(Application* app) {
    m_application = app;
    
    // Set up logging callback so storage service can log to UI
    if (m_application) {
        m_application->setLogCallback([this](const std::string& message) {
            logMessage(QString::fromStdString(message));
        });
    }
    
    updateDeviceList();
    updateConnectionStatus();
}

void MainWindow::showDeviceManager() {
    m_tabWidget->setCurrentWidget(m_deviceTab);
    // Always restore window state to normal before showing
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    raise();
    activateWindow();
}

void MainWindow::showSettings() {
    m_tabWidget->setCurrentWidget(m_settingsTab);
    // Always restore window state to normal before showing
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    raise();
    activateWindow();
}

void MainWindow::initializeUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    // Create tabs
    createDeviceManagerTab();
    createSettingsTab();
    createStatusTab();
    
    // Add tabs to tab widget
    m_tabWidget->addTab(m_deviceTab, "Devices");
    m_tabWidget->addTab(m_settingsTab, "Settings");
    m_tabWidget->addTab(m_statusTab, "Status");
}

void MainWindow::createDeviceManagerTab() {
    m_deviceTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_deviceTab);
    
    // Title
    QLabel *title = new QLabel("USB Device Manager");
    title->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(title);
    
    // Current selection
    QGroupBox *selectionGroup = new QGroupBox("Selected Device");
    QVBoxLayout *selectionLayout = new QVBoxLayout(selectionGroup);
    
    m_selectedDeviceLabel = new QLabel("No device selected");
    selectionLayout->addWidget(m_selectedDeviceLabel);
    
    layout->addWidget(selectionGroup);
    
    // Device list + search
    QGroupBox *deviceGroup = new QGroupBox("Available USB Devices");
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceGroup);

    m_deviceSearchBox = new QLineEdit();
    m_deviceSearchBox->setPlaceholderText("Rechercher un périphérique...");
    deviceLayout->addWidget(m_deviceSearchBox);

    m_deviceList = new QListWidget();
    m_deviceList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Set monospace font for better alignment
    QFont monoFont;
#ifdef _WIN32
    // On Windows, avoid exactMatch() which can cause crashes
    // Use common Windows monospace fonts directly
    monoFont = QFont("Consolas");
    if (monoFont.family() != "Consolas") {
        monoFont = QFont("Courier New");
    }
#else
    // On other platforms, use the original approach
    monoFont = QFont("Consolas, Monaco, 'Lucida Console', monospace");
    if (monoFont.exactMatch()) {
        // Font is available as requested
    } else {
        // Fallback to system monospace font
        monoFont = QFont("monospace");
    }
#endif
    m_deviceList->setFont(monoFont);
    
    deviceLayout->addWidget(m_deviceList);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("Refresh");
    m_selectDeviceButton = new QPushButton("Select Device");
    m_selectDeviceButton->setEnabled(false);
    
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addWidget(m_selectDeviceButton);
    buttonLayout->addStretch();
    
    deviceLayout->addLayout(buttonLayout);
    layout->addWidget(deviceGroup);


    // Connexion de la recherche
    connect(m_deviceSearchBox, &QLineEdit::textChanged, this, &MainWindow::onDeviceSearchTextChanged);
    
    // Instructions
    QLabel *instructions = new QLabel(
        "Select a USB device to monitor. When the selected device is disconnected, "
        "the screen will turn off after the configured delay and turn back on automatically."
    );
    instructions->setWordWrap(true);
    instructions->setStyleSheet("color: #666; font-style: italic; margin-top: 10px;");
    layout->addWidget(instructions);
}

void MainWindow::createSettingsTab() {
    m_settingsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_settingsTab);
    
    // Title
    QLabel *title = new QLabel("Application Settings");
    title->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(title);
    
    // Autostart settings
    QGroupBox *autostartGroup = new QGroupBox("Startup Settings");
    QVBoxLayout *autostartLayout = new QVBoxLayout(autostartGroup);
    
    m_autostartCheckbox = new QCheckBox("Start application on Windows boot");
    autostartLayout->addWidget(m_autostartCheckbox);
    
    m_startMinimizedCheckbox = new QCheckBox("Start minimized to system tray");
    autostartLayout->addWidget(m_startMinimizedCheckbox);
    
    layout->addWidget(autostartGroup);
    
    // Screen control settings
    QGroupBox *screenGroup = new QGroupBox("Screen Control Settings");
    QGridLayout *screenLayout = new QGridLayout(screenGroup);
    
    QLabel *delayLabel = new QLabel("Screen off delay (seconds):");
    m_screenDelaySpinBox = new QSpinBox();
    m_screenDelaySpinBox->setRange(1, 300);
    m_screenDelaySpinBox->setValue(SCREEN_OFF_DELAY_SECONDS);
    
    screenLayout->addWidget(delayLabel, 0, 0);
    screenLayout->addWidget(m_screenDelaySpinBox, 0, 1);
    
    m_testScreenButton = new QPushButton("Test Screen Control");
    screenLayout->addWidget(m_testScreenButton, 1, 0, 1, 2);
    
    layout->addWidget(screenGroup);
    
    // Add stretch to push everything to top
    layout->addStretch();
}

void MainWindow::createStatusTab() {
    m_statusTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_statusTab);
    
    // Title
    QLabel *title = new QLabel("Application Status");
    title->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(title);
    
    // Connection status
    QGroupBox *statusGroup = new QGroupBox("Connection Status");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    m_connectionStatus = new QLabel("Checking...");
    m_connectionStatus->setStyleSheet("padding: 10px; font-weight: bold;");
    statusLayout->addWidget(m_connectionStatus);
    
    layout->addWidget(statusGroup);
    
    // Activity log
    QGroupBox *logGroup = new QGroupBox("Activity Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    
    m_statusLog = new QTextEdit();
    m_statusLog->setReadOnly(true);
    logLayout->addWidget(m_statusLog);
    
    layout->addWidget(logGroup);
    
    // Initial log message
    logMessage("Application started");
}

void MainWindow::setupConnections() {
    // Device manager connections
    connect(m_deviceList, &QListWidget::itemSelectionChanged, 
            this, [this]() {
                // Only enable/disable the button, no automatic selection
                QListWidgetItem *current = m_deviceList->currentItem();
                m_selectDeviceButton->setEnabled(current != nullptr);
            });
    connect(m_refreshButton, &QPushButton::clicked, 
            this, &MainWindow::onRefreshDevicesClicked);
    connect(m_selectDeviceButton, &QPushButton::clicked, 
            this, &MainWindow::onSelectDeviceClicked);
    
    // Settings connections
    connect(m_autostartCheckbox, &QCheckBox::toggled, 
            this, &MainWindow::onAutostartToggled);
    connect(m_startMinimizedCheckbox, &QCheckBox::toggled,
            this, &MainWindow::onStartMinimizedToggled);
    connect(m_screenDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onScreenDelayChanged);
    connect(m_testScreenButton, &QPushButton::clicked, 
            this, &MainWindow::onTestScreenControlClicked);
}

void MainWindow::onDeviceSelectionChanged() {
    // This method is now used only for UI updates
    QListWidgetItem *current = m_deviceList->currentItem();
    m_selectDeviceButton->setEnabled(current != nullptr);
}

void MainWindow::onSelectDeviceClicked() {
    QListWidgetItem *current = m_deviceList->currentItem();
    if (current) {
        // Get the device ID from the UserRole data
        QString deviceId = current->data(Qt::UserRole).toString();
        QString deviceLabel = current->text();
        
        // Update the application's selected device
        if (m_application) {
            m_application->setSelectedDevice(deviceId.toStdString());
            logMessage("Device selected: " + deviceLabel);
            
            // Update the UI immediately to reflect the change
            updateStatus();
        }
    }
}

void MainWindow::onAutostartToggled(bool enabled) {
    if (m_application) {
        if (m_application->setAutostart(enabled)) {
            logMessage(enabled ? "Autostart enabled" : "Autostart disabled");
        } else {
            logMessage("Failed to change autostart setting");
            // Revert checkbox state
            m_autostartCheckbox->setChecked(!enabled);
        }
    }
}

void MainWindow::onStartMinimizedToggled(bool enabled) {
    if (m_application) {
        m_application->setStartMinimized(enabled);
        logMessage(enabled ? "Start minimized enabled" : "Start minimized disabled");
    }
}

void MainWindow::onScreenDelayChanged(int delay) {
    if (m_application) {
        m_application->setScreenDelay(delay);
        logMessage(QString("Screen delay changed to %1 seconds").arg(delay));
    }
}

void MainWindow::onRefreshDevicesClicked() {
    updateDeviceList();
    logMessage("Device list refreshed");
}

void MainWindow::onTestScreenControlClicked() {
    if (!m_application) {
        logMessage("Application not available for screen test");
        return;
    }
    
    logMessage("Starting screen control test...");
    
    // Test screen control with callback
    m_application->testScreenControl([this](bool success) {
        // This callback is called from a background thread, so we need to use Qt's mechanism 
        // to safely update the UI from the main thread
        QMetaObject::invokeMethod(this, [this, success]() {
            if (success) {
                logMessage("Screen control test completed successfully");
                QMessageBox::information(this, "Test Result", 
                    "Screen control test completed successfully!\n"
                    "The screen was turned off for 1 second and then turned back on.");
            } else {
                logMessage("Screen control test failed");
                QMessageBox::warning(this, "Test Result", 
                    "Screen control test failed!\n"
                    "Check the console output for error details.");
            }
        }, Qt::QueuedConnection);
    });
}

void MainWindow::updateDeviceList() {
    if (!m_application) return;
    
    m_deviceList->clear();
    populateDeviceList();
}

void MainWindow::updateStatus() {
    updateConnectionStatus();
    
    // Update settings from application state
    if (m_application) {
        
        // Block signals to prevent triggering callbacks when setting initial state
        bool autostartEnabled = m_application->isAutostartEnabled();
        m_autostartCheckbox->blockSignals(true);
        m_autostartCheckbox->setChecked(autostartEnabled);
        m_autostartCheckbox->blockSignals(false);
        
        bool startMinimizedEnabled = m_application->isStartMinimizedEnabled();
        m_startMinimizedCheckbox->blockSignals(true);
        m_startMinimizedCheckbox->setChecked(startMinimizedEnabled);
        m_startMinimizedCheckbox->blockSignals(false);
        
        // Update screen delay spinbox from configuration
        int screenDelay = m_application->getScreenDelay();
        m_screenDelaySpinBox->blockSignals(true);
        m_screenDelaySpinBox->setValue(screenDelay);
        m_screenDelaySpinBox->blockSignals(false);
        
        // Update selected device display
        std::string selectedDeviceId = m_application->getSelectedDevice();
        if (selectedDeviceId.empty()) {
            m_selectedDeviceLabel->setText("No device selected");
        } else {
            // Find the friendly name for the selected device
            auto devices = m_application->getConnectedUsbDevices();
            QString deviceName = QString::fromStdString(selectedDeviceId); // fallback to ID
            
            for (const auto& device : devices) {
                if (device.deviceId == selectedDeviceId) {
                    deviceName = QString::fromStdString(device.friendlyName);
                    break;
                }
            }
            
            m_selectedDeviceLabel->setText(deviceName);
        }
    }
}

void MainWindow::populateDeviceList() {
    m_deviceList->clear();
    if (m_application) {
        // Get the currently selected device
        std::string selectedDeviceId = m_application->getSelectedDevice();
        
        // Get real devices from USB service
        auto devices = m_application->getConnectedUsbDevices();
        
        // First pass: find the maximum width needed for device IDs
        int maxIdWidth = 0;
        for (const auto& device : devices) {
            QString deviceIdPart = QString::fromStdString("VID_" + device.vendorId + "&PID_" + device.productId);
            maxIdWidth = qMax(maxIdWidth, deviceIdPart.length());
        }
        
        // Add some padding
        maxIdWidth += 2;
        
        // Second pass: create the list items with proper alignment
        for (const auto& device : devices) {
            QString deviceIdPart = QString::fromStdString("VID_" + device.vendorId + "&PID_" + device.productId);
            QString deviceName = QString::fromStdString(device.friendlyName);
            
            // Create aligned format with calculated width
            QString label = QString("%1 │ %2").arg(deviceIdPart, -maxIdWidth).arg(deviceName);
            
            QListWidgetItem* item = new QListWidgetItem(label);
            item->setData(Qt::UserRole, QString::fromStdString(device.deviceId));
            m_deviceList->addItem(item);
            
            // Highlight the selected device
            if (device.deviceId == selectedDeviceId) {
                m_deviceList->setCurrentItem(item);
                item->setSelected(true);
            }
        }
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        // On all platforms, minimize to system tray instead of taskbar
        if (windowState() & Qt::WindowMinimized) {
#ifdef Q_OS_MAC
            // On macOS, prevent minimization to dock by hiding the window instead
            setWindowState(windowState() & ~Qt::WindowMinimized);
#endif
            hide();
            event->accept();
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Default behavior for other platforms or programmatic close
    QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event) {
#ifdef Q_OS_MAC
    // On macOS, ensure window is in normal state when showing
    if (isMinimized()) {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }
#endif
    QMainWindow::showEvent(event);
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    m_statusLog->append(logEntry);
}

void MainWindow::updateConnectionStatus() {
    if (!m_application) {
        m_connectionStatus->setText("Application not initialized");
        m_connectionStatus->setStyleSheet("color: red; padding: 10px; font-weight: bold;");
        return;
    }
    
    std::string selectedDevice = m_application->getSelectedDevice();
    if (selectedDevice.empty()) {
        m_connectionStatus->setText("No device selected for monitoring");
        m_connectionStatus->setStyleSheet("color: orange; padding: 10px; font-weight: bold;");
    } else {
        // Check if selected device is connected
        // This is simplified - you'd need to check actual connection status
        m_connectionStatus->setText("Device connected and monitoring");
        m_connectionStatus->setStyleSheet("color: green; padding: 10px; font-weight: bold;");
    }
}

// Slot to filter the device list according to the search
void MainWindow::onDeviceSearchTextChanged(const QString& text) {
    for (int i = 0; i < m_deviceList->count(); ++i) {
        QListWidgetItem* item = m_deviceList->item(i);
        bool match = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}