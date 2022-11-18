#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mcp2221a.h"
#include <windows.h>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QButtonGroup>
//#include <QStandardItemModel>




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    workState = 0;           //设备未连接
    SMBusMode = 3;           //默认读字
    timer =new QTimer(this); //用于轮询定时器

    //设置时间显示
    /* 设置 lcd 显示为当前系统时间 */
    QTime nowTime = QTime::currentTime();
    /* 设置显示的样式 */
    ui->timelcdNumber->display(nowTime.toString("hh:mm:ss"));
    nowtimer = new QTimer(this);
    nowtimer->start(1000);

    QButtonGroup * BtnGroupA = new QButtonGroup(this);
    BtnGroupA->addButton(ui->radioButton,0);
    BtnGroupA->addButton(ui->radioButton_2,1);
    BtnGroupA->addButton(ui->radioButton_3,2);
    BtnGroupA->addButton(ui->radioButton_4,3);
    BtnGroupA->addButton(ui->radioButton_5,4);
    BtnGroupA->addButton(ui->radioButton_6,5);
    BtnGroupA->addButton(ui->radioButton_7,6);
    BtnGroupA->setExclusive(true);



    ui->deviceInfoTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//单元格不可编辑
    ui->deviceInfoTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(30,144,255);color: white;}");
    ui->workDataTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//单元格不可编辑
    ui->workDataTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(30,144,255);color: white;}");
    //先关闭排序功能
    ui->deviceInfoTableWidget->setSortingEnabled(false);
    //先关闭排序功能
    ui->workDataTableWidget->setSortingEnabled(false);




    mcp2221a = new MCP2221A(this);
    mcp2221a->mcpHandle = Mcp2221_OpenByIndex(DEFAULT_VID, DEFAULT_PID,mcp2221a->mcpIndex);
    mcp2221a->Clean_I2C_Read_Data(); //初始化数组


    ui->pollComboBox->addItem("100ms");
    ui->pollComboBox->addItem("200ms");
    ui->pollComboBox->addItem("500ms");
    ui->pollComboBox->addItem("1000ms");

    //    //设置数据框中的数据行数不超过100行，超过从初始值开始删除
    //    ui->dataTextEdit->setMaximumBlockCount(100);
    LoadConfig();

    connect(timer,SIGNAL(timeout()),this,SLOT(timerTimeOut()));
    connect(ui->openButton,SIGNAL(clicked()),this,SLOT(openButtonClicked()));
    connect(ui->pollPushButton,SIGNAL(clicked()),this,SLOT(pollPushButtonClicked()));
    connect(ui->stopPushButton,SIGNAL(clicked()),this,SLOT(stopPushButtonClicked()));
    connect(ui->cleanDataPushButton,SIGNAL(clicked()),this,SLOT(cleanDataPushButton_clicked()));
    connect(ui->checkByteCountPushButton,SIGNAL(clicked()),this,SLOT(checkByteCountPushButtonClicked()));

    connect(ui->updataPushButton,SIGNAL(clicked()),this,SLOT(updataPushButton_clicked()));
    connect(nowtimer, SIGNAL(timeout()), this,SLOT(timeUpdata()));



}

MainWindow::~MainWindow()
{
    delete ui;
    SaveConfig();
    Mcp2221_Close(mcp2221a->mcpHandle);
}

void MainWindow::pringfError(int result)
{
    QString str = "失败代码为：";
    str += QString::number(result);
    ui->statePlainTextEdit->appendPlainText(str+'\n');
}

int MainWindow::qStringToInt(QString str)
{
    //提取数字
    QString tmp;
    for(int j = 0; j < str.length(); j++)
    {
        if(str[j] >= '0' && str[j] <= '9')
            tmp.append(str[j]);
    }
    return tmp.toInt();
}

