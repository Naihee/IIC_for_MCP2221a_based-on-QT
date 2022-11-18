#ifndef MCP2221A_H
#define MCP2221A_H
#include <QApplication>
#include <windows.h>
#define        MCP2221_LIB        1	//for projects importing the .lib, use the MCP2221_LIB preprocessor definition
#include "mcp2221_dll_um.h"
#include <QDebug>
#include <iostream>
#include <QMainWindow>
#define        DEFAULT_VID           0x04D8  //默认设备VID
#define        DEFAULT_PID           0x00DD  //默认设备PID
#define        MAX_NUMBER_OF_CYCLES  10      //默认允许失败次数

//void*          mcpHandle = nullptr ;     //函数中间操作数，返回操作句柄




class MCP2221A : public QMainWindow
{
    Q_OBJECT
public:
    explicit MCP2221A(QWidget *parent = nullptr);

    wchar_t         libVersion[64];            //函数中间操作数，获取版本号
    unsigned int    numberOfDevices = 0;       //函数中间操作数，获取连接设备数量
    unsigned int    mcpIndex = 0;              //函数中间操作数，当前句柄
    short           result = 0;                //函数中间操作数，返回每次操作的结果
    unsigned int            DIC = 0;
    unsigned char   SlaveAddress = 0x58;   //当前试验地址： 101 1000(B0)=88
    unsigned char   I2C_Read_Data[25];     //接收数据数组
    unsigned short  Linear_Data;           //Linear格式数据
    bool            MCP2221_Status = 0;    //BOOL变量：=0代表设备未打开，=1代表设备已打开
    bool            MCP2221_While = 0;     //轮询变量
    int             I2C_speed = 100000;


public:
    float           Linear11_To_Float(unsigned short );
    unsigned short  Float_To_Linear11(float );
    float           Linear16_To_Float(unsigned short );
    unsigned short  Float_To_Linear16(float );
    unsigned char   AsciiToSixteen(char );
    void            Read_Temp();                          //读取设备温度
    void            Read_Power();                         //读取输出功率
    void            Read_FanSpeed();                      //读取风扇转速
    void            Clean_I2C_Read_Data();

    int            SmbusSendByte(unsigned char);
    int            SmbusReadByte(unsigned char command);         // 返回值在I2C_Read_Data中
    int            SmbusWriteByte(unsigned char command,unsigned char data);
    int            SmbusWriteWord(unsigned char command,unsigned char data[]);        // 返回值在I2C_Read_Data中
    int            SmbusReadWord(unsigned char command);
    int            SmbusBlockWrite( unsigned char command, unsigned char byteCount,unsigned char data[]);
    int            SmbusBlockRead( unsigned char command, unsigned char byteCount);



    float           getEnvirTemperature();
    float           getSRTemperature();
    float           getFFTemperature();
    float           getOutputVoltage();
    float           getOutputCurrent();
    float           getOutputPower();
    int             getFanSpeed();
    float           getVinMin();
    float           getVinMax();
    float           getIinMax();
    float           getPinMax();
    float           getVoutMin();
    float           getVoutMax();
    float           getIoutMax();
    float           getPoutMax();
    float           getTambientMax();
    float           getTambientMin();
    float           getEfficncyLL();
    float           getEfficncyHL();
    float           getVinI();
    float           getVinV();

    void*  mcpHandle = nullptr ;

private:

    float envirTemperature;
    float SRTemperature;
    float FFTemperature;
    float outputVoltage;
    float outputCurrent;
    float outputPower;
    float vinMin;
    float vinMax;
    float iinMax;
    float pinMax;
    float voutMin;
    float voutMax;
    float ioutMax;
    float poutMax;
    float tambientMax;
    float tambientMin;
    float efficncyLL;
    float efficncyHL;
    float vinI;
    float vinV;

    int   fanSpeed;


signals:

};

#endif // MCP2221A_H
