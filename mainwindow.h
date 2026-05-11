#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QDateTime>

#include <QCamera>
#include <QCameraInfo>
#include <QVideoWidget>

class QVideoWidget; // 前向声明

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
//    void on_refreshBtn_clicked();
    void refreshPorts();
    void connectSerial();
    void disconnectSerial();
    void readSN();
    void setPressures();
    void sendReset();
    void startReading();
    void stopReading();
    void autoReadPressure();

    void logMessage(const QString &msg);


private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *timer;

    QString sendCommand(const QString &cmd);  // <-- 声明 sendCommand

    // 新增：摄像头相关
    QCamera *camera = nullptr;
    QVideoWidget *videoWidget = nullptr;

    // 在 class MainWindow 的 private: 部分添加以下成员变量
//        Ui::MainWindow *ui;
//        QSerialPort *serial;
//        QTimer *timer; // 原有的定时器（如果用于其他功能保留，电机控制我们直接发）

    // --- 新增：电机控制相关成员 ---
    QSerialPort *motorSerial; // 如果电机和压力计用同一个串口，可以用同一个serial对象
                               // 如果是独立串口，请在这里声明；如果是同一个，可以不用声明新的

    // --- 新增：UI 控件指针 (如果你在 Designer 中命名了这些控件，请确保名字一致) ---
    // 注意：通常不需要在这里声明指针，直接用 ui->xxx 访问即可。
    // 除非你动态创建控件，否则这里不需要加变量，直接在 cpp 里用 ui->motorSpeedEdit 访问。

    // --- 新增：槽函数声明 ---
private slots:
    // ... 原有的 slots ...

    // --- 新增：电机控制槽函数 ---
    void on_connectMotorBtn_clicked(); // 连接电机串口
    void on_startMotorBtn_clicked();   // 启动电机
    void on_stopMotorBtn_clicked();    // 停止电机

    // --- 新增：辅助函数声明 (C++中通常直接写在cpp里作为私有函数) ---
//        void sendMotorCommand(const QByteArray &data);
    // --- 新增：自动连接相关函数声明 ---
private:
        void autoConnectDevices();
        void connectByVidPid(int targetVid, int targetPid, const QString &deviceName);

};
#endif // MAINWINDOW_H
