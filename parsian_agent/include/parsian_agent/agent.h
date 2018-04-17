#ifndef CAGENT_H
#define CAGENT_H

#include <parsian_util/base.h>
#include <planner/planner.h>
#include <parsian_util/geom/geom.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_util/matrix.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_get_plan.h>
#include <parsian_msgs/parsian_path.h>
#include <QDebug>
#include <QFile>
#include <fstream>
#include <QTime>
#include <vector>
#include <string>
#include <ros/package.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
using namespace std;

#define MAX_KICK_SPEED 1023
#define new_com_test_robot_id (-1)

struct Fault {

};


class CSkill;
class Agent {
public:
    class Abilities {
    public:
        Abilities();
        bool canChip;
        bool canKick;
        bool canSpin;
        bool highBattery;
        bool hasGyro;
    } abilities;

    class Status {
    public:
        Status();
        double battery;
        double capCharge; // the charge of shot capacities
        double dataLost;
        bool spin;
        bool shotSensor;
        bool fault;
        bool faild;
        bool halt;
        bool shotBoard;
        bool kickFault;
        bool chipFault;
        bool encoderFault[4];
        bool motorFault[5];
        bool beep;
        bool shotSensorFault;
        int boardID;

    } status;

    CSkill *skill;
    QString skillName;
    bool changeIsNeeded;

    Vector2D homePos;
    void accelerationLimiter(double vf, bool diveMode = false);
    double goalVisibility;
    QTime agentStopTime;
    bool timerReset;

    explicit Agent(int _ID);
    bool startTrain;
    bool stopTrain;
    double wh1, wh2, wh3, wh4;
    bool starter;
    bool canRecvPass;
    bool idle;
    int id();
    int commandId();
    bool trajectory(double& vf, double& vn, double& va, double w1, double w2, double w3, double w4, bool &stop);
    void loadProfiles();
    double getVisibility();
    void setVisibility(const double &inSight);
    bool isVisible();
    bool notVisible();


    void setOnOffState(bool state);
    void setCommandID(int ID);

    bool shootSensor();
    void setShootSensor(bool b);
    bool canOneTouch();

    //position and velocity
    Vector2D pos();
    Vector2D dir();
    double dirDegree();
    Vector2D vel();
    double angularVel();
    CRobot* self();
    Vector2D distToBall();

    //Low Level Commands
    void setRobotVel(double _vtan, double _vnorm, double _w); //vtan, vnorm in m/s and w in rad/s
    void setRobotAbsVel(double _vx, double _vy, double _w);
    void addRobotVel(double _vtan, double _vnorm, double _w);

    void setKick(double _kickSpeed);
    void setChip(double _kickSpeed);
    void setBeep(bool _beep);
    void setForceKick(bool _force);
    void setRoller(int state);
    char getRoller();
    void waitHere();

    double v1L, v2L, v3L, v4L;
    double vforwardL, vnormalL, vangularL;

    QString localName, prevLocalName;

    bool kickState, chipState, onOffState , inOutState;
    int commandID;
    float getMotorMaxRadPerSec();
    float getvLimit();
    float getwLimit();
    double v1, v2, v3, v4;
    int roller;
    double kickSpeed;

    int sumEX, sumEY, lastEX, lastEY;

    double shotProfile[32][2];
    double chipProfile[32][2];

    double getKickValue(bool chip, bool spin, double v);
    double kickValueSpeed(double value,bool spinner);
    double kickSpeedValue(double speed, int spinner);
    double chipValueDistance(double value, bool spinner);
    double chipDistanceValue(double distance,int spinner);

    int kickValueForDistance(double dist, double finalVel);

    bool requestBit , chip, forceKick, forceKickCounter, lowSpeedMode;
    bool beep;
    double vforward, vnormal, vangular;
    double _ACC, _DEC;
    Vector2D oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDir);
    vector<Vector2D> pathPlannerResult;
    Vector2D plannerAverageDir;

    void setGyroZero();
    Vector2D agentAngelForGyro;
    int calibrated;
    void jacobian(double _vx, double _vy, double _w, double &v1, double &v2, double &v3, double &v4);

private:
    void jacobianInverse(double _v1, double _v2, double _v3, double _v4, double &_vx, double &_vy, double &_w);
    bool calibrateGyro;
    unsigned int packetNum;
    double lastVf, lastVn;
    short int selfID;
    const double Gravity = 9.8;
    double getVar(const double* data);
    Matrix ANN_forward( Matrix input );
    //kick profiler usage
    void getkickprofilerdata();
    bool gotkickprofilerdatas;
    float convertkickspeedtokickchargetime(float kickspeed, int spin);
    float kick_coef_a[8], kick_coef_b[8], kick_coef_c[8];
    //chip profiler usage
    void getchipprofilerdata();
    bool gotchipprofilerdatas;
    float convertchipdisttochipchargetime(float chipdist, int spin);
    float chip_coef_a[8], chip_coef_b[8], chip_coef_c[8];

public:

    ros::Publisher planner_pub;
    void initPlanner(const Vector2D &_target, const QList<int> &_ourRelaxList,
                     const QList<int> &_oppRelaxList, const bool &_avoidPenaltyArea, const bool &_avoidCenterCircle,
                     const double &_ballObstacleRadius);
    void getPathPlannerResult(vector<Vector2D> _result , Vector2D _averageDir);

    const double gain = 1.013;
    void execute();
    parsian_msgs::parsian_robot_commandPtr getCommand();
};

#endif // CAGENT_H
