#include "mcp2221a.h"




MCP2221A::MCP2221A(QWidget *parent)
    : QMainWindow{parent}
{

}

float MCP2221A::Linear11_To_Float(unsigned short LinearData)
{
    unsigned short  TransData;
    unsigned char   TransI;
    float  FloatData;

    TransData = LinearData & 0x7FF;//低0-10位：Y
    if (TransData > 1023)  FloatData = (float)TransData - 2048;
    else                FloatData = (float)TransData;

    TransData = LinearData >> 11;//高11-15位：N
    if (TransData > 15)
    {
        TransData = 32 - TransData;
        for (TransI = 0; TransI < TransData; TransI++)
        {
            FloatData = FloatData / 2;
        }
    }
    else
    {
        for (TransI = 0; TransI < TransData; TransI++)
        {
            FloatData = FloatData * 2;
        }
    }

    return (FloatData);

}

unsigned short MCP2221A::Float_To_Linear11(float FloatData)
{
    float           TransData;
    unsigned short  TransIndex = 0;
    unsigned char   TransI = 0;
    unsigned char   TransSign = 0;
    unsigned short  LinearData = 0;

    if (FloatData < 0)
    {
        TransData = 0 - FloatData;
        TransSign = 1;
    }
    else
    {
        TransData = FloatData;
    }
    for (TransI = 0; TransI < 15; TransI++)
    {
        if (TransData > 1023)
        {
            TransData = TransData / 2;
            TransIndex++;
        }
        else if (TransData < 512)
        {
            TransData = TransData * 2;
            TransIndex--;
        }
        else
        {
            TransI = 15;
        }
    }
    if (TransData < 1)
    {
        LinearData = 0;
    }
    else
    {
        if (TransData > 1023)
        {
            TransData = 1023;
        }
        if (TransSign)
        {
            TransData = 1024 - TransData;
            LinearData = (unsigned short)TransData;
            LinearData |= 0x0400;
        }
        else
        {
            LinearData = (unsigned short)TransData;
        }
        TransIndex = TransIndex << 11;
        LinearData |= TransIndex;
    }

    return (LinearData);

}

float MCP2221A::Linear16_To_Float(unsigned short LinearData)
{
    unsigned char  TransMode;
    unsigned char  TransI;
    float          FloatData;

    TransMode = 0x17 & 0x1F;

    FloatData = (float)LinearData;

    if (TransMode > 15)
    {
        TransMode = 32 - TransMode;
        for (TransI = 0; TransI < TransMode; TransI++)
        {
            FloatData = FloatData / 2;
        }
    }
    else
    {
        for (TransI = 0; TransI < TransMode; TransI++)
        {
            FloatData = FloatData * 2;
        }
    }
    return (FloatData);

}

unsigned short MCP2221A::Float_To_Linear16(float FloatData)
{
    unsigned char   TransMode;
    unsigned char   TransI;
    float           TransData;
    unsigned short  LinearData;

    TransMode = 0x17 & 0x1F;
    TransData = FloatData;

    if (TransMode > 15)
    {
        TransMode = 32 - TransMode;
        for (TransI = 0; TransI < TransMode; TransI++)
        {
            TransData = TransData * 2;
        }
    }
    else
    {
        for (TransI = 0; TransI < TransMode; TransI++)
        {
            TransData = TransData / 2;
        }
    }
    if (TransData > 0xFFFF)
    {
        LinearData = 0xFFFF;
    }
    else
    {
        LinearData = (unsigned short)TransData;
    }
    return (LinearData);
}

unsigned char MCP2221A::AsciiToSixteen(char Dat)
{
    switch(Dat)
    {
    case '0': { return 0; }		break;
    case '1': { return 1; }		break;
    case '2': { return 2; }		break;
    case '3': { return 3; }		break;
    case '4': { return 4; }		break;
    case '5': { return 5; }		break;
    case '6': { return 6; }		break;
    case '7': { return 7; }		break;
    case '8': { return 8; }		break;
    case '9': { return 9; }		break;
    case 'A': { return 10; }	break;
    case 'a': { return 10; }	break;
    case 'B': { return 11; }	break;
    case 'b': { return 11; }	break;
    case 'C': { return 12; }	break;
    case 'c': { return 11; }	break;
    case 'D': { return 13; }	break;
    case 'd': { return 11; }	break;
    case 'E': { return 14; }	break;
    case 'e': { return 11; }	break;
    case 'F': { return 15; }	break;
    case 'f': { return 11; }	break;
    default:  { return 99; }	break;
    }
}

void MCP2221A::Read_Temp()
{
    //获取环境温度
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x8D, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        envirTemperature = Linear11_To_Float(Linear_Data);
    }
    //获取SR温度
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x8E, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        SRTemperature = Linear11_To_Float(Linear_Data);
    }
    //获取防反温度
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x8F, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        FFTemperature = Linear11_To_Float(Linear_Data);
    }
}

void MCP2221A::Read_Power()
{
    //PMBUS读字（两个字节共16位）：句柄、从机地址、7位地址、不使用PEC校验，命令0x8B(输出电压)、接收数据数组
    //获取输出电压
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x8B, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        outputVoltage = Linear16_To_Float(Linear_Data);
    }
    //获取输出电流
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x8C, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        outputCurrent = Linear11_To_Float(Linear_Data);
    }
    //获取输出功率
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x96, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        outputPower = Linear11_To_Float(Linear_Data);
    }

}

void MCP2221A::Read_FanSpeed()
{
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0x90, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        fanSpeed = Linear11_To_Float(Linear_Data);
    }
}

