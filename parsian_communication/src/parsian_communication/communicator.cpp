#include <parsian_communication/communicator.h>
#include <string.h>
#include <stdio.h>
#include <parsian_util/tools/drawer.h>


/*************************************** CBaseCommunicator Class ******************************************/

CBaseCommunicator::CBaseCommunicator() {
    serial_open = false;
    error      = false;
    p = new CMySerialPort();
    p->serial_port = nullptr;
//    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF,40};
//    recTime = new QTimer;
//    recTime->setInterval(100);
//    recTime->start();
    recDataFlow.clear();
    for (auto &i : robotPacket) {
        for (unsigned char &j : i) {
            j = 0;
        }
    }

    for (auto &i : onlineRobotsTimer) {
        i.start();
    }
}


void CBaseCommunicator::readData() {

    QByteArray dataFlow;
    if (p->serial_port->bytesAvailable()) {
        dataFlow = p->serial_port->read(p->serial_port->bytesAvailable());
//        drawer->draw(Vector2D(1,0));
    }
    p->serial_port->flush();
    recDataFlow.append(dataFlow);
    if (recDataFlow.size() > 100) {
        recDataFlow.remove(0, recDataFlow.size() / 2);
    }

//    debug(QString("SIZE : %1").arg(recDataFlow.size()), D_MHMMD);
    if (recDataFlow.size()) {

        for (int i = 0 ; i < recDataFlow.size() ; i++) {
            if (recDataFlow[i] == static_cast<unsigned char>(0x99)) {

                if (i >= 12) {

                    for (int j = 0 ; j <= 12 ; j++) {
                        if (i - 12 < 12) {
                            robotPacket[recDataFlow[i - 12]][j] = static_cast<unsigned char>(recDataFlow[j + i - 12]);
                            onlineRobotsTimer[recDataFlow[i - 12]].restart();
                        }
                    }
                    if (!recDataFlow.isEmpty()) {
                        recDataFlow.remove(0, i + 1);
                        i = 0;
                    }

                } else {
                    if (!recDataFlow.isEmpty()) {
                        recDataFlow.remove(0, i + 1);
                        i = 0;
                    }
                }

            }
        }
    }



//    knowledge->onlineRobots.clear();
//    for(int i = 0 ; i < 12 ; i++)
//    {
//        knowledge->onlineRobots.append(i);
//        if(onlineRobotsTimer[i].elapsed() > 200)
//        {
//            knowledge->onlineRobots.removeOne(i);
//        }
//    }

//    debug(QString("id : %1").arg((int)robotPacket[4][1]), D_MHMMD);
//
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        robotStat[i].id                    = static_cast<unsigned char>(i);
        robotStat[i].shootSensor           = static_cast<unsigned char>( robotPacket[i][1] & 0x01);
        robotStat[i].battery               = static_cast<unsigned char>((robotPacket[i][1] & 0b11111100) >> 2);
        robotStat[i].capCharge             = static_cast<unsigned char>( robotPacket[i][2] & 0b00111111);
        robotStat[i].dataLoss              = static_cast<unsigned char>( robotPacket[i][4] & 0b00001111);
        robotStat[i].spinCatchBall         = static_cast<unsigned char>((robotPacket[i][1] & 0b00000010) >> 1);
        robotStat[i].shootBoardFault       = static_cast<unsigned char>((robotPacket[i][5] & 0b01000000) >> 6);
        robotStat[i].kickFault             = static_cast<unsigned char>((robotPacket[i][5] & 0b00100000) >> 5);
        robotStat[i].chipFault             = static_cast<unsigned char>((robotPacket[i][5] & 0b00010000) >> 4);
        robotStat[i].En1Fault              = static_cast<unsigned char>((robotPacket[i][5] & 0b00001000) >> 3);
        robotStat[i].En2Fault              = static_cast<unsigned char>((robotPacket[i][5] & 0b00000100) >> 2);
        robotStat[i].En3Fault              = static_cast<unsigned char>((robotPacket[i][5] & 0b00000010) >> 1);
        robotStat[i].En4Fault              = static_cast<unsigned char>( robotPacket[i][5] & 0b00000001);
        robotStat[i].m1Fault               = static_cast<unsigned char>( robotPacket[i][6] & 0b00000001);
        robotStat[i].m2Fault               = static_cast<unsigned char>((robotPacket[i][6] & 0b00000010) >> 1);
        robotStat[i].m3Fault               = static_cast<unsigned char>((robotPacket[i][6] & 0b00000100) >> 2);
        robotStat[i].m4Fault               = static_cast<unsigned char>((robotPacket[i][6] & 0b00001000) >> 3);
//        robotStat[i].motorFault[4]   = robotPacket[i][6] & 0b00010000;
        robotStat[i].boardId               = static_cast<unsigned char>(robotPacket[i][3] & 0b00011111);
    }
    robotsStat.reset(new parsian_msgs::parsian_robots_status);
    for (const auto &i : robotStat) {
        robotsStat->status.push_back(i);
    }
}



CBaseCommunicator::~CBaseCommunicator() {
    closeSerial();
    delete p;
}

void CBaseCommunicator::setSerialParams(unsigned int baud, unsigned int charsize, unsigned int parity, short stopbits) {
    p->setSerialParams(baud, charsize, parity, stopbits);
}


void CBaseCommunicator::connectSerial(const char* port) {
    closeSerial();
    _port = QString(port);
    p->setSerialParams(115200, 8, 0, 1);

    p->serial_port = new QextSerialPort(p->portSettings);
    p->serial_port->setPortName(_port);
    if (p->serial_port->open(QIODevice::ReadWrite)) {
        serial_open = true;
        ROS_INFO("successfully");
//        connect(recTime,SIGNAL(timeout()),this,SLOT(readData()));
    } else {
        ROS_ERROR("Error occured, comunicator connection failed");
        err = QString("Error occured, comunicator connection failed");
        serial_open = false;
        error = true;
        return;
    }
}

void CBaseCommunicator::closeSerial() {
    if (p->serial_port == nullptr) {
        return;
    }
    p->serial_port->close();
    delete p->serial_port;
    p->serial_port = nullptr;
    serial_open = false;
}

bool CBaseCommunicator::errorOccured() {
    return error;
}
bool CBaseCommunicator::isSerialConnected() {
    return serial_open;
}

///////////////////////////////////// communicator
CCommunicator::CCommunicator() : CBaseCommunicator() {
}

CCommunicator::~CCommunicator() = default;

void CCommunicator::packetCallBack(const parsian_msgs::parsian_packetsConstPtr &_packet) {
    char* tempStr;
    char test[100];
    for (const auto &robotPacket : _packet->value) {
        tempStr = new char[robotPacket.packets.size()];
        for (int j = 0 ; j < robotPacket.packets.size() ; j++) {
            tempStr[j] = robotPacket.packets.at(j);
        }
        sprintf(test, "packet :%lu", robotPacket.packets.size());
        ROS_INFO_STREAM(test);
        tempStr[0] = static_cast<char>(0x99);

        sendString(tempStr, static_cast<int>(robotPacket.packets.size()));

        delete[] tempStr;
    }
}

void CCommunicator::sendString(const char* s, int len) {

    for (int i = 0 ; i < len ; i++) {
        p->serial_port->write(s + i, 1);
    }
}

void CCommunicator::sendByte(char c) {
    char ch = c;
    sendString(&ch, 1);
}
