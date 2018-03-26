#include <parsian_msgs/parsian_packets.h>
#include <QtExtSerialPort/qextserialport.h>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QByteArray>
#include <string>
#include <ros/ros.h>
#include "parsian_util/base.h"
#include <parsian_msgs/parsian_robots_status.h>
#include "parsian_msgs/parsian_robot_status.h"
/*************************************** CMySerialPort Class ******************************************/

class CMySerialPort {
public:
    QextSerialPort *serial_port;
    PortSettings portSettings;

    CMySerialPort() {
        setSerialParams(0, 0, 0, 0); //setup default values
        serial_port = NULL;
    }

    void setSerialParams(unsigned int _baud, unsigned int _charsize, unsigned int _parity, short int _stopbits) {
        switch (_baud) {
        case 50:
            portSettings.BaudRate = BAUD50;
            break;
        case 75:
            portSettings.BaudRate = BAUD75;
            break;
        case 110:
            portSettings.BaudRate = BAUD110;
            break;
        case 134:
            portSettings.BaudRate = BAUD134;
            break;
        case 150:
            portSettings.BaudRate = BAUD150;
            break;
        case 200:
            portSettings.BaudRate = BAUD200;
            break;
        case 300:
            portSettings.BaudRate = BAUD300;
            break;
        case 600:
            portSettings.BaudRate = BAUD600;
            break;
        case 1200:
            portSettings.BaudRate = BAUD1200;
            break;
        case 1800:
            portSettings.BaudRate = BAUD1800;
            break;
        case 2400:
            portSettings.BaudRate = BAUD2400;
            break;
        case 4800:
            portSettings.BaudRate = BAUD4800;
            break;
        case 9600:
            portSettings.BaudRate = BAUD9600;
            break;
        case 19200:
            portSettings.BaudRate = BAUD19200;
            break;
        case 38400:
            portSettings.BaudRate = BAUD38400;
            break;
        case 57600:
            portSettings.BaudRate = BAUD57600;
            break;
        case 115200:
            portSettings.BaudRate = BAUD115200;
            break;
#ifndef Q_OS_MAC
        case 230400:
            portSettings.BaudRate = BAUD230400;
            break;
        case 460800:
            portSettings.BaudRate = BAUD460800;
            break;
        case 500000:
            portSettings.BaudRate = BAUD500000;
            break;
        case 576000:
            portSettings.BaudRate = BAUD576000;
            break;
        case 921600:
            portSettings.BaudRate = BAUD921600;
            break;
        case 1000000:
            portSettings.BaudRate = BAUD1000000;
            break;
        case 1152000:
            portSettings.BaudRate = BAUD1152000;
            break;
        case 1500000:
            portSettings.BaudRate = BAUD1500000;
            break;
        case 2000000:
            portSettings.BaudRate = BAUD2000000;
            break;
        case 2500000:
            portSettings.BaudRate = BAUD2500000;
            break;
        case 3000000:
            portSettings.BaudRate = BAUD3000000;
            break;
        case 3500000:
            portSettings.BaudRate = BAUD3500000;
            break;
        case 4000000:
            portSettings.BaudRate = BAUD4000000;
            break;
#endif
        default:
            portSettings.BaudRate = BAUD57600;
            break;
        }
        switch (_charsize) {
        case 5 :
            portSettings.DataBits = DATA_5;
            break;
        case 6 :
            portSettings.DataBits = DATA_6;
            break;
        case 7 :
            portSettings.DataBits = DATA_7;
            break;
        case 8 :
            portSettings.DataBits = DATA_8;
            break;
        default:
            portSettings.DataBits = DATA_8;
            break;
        }
        switch (_parity) {
        case 0 :
            portSettings.Parity = PAR_NONE;
            break;
        case 1 :
            portSettings.Parity = PAR_ODD;
            break;
        case 2 :
            portSettings.Parity = PAR_EVEN;
            break;
        default:
            portSettings.Parity = PAR_NONE;
            break;
        }
        // ********* check this part **********//
        switch (_stopbits) {
        case 1:
            portSettings.StopBits = STOP_1;
            break;
#if defined(Q_OS_WIN) || defined(qdoc)
        case 33:
            portSettings.StopBits = STOP_1_5;
            break;
#endif
        case 2:
            portSettings.StopBits = STOP_2;
            break;
        default:
            portSettings.StopBits = STOP_1;
            break;
        }
        portSettings.FlowControl = FLOW_OFF;
        portSettings.Timeout_Millisec = 40;
    }
};

/*************************************** CBaseCommunicator Class ******************************************/
class CBaseCommunicator {


public:
    CBaseCommunicator();
    ~CBaseCommunicator();
    //////public serial funcs
    void setSerialParams(unsigned int baud, unsigned int charsize, unsigned int parity, short int stopbits);
    void connectSerial(const char* port);
//    void activateSerial();
//    void deactivateSerial();
    void closeSerial();
    bool isSerialConnected();
    bool errorOccured();
//    QString getError();
//    QString getSerialPort();
protected:
    CMySerialPort* p;
private:
    QString _port;
    QString err;
    bool error;
    bool serial_open;
public :
    void readData();
    parsian_msgs::parsian_robots_statusPtr robotsStat;
private:
    char a;
    QTimer *recTime;
    QTime onlineRobotsTimer[12];
    QByteArray recDataFlow;
    unsigned char rPack[13];
    unsigned char robotPacket[16][13];
    parsian_msgs::parsian_robot_status robotStat[_MAX_NUM_PLAYERS];


};
/*************************************** communicator class ******************************************/

class CCommunicator: public CBaseCommunicator {
public:
    void packetCallBack(const parsian_msgs::parsian_packetsConstPtr& _packet);
    CCommunicator();
    ~CCommunicator();
    //////public serial funcs

    void sendString(const char* s, int len);
    void sendByte(char c);

private:

};