void MCP2221A::Clean_I2C_Read_Data()
{
    memset(I2C_Read_Data,'\0',sizeof(I2C_Read_Data));
}

int MCP2221A::SmbusSendByte(unsigned char data)
{

    result = Mcp2221_SmbusSendByte(mcpHandle, SlaveAddress, 1, 0, data);
    return result;
}

int MCP2221A::SmbusReadByte(unsigned char command)
{

    result = Mcp2221_SmbusReadByte(mcpHandle, SlaveAddress, 1, 0, command, I2C_Read_Data);
    return result;
}

int MCP2221A::SmbusWriteByte(unsigned char command, unsigned char data)
{
    result = Mcp2221_SmbusWriteByte(mcpHandle, SlaveAddress, 1, 0, command,data);
    return result;
}

int MCP2221A::SmbusWriteWord(unsigned char command,unsigned char data[])
{


    result = Mcp2221_SmbusWriteWord(mcpHandle, SlaveAddress, 1, 0, command,data);
    return result;
}

int MCP2221A::SmbusReadWord(unsigned char command)
{
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, command, I2C_Read_Data);
    if(result !=0)
    {
        Clean_I2C_Read_Data();

    }
    return result;

}

int MCP2221A::SmbusBlockWrite(unsigned char command, unsigned char byteCount, unsigned char data[])
{
    result = Mcp2221_SmbusBlockWrite(mcpHandle, SlaveAddress, 1, 0,command,byteCount,data);
    return result;
}

int MCP2221A::SmbusBlockRead(unsigned char command, unsigned char byteCount)
{

//    result = Mcp2221_SmbusBlockRead(mcpHandle, SlaveAddress, 1, 0,command,byteCount,I2C_Read_Data);
    unsigned char *com = &command;
    unsigned char num = 0;
    result = Mcp2221_I2cWrite(mcpHandle,1,SlaveAddress,1,com);
    if(result == 0)
    {
        result = Mcp2221_I2cRead(mcpHandle,byteCount+1,SlaveAddress,1,I2C_Read_Data);
        if(result == 0)
        {
            num = I2C_Read_Data[0];
            for(int i=0 ; i < sizeof(I2C_Read_Data) ;i++)
            {
                I2C_Read_Data[i] = I2C_Read_Data[i+1];
            }
        }
    }



    return (result==0)?(num):(result);
}

float MCP2221A::getEnvirTemperature()
{
    return envirTemperature;
}

float MCP2221A::getSRTemperature()
{
    return SRTemperature;
}

float MCP2221A::getFFTemperature()
{
    return FFTemperature;
}

float MCP2221A::getOutputVoltage()
{
    return outputVoltage;
}

float MCP2221A::getOutputCurrent()
{
    return outputCurrent;
}

float MCP2221A::getOutputPower()
{
    return outputPower;
}

int MCP2221A::getFanSpeed()
{
    return fanSpeed;
}

float MCP2221A::getVinMin()
{
    result = Mcp2221_SmbusReadWord(mcpHandle, SlaveAddress, 1, 0, 0xA0, I2C_Read_Data);
    if (result == 0)
    {
        Linear_Data = (I2C_Read_Data[1] << 8) + I2C_Read_Data[0];
        vinMin = Linear11_To_Float(Linear_Data);
    }
    return vinMin;
}

float MCP2221A::getVinMax()
{
    result = SmbusReadWord(0xA1);
    if (result == 0)
    {
        vinMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return vinMax;

}

float MCP2221A::getIinMax()
{
    result = SmbusReadWord(0xA2);
    if (result == 0)
    {
        iinMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return iinMax;
}

float MCP2221A::getPinMax()
{
    result = SmbusReadWord(0xA3);
    if (result == 0)
    {
        pinMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return pinMax;
}

float MCP2221A::getVoutMin()
{
    result = SmbusReadWord(0xA4);
    if (result == 0)
    {
        voutMin = Linear16_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return voutMin;
}

float MCP2221A::getVoutMax()
{
    result = SmbusReadWord(0xA5);
    if (result == 0)
    {
        voutMax = Linear16_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return voutMax;
}

float MCP2221A::getIoutMax()
{
    result = SmbusReadWord(0xA6);
    if (result == 0)
    {
        ioutMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return ioutMax;
}

float MCP2221A::getPoutMax()
{
    result = SmbusReadWord(0xA7);
    if (result == 0)
    {
        poutMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return poutMax;
}

float MCP2221A::getTambientMax()
{
    result = SmbusReadWord(0xA8);
    if (result == 0)
    {
        tambientMax = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return tambientMax;
}

float MCP2221A::getTambientMin()
{
    result = SmbusReadWord(0xA9);
    if (result == 0)
    {
        tambientMin = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return tambientMin;
}

float MCP2221A::getEfficncyLL()
{
    result = SmbusReadWord(0xAA);
    if (result == 0)
    {
        efficncyLL = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return efficncyLL;
}
float MCP2221A::getEfficncyHL()
{
    result = SmbusReadWord(0xAB);
    if (result == 0)
    {
        efficncyHL = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return efficncyHL;
}

float MCP2221A::getVinI()
{
    result = SmbusReadWord(0x89);
    if (result == 0)
    {
        vinI = Linear11_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return vinI;
}

float MCP2221A::getVinV()
{
    result = SmbusReadWord(0x88);
    if (result == 0)
    {
        vinV = Linear16_To_Float((I2C_Read_Data[1] << 8) + I2C_Read_Data[0]);
    }
    return vinV;

}

