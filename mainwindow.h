#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QDateTime>

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
};
#endif // MAINWINDOW_H
