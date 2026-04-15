#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
    , timer(new QTimer(this))
{
    ui->setupUi(this);

    // 初始化状态
    ui->disconnectBtn->setEnabled(false);
    ui->stopReadBtn->setEnabled(false);

    // 连接信号
    connect(ui->refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(ui->connectBtn, &QPushButton::clicked, this, &MainWindow::connectSerial);
    connect(ui->disconnectBtn, &QPushButton::clicked, this, &MainWindow::disconnectSerial);
    connect(ui->readSnBtn, &QPushButton::clicked, this, &MainWindow::readSN);
    connect(ui->applyPressureBtn, &QPushButton::clicked, this, &MainWindow::setPressures);
    connect(ui->resetBtn, &QPushButton::clicked, this, &MainWindow::sendReset);
    connect(ui->startReadBtn, &QPushButton::clicked, this, &MainWindow::startReading);
    connect(ui->stopReadBtn, &QPushButton::clicked, this, &MainWindow::stopReading);
    connect(timer, &QTimer::timeout, this, &MainWindow::autoReadPressure);

    refreshPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logMessage(const QString &msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->logText->append(QString("[%1] %2").arg(timeStr).arg(msg));
}

void MainWindow::refreshPorts()
{
    ui->portCombo->clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        ui->portCombo->addItem(info.portName());
    }
    if (ui->portCombo->count() == 0) {
        ui->portCombo->addItem("无可用串口");
    }
}

//void MainWindow::on_refreshBtn_clicked()
//{
//    ui->portCombo->clear();
//    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
//        ui->portCombo->addItem(info.portName());
//    }
//    if (ui->portCombo->count() == 0) {
//        ui->portCombo->addItem("无可用串口");
//    }
//}

void MainWindow::connectSerial()
{
    QString portName = ui->portCombo->currentText();
    if (portName == "无可用串口") {
        QMessageBox::warning(this, "错误", "没有可用串口！");
        return;
    }

    serial->setPortName(portName);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadWrite)) {
        ui->connectBtn->setEnabled(false);
        ui->disconnectBtn->setEnabled(true);
        logMessage("已连接到 " + portName);
    } else {
        QMessageBox::critical(this, "连接失败", "无法打开串口:\n" + serial->errorString());
        logMessage("连接失败: " + serial->errorString());
    }
}

void MainWindow::disconnectSerial()
{
    if (serial->isOpen()) {
        serial->close();
    }
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    logMessage("串口已断开");
}

QString MainWindow::sendCommand(const QString &cmd)
{
    if (!serial->isOpen()) {
        logMessage("错误: 串口未连接");
        return "";
    }

    QByteArray data = cmd.toLocal8Bit();
    serial->write(data);
    serial->waitForBytesWritten(100);

    if (serial->waitForReadyRead(1000)) {
        QByteArray response = serial->readAll();
        while (serial->waitForReadyRead(10)) {
            response += serial->readAll();
        }
        QString respStr = QString::fromLocal8Bit(response).trimmed();
        logMessage(QString("← 发送: %1 → 接收: %2").arg(cmd.trimmed()).arg(respStr));
        return respStr;
    } else {
        logMessage("超时：无响应");
        return "";
    }
}

void MainWindow::readSN()
{
    QString resp = sendCommand("a");
    if (!resp.isEmpty()) {
        ui->snLabel->setText("序列号: " + resp);
    } else {
        ui->snLabel->setText("序列号: 读取失败");
    }
}

void MainWindow::setPressures()
{
    double p1 = ui->ch1Spin->value();
    double p2 = ui->ch2Spin->value();
    double p3 = ui->ch3Spin->value();
    double p4 = ui->ch4Spin->value();
    double sum = p1 + p2 + p3 + p4;

    QString cmd = QString("w%1,%2,%3,%4,%5\r")
                  .arg(p1, 0, 'f', 2)
                  .arg(p2, 0, 'f', 2)
                  .arg(p3, 0, 'f', 2)
                  .arg(p4, 0, 'f', 2)
                  .arg(sum, 0, 'f', 2);

    QString resp = sendCommand(cmd);
    if (resp.contains("transmission OK")) {
        QMessageBox::information(this, "成功", "压力设定已发送");
    } else {
        QMessageBox::warning(this, "警告", "设定失败: " + resp);
    }
}

void MainWindow::sendReset()
{
    if (!serial->isOpen()) return;
    serial->write("i");
    serial->waitForBytesWritten(100);
    logMessage("已发送复位命令 'i'");
}

void MainWindow::autoReadPressure()
{
    QString resp = sendCommand("r");
    if (!resp.isEmpty()) {
        QStringList parts = resp.split(',');
        if (parts.size() == 4) {
            QString text = QString("Ch1: %1 | Ch2: %2 | Ch3: %3 | Ch4: %4")
                           .arg(parts[0]).arg(parts[1]).arg(parts[2]).arg(parts[3]);
            ui->pressureDisplay->setText("当前压力: " + text);
        } else {
            ui->pressureDisplay->setText("当前压力: 格式错误");
        }
    } else {
        ui->pressureDisplay->setText("当前压力: 无响应");
    }
}

void MainWindow::startReading()
{
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "错误", "请先连接串口！");
        return;
    }
    timer->start(1000); // 每秒一次
    ui->startReadBtn->setEnabled(false);
    ui->stopReadBtn->setEnabled(true);
    logMessage("开始自动读取压力...");
}

void MainWindow::stopReading()
{
    timer->stop();
    ui->startReadBtn->setEnabled(true);
    ui->stopReadBtn->setEnabled(false);
    logMessage("停止自动读取");
}


