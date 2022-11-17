#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mcp2221a.h"
#include <QTimer>
#include <QTime>


class  MCP2221A;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int     workState;   //设备连接标志位
    int     SMBusMode;
    int     errorCode;   //IIC通讯标志位
    void    pringfError(int);
    int     qStringToInt(QString);
    QString toHexStr(QByteArray data, int len = 0);
    void    SaveConfig(); //用于保存上次配置
    void    LoadConfig(); //加载上次配置
    void    getDeviceInfo();//获取配置信息



private:
    Ui::MainWindow *ui;
    MCP2221A *mcp2221a;
    QTimer *timer;     //用于轮询定时
    QTimer *nowtimer;  //用于更新时间显示



private slots:
    void setI2cSpeed(int);
    void openButtonClicked();
    void pollPushButtonClicked();
    void stopPushButtonClicked();
    void checkByteCountPushButtonClicked();
    void timerTimeOut();
    void timeUpdata();
    void on_SMbusPushButton_clicked();
    void on_radioButton_clicked();
    void on_radioButton_4_clicked();
    void on_radioButton_3_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_5_clicked();
    void on_radioButton_6_clicked();
    void on_radioButton_7_clicked();
    void cleanDataPushButton_clicked();
    void updataPushButton_clicked();
};
#endif // MAINWINDOW_H