QString MainWindow::toHexStr(QByteArray data, int len)
{
    QString tempStr= "";
    QString hexStr = data.toHex();//把QByteArray转为Hex编码

    len = (len == 0)? hexStr.length() : len;//不带len参数调用时,通过length()方法获取长度
    for (int i=0; i<len; i=i+2) {
        tempStr += hexStr.mid(i, 2) + " ";//加空格
    }
    //trimmed():删除字符串开头和末尾的空格
    //toUpper():将字符串转换成大写
    return tempStr.trimmed().toUpper();
}

void MainWindow::SaveConfig()
{
    QSettings *configIniWrite = new QSettings("MyConfig.ini", QSettings::IniFormat, this);
    // ----------保存配置---------- //
    configIniWrite->setValue("/Last_Message/lineEdit",ui->SlaveAddressLineEdit->text());
}

void MainWindow::LoadConfig()
{
    // ----------读取配置并进行设置---------- //
    if ( QFile::exists("MyConfig.ini") ) {
        QSettings *configIniRead = new QSettings("MyConfig.ini", QSettings::IniFormat, this);
        ui->SlaveAddressLineEdit->setText(configIniRead->value("/Last_Message/lineEdit").toString());
    }
}

void MainWindow::getDeviceInfo()
{
    QString hexStr;
    QString strSerial ;
    //更新设备型号
    errorCode = mcp2221a->SmbusBlockRead(0x9A,10);
    //    qDebug()<<errorCode;
    if(errorCode <= 0){
        ui->statePlainTextEdit->appendPlainText("配置信息更新失败！失败代码：>> "+QString::number(errorCode,10));
    }else{
        //        QByteArray temp((const char*)(mcp2221a->I2C_Read_Data), sizeof(mcp2221a->I2C_Read_Data));
        QByteArray temp((const char*)(mcp2221a->I2C_Read_Data), errorCode);
        strSerial = QString::fromStdString(temp.toStdString());

        ui->deviceInfoTableWidget->setItem(0,1,new QTableWidgetItem(strSerial));
        ui->deviceInfoTableWidget->item(0,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        mcp2221a->Clean_I2C_Read_Data();
    }

    //更新生产日期
    errorCode = mcp2221a->SmbusBlockRead(0x9D,3);
    if(errorCode <=0 ){
        ui->statePlainTextEdit->appendPlainText("配置信息更新失败！失败代码：>> "+QString::number(errorCode,10));
    }else{
        hexStr = toHexStr(QByteArray((const char*)mcp2221a->I2C_Read_Data),errorCode);
        //        qDebug()<<hexStr;

        ui->deviceInfoTableWidget->setItem(1,1,new QTableWidgetItem(hexStr));
        ui->deviceInfoTableWidget->item(1,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        mcp2221a->Clean_I2C_Read_Data();
    }

    //更新设备ID
    errorCode = mcp2221a->SmbusBlockRead(0x99,3);
    if( errorCode<=0 ){
        ui->statePlainTextEdit->appendPlainText("配置信息更新失败！失败代码：>> "+QString::number(errorCode,10));
    }else{
        QByteArray temp2((const char*)(mcp2221a->I2C_Read_Data),errorCode);
        strSerial = QString::fromStdString(temp2.toStdString());

        ui->deviceInfoTableWidget->setItem(2,1,new QTableWidgetItem(strSerial));
        ui->deviceInfoTableWidget->item(2,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        mcp2221a->Clean_I2C_Read_Data();
    }

    //更新设生产产地
    errorCode = mcp2221a->SmbusBlockRead(0x9C,14);
    if(errorCode <=0 ){
        ui->statePlainTextEdit->appendPlainText("配置信息更新失败！失败代码：>> "+QString::number(errorCode,10));
    }else{
        QByteArray temp((const char*)(mcp2221a->I2C_Read_Data),errorCode);
        strSerial = QString::fromStdString(temp.toStdString());

        ui->deviceInfoTableWidget->setItem(3,1,new QTableWidgetItem(strSerial));
        ui->deviceInfoTableWidget->item(3,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        mcp2221a->Clean_I2C_Read_Data();
    }

    //更新芯片ID
    errorCode = mcp2221a->SmbusBlockRead(0xAD,7);
    if(errorCode <=0 ){
        ui->statePlainTextEdit->appendPlainText("配置信息更新失败！失败代码：>> "+QString::number(errorCode,10));
    }else{
        QByteArray temp((const char*)(mcp2221a->I2C_Read_Data),errorCode);
        strSerial = QString::fromStdString(temp.toStdString());

        ui->deviceInfoTableWidget->setItem(4,1,new QTableWidgetItem(strSerial));
        ui->deviceInfoTableWidget->item(4,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        mcp2221a->Clean_I2C_Read_Data();
    }

    //更新最小输入电压
    ui->deviceInfoTableWidget->setItem(5,1,new QTableWidgetItem(QString::number(mcp2221a->getVinMin(), 'f', 2)));
    //    qDebug()<<mcp2221a->getVinMin();
    ui->deviceInfoTableWidget->item(5,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输入电压
    ui->deviceInfoTableWidget->setItem(6,1,new QTableWidgetItem(QString::number(mcp2221a->getVinMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(6,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输入电流
    ui->deviceInfoTableWidget->setItem(7,1,new QTableWidgetItem(QString::number(mcp2221a->getIinMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(7,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输入功率
    ui->deviceInfoTableWidget->setItem(8,1,new QTableWidgetItem(QString::number(mcp2221a->getPinMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(8,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输出电压
    ui->deviceInfoTableWidget->setItem(9,1,new QTableWidgetItem(QString::number(mcp2221a->getVoutMin())));
    ui->deviceInfoTableWidget->item(9,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最小输出电压
    ui->deviceInfoTableWidget->setItem(10,1,new QTableWidgetItem(QString::number(mcp2221a->getVoutMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(10,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输出电流
    ui->deviceInfoTableWidget->setItem(11,1,new QTableWidgetItem(QString::number(mcp2221a->getIoutMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(11,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大输出功率
    ui->deviceInfoTableWidget->setItem(12,1,new QTableWidgetItem(QString::number(mcp2221a->getPoutMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(12,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最大环境温度
    ui->deviceInfoTableWidget->setItem(13,1,new QTableWidgetItem(QString::number(mcp2221a->getTambientMax(), 'f', 2)));
    ui->deviceInfoTableWidget->item(13,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新最小环境温度
    ui->deviceInfoTableWidget->setItem(14,1,new QTableWidgetItem(QString::number(mcp2221a->getTambientMin(), 'f', 2)));
    ui->deviceInfoTableWidget->item(14,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新90Vac效率
    ui->deviceInfoTableWidget->setItem(15,1,new QTableWidgetItem(QString::number(mcp2221a->getEfficncyLL(), 'f', 2)));
    ui->deviceInfoTableWidget->item(15,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    //更新264Vac效率
    ui->deviceInfoTableWidget->setItem(16,1,new QTableWidgetItem(QString::number(mcp2221a->getEfficncyHL(), 'f', 2)));
    ui->deviceInfoTableWidget->item(16,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

}

void MainWindow::setI2cSpeed(int speed)
{

    //    qDebug()<< speed << Qt::endl;

    mcp2221a->result = Mcp2221_SetSpeed(mcp2221a->mcpHandle, speed);
    if(mcp2221a->result == E_NO_ERR){
        ui->statePlainTextEdit->appendPlainText("设备频率设置成功，频率为：100KHz\t");
    }else{
        ui->statePlainTextEdit->appendPlainText("设备频率设置失败\n");
        pringfError(mcp2221a->result);
    }
}

void MainWindow::openButtonClicked()
{
    //获取设备地址
    bool ok;
    mcp2221a->SlaveAddress = ui->SlaveAddressLineEdit->text().toInt(&ok,16);
    mcp2221a->SlaveAddress = mcp2221a->SlaveAddress >> 1;  //
    mcp2221a->result = Mcp2221_GetLastError();//获取结果
    if(mcp2221a->result == E_NO_ERR){

        ui->statePlainTextEdit->appendPlainText("设备打开成功\t");

        //       qDebug()<< qStringToInt(ui->FrequencyComboBox->currentText()) << Qt::endl;
        setI2cSpeed(100*1000);
        workState = 1;

    }else{
        pringfError(mcp2221a->result);
    }
}

void MainWindow::pollPushButtonClicked()
{
    if(workState)
    {
        int time = qStringToInt(ui->pollComboBox->currentText());
        //    qDebug()<< time << Qt::endl;
        mcp2221a->MCP2221_While = 1;
        timer->start(time);
        ui->statePlainTextEdit->appendPlainText("开启轮询\t");
    }
    else
    {
        ui->statePlainTextEdit->appendPlainText("设备未连接\t");
    }
}

void MainWindow::stopPushButtonClicked()
{
    if(workState){
        mcp2221a->MCP2221_While = 0;
        timer->stop();
        ui->statePlainTextEdit->appendPlainText("关闭轮询\n");
    }else
    {
        ui->statePlainTextEdit->appendPlainText("设备未连接\t");
    }
}

void MainWindow::checkByteCountPushButtonClicked()
{
    bool ok;

    for(int i =1;i<40;i++)
    {
        mcp2221a->result = Mcp2221_SmbusBlockRead(mcp2221a->mcpHandle,
                                                  mcp2221a->SlaveAddress,
                                                  1, 0,
                                                  ui->BlockReadlineEdit->text().toInt(&ok,16),
                                                  i,
                                                  mcp2221a->I2C_Read_Data);
        if(mcp2221a->result == 0)
        {
            ui->BlockReadCountlineEdit->setText(QString::number(i,16).toUpper());
        }
    }

}

void MainWindow::timerTimeOut()
{
    if(mcp2221a->MCP2221_While)
    {
        mcp2221a->Read_Temp();
        mcp2221a->Read_Power();
        mcp2221a->Read_FanSpeed();

        //        ui->workDataTableWidget->setItem(1,1,new QTableWidgetItem(QString::number(mcp2221a->getVinI(), 'f', 2)));
        //        ui->workDataTableWidget->setItem(2,1,new QTableWidgetItem(QString::number(mcp2221a->getVinV(), 'f', 2)));
        ui->workDataTableWidget->setItem(3,1,new QTableWidgetItem(QString::number(mcp2221a->getOutputPower(), 'f', 2)));
        ui->workDataTableWidget->setItem(4,1,new QTableWidgetItem(QString::number(mcp2221a->getOutputCurrent(), 'f', 2)));
        ui->workDataTableWidget->setItem(5,1,new QTableWidgetItem(QString::number(mcp2221a->getOutputVoltage(), 'f', 2)));
        ui->workDataTableWidget->setItem(6,1,new QTableWidgetItem(QString::number(mcp2221a->getEnvirTemperature(), 'f', 2)));
        ui->workDataTableWidget->setItem(7,1,new QTableWidgetItem(QString::number(mcp2221a->getSRTemperature(), 'f', 2)));
        ui->workDataTableWidget->setItem(8,1,new QTableWidgetItem(QString::number(mcp2221a->getFFTemperature(), 'f', 2)));
        ui->workDataTableWidget->setItem(9,1,new QTableWidgetItem(QString::number(mcp2221a->getFanSpeed())));
        for (int  i= 3;  i< 10; i++) {
            ui->workDataTableWidget->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        }


    }
}

void MainWindow::timeUpdata()
{
    /* 设置 lcd 显示为当前系统时间 */
    QTime nowTime = QTime::currentTime();
    /* 设置显示的样式 */
    ui->timelcdNumber->display(nowTime.toString("hh:mm:ss"));

}


void MainWindow::on_radioButton_clicked()
{
    SMBusMode = 1;
}


void MainWindow::on_SMbusPushButton_clicked()
{
    if(workState)
    {
        bool ok;
        unsigned char commend;
        unsigned char byteCount;
        QString hexStr;

        QTime nowTime1 = QTime::currentTime();

        qDebug()<< SMBusMode << Qt::endl;
        switch (SMBusMode) {
        case 1:     //Send Byte

            commend = ui->SendByteLineEdit->text().toInt(&ok,16);
            errorCode = mcp2221a->SmbusSendByte(commend);
            if(errorCode){
                ui->statePlainTextEdit->appendPlainText("失败！失败代码：>>"+QString::number(errorCode,10));
            }

            break;
        case 2:     //Read Byte

            commend = ui->readByteLineEdit->text().toInt(&ok,16);
            errorCode = mcp2221a->SmbusReadByte(commend);
            if(errorCode){
                ui->statePlainTextEdit->appendPlainText("失败！失败代码：>> "+QString::number(errorCode,10));
            }else{
                hexStr = toHexStr(QByteArray((const char*)mcp2221a->I2C_Read_Data));
                ui->statePlainTextEdit->appendPlainText(nowTime1.toString("hh:mm:ss")+">>"+hexStr);
                mcp2221a->Clean_I2C_Read_Data();
            }

            break;
        case 3:     //Read Word

            commend = ui->readWordLineEdit->text().toInt(&ok,16);
            mcp2221a->SmbusReadWord(commend);
            hexStr = toHexStr(QByteArray((const char*)mcp2221a->I2C_Read_Data));
            ui->statePlainTextEdit->appendPlainText(nowTime1.toString("hh:mm:ss")+">>"+hexStr);
            mcp2221a->Clean_I2C_Read_Data();

            break;
        case 4:     //Block Read

            commend = ui->BlockReadlineEdit->text().toInt(&ok,16);
            byteCount = ui->BlockReadCountlineEdit->text().toInt(&ok,16);
            errorCode = mcp2221a->SmbusBlockRead(commend,byteCount);
            qDebug()<<errorCode;
            if( errorCode <= 0 ){
                ui->statePlainTextEdit->appendPlainText("失败！失败代码：>> "+QString::number(errorCode,10));
            }else{

                hexStr = toHexStr(QByteArray((const char*)mcp2221a->I2C_Read_Data));
                //                    QByteArray temp((const char*)(mcp2221a->I2C_Read_Data));
                //                    QString strSerial = QString::fromStdString(temp.toStdString());
                //                    ui->statePlainTextEdit->appendPlainText(nowTime1.toString("hh:mm:ss")+">>"+strSerial);
                ui->statePlainTextEdit->appendPlainText(nowTime1.toString("hh:mm:ss")+">>"
                                                        +"命令：0x" + ui->BlockReadlineEdit->text().toUpper()
                                                        +">>"
                                                        +"("+ui->BlockReadCountlineEdit->text().toUpper()+") "
                                                        +
                                                        hexStr);
                mcp2221a->Clean_I2C_Read_Data();

            }

            break;
        case 5:     //Block Write

            //            commend = ui->BlockReadlineEdit->text().toInt(&ok,16);
            //            writeData = ui->BlockWritedatalineEdit->text().toInt(&ok,16); //OX EA


            break;
        case 6:     //Write Byte

            break;
        case 7:     //Write  Word

            break;
        default:
            break;
        }
    }else{
        ui->statePlainTextEdit->appendPlainText("设备未连接\t");
    }
}


void MainWindow::on_radioButton_4_clicked()
{
    SMBusMode = 4;
}


void MainWindow::on_radioButton_3_clicked()
{
    SMBusMode = 3;
}


void MainWindow::on_radioButton_2_clicked()
{
    SMBusMode = 2;
}


void MainWindow::on_radioButton_5_clicked()
{
    SMBusMode = 5;
}


void MainWindow::on_radioButton_6_clicked()
{
    SMBusMode = 6;
}


void MainWindow::on_radioButton_7_clicked()
{
    SMBusMode = 7;
}


void MainWindow::cleanDataPushButton_clicked()
{
    for(int i = 0; i<17; i++)
    {
        ui->deviceInfoTableWidget->takeItem(i, 1);
    }
}


void MainWindow::updataPushButton_clicked()
{

    //    ui->deviceInfoTableWidget->setItem(1,1,new QTableWidgetItem(tr("MaxNum")));
    getDeviceInfo();
    qDebug()<<"ok";

}

