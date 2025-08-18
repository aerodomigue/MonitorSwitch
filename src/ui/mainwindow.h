#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QGroupBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QTimer>

// Forward declaration
class Application;
struct UsbDevice;

class MainWindow : public QMainWindow {
protected:
    void changeEvent(QEvent* event) override;
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setApplication(Application* app);
    void showDeviceManager();
    void showSettings();
    void updateStatus();
    void updateDeviceList();

private slots:
    void onDeviceSelectionChanged();
    void onSelectDeviceClicked();
    void onAutostartToggled(bool enabled);
    void onScreenDelayChanged(int delay);
    void onRefreshDevicesClicked();
    void onTestScreenControlClicked();

private slots:
    void onDeviceSearchTextChanged(const QString& text);

private:
    void initializeUI();
    void createDeviceManagerTab();
    void createSettingsTab();
    void createStatusTab();
    void populateDeviceList();
    void setupConnections();
    void closeEvent(QCloseEvent *event) override;

    // UI Components
    QTabWidget* m_tabWidget;
    
    // Device Manager Tab
    QWidget* m_deviceTab;
    QLineEdit* m_deviceSearchBox;
    QListWidget* m_deviceList;
    QPushButton* m_refreshButton;
    QPushButton* m_selectDeviceButton;
    QLabel* m_selectedDeviceLabel;
    
    // Settings Tab
    QWidget* m_settingsTab;
    QCheckBox* m_autostartCheckbox;
    QSpinBox* m_screenDelaySpinBox;
    QPushButton* m_testScreenButton;
    
    // Status Tab
    QWidget* m_statusTab;
    QTextEdit* m_statusLog;
    QLabel* m_connectionStatus;
    
    // Core application reference
    Application* m_application;
    
    // Update timer
    QTimer* m_updateTimer;
    
    // Helper methods
    void logMessage(const QString& message);
    void updateConnectionStatus();
};

#endif // MAINWINDOW_H