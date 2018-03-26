#include <parsian_agent/agent.h>
#include <parsian_agent/skills.h>
#include <parsian_agent/config.h>

#define errlen 100


double Agent::getVar(const double *data) {
    double mean = 0.0;
    for (int i = 0 ; i < errlen ; i++) {
        mean += data[i];
    }
    mean /= errlen;

    double var = 0.0;
    double error;
    for (int i = 0 ; i < errlen ; i++) {
        error = data[i] - mean;
        var += error * error;
    }
    var /= errlen;
    var = sqrt(var);
    return var;
}

Matrix tansig(const Matrix &n) {
    for (int i = 0 ; i < n.nrows() ; i++)
        for (int j = 0 ; j < n.ncols() ; j++) {
            n.e(i , j) = 2.0 / (1.0 + exp(-2.0 * n.e(i , j))) - 1.0;
        }
    return n;
}

Matrix Agent::ANN_forward(Matrix input) {
    static Matrix b1(10, 1), b2(4, 1), IW(10, 3), LW(4, 10);
    static bool run = true;
    if (run) {
        fstream file1, file2, file3, file4;
        file1.open("test/b1.txt", fstream::in);
        file2.open("test/b2.txt", fstream::in);
        file3.open("test/IW.txt", fstream::in);
        file4.open("test/LW.txt", fstream::in);
        for (int i = 0; i < 10 ; i++) {
            file1 >> b1.e(i, 0);
        }
        for (int i = 0; i < 4 ; i++) {
            file2 >> b2.e(i, 0);
        }
        for (int i = 0; i < 10 ; i++) {
            for (int j = 0; j < 3 ; j++) {
                file3 >> IW.e(i, j);
            }
        }
        for (int i = 0; i < 4 ; i++) {
            for (int j = 0; j < 10 ; j++) {
                file4 >> LW.e(i, j);
            }
        }
        run = false;
    }

    Matrix output(4, 1);
    Matrix xmin(3, 1);
    xmin.e(0, 0) = -0.561029;
    xmin.e(1, 0) = -0.533322;
    xmin.e(2, 0) = -57.9859;
    Matrix xmax(3, 1);
    xmax.e(0, 0) = 0.57;
    xmax.e(1, 0) = 0.5344;
    xmax.e(2, 0) = 56.928;

    input = input - xmin;
    for (int i = 0; i < input.nrows() ; i++)
        for (int j = 0; j < input.ncols() ; j++) {
            input.e(i, j) *= (2.0) / (xmax.e(i, j) - xmin.e(i, j));
            input.e(i, j) -= 1.0;
        }

    output = LW * tansig(IW * input + b1) + b2;

    Matrix Xmin(4, 1);
    Xmin.e(0, 0) = -27.3596;
    Xmin.e(1, 0) = -28.2358;
    Xmin.e(2, 0) = -28.2358;
    Xmin.e(3, 0) = -27.3596;
    Matrix Xmax(4, 1);
    Xmax.e(0, 0) = 27.3596;
    Xmax.e(1, 0) = 28.2358;
    Xmax.e(2, 0) = 28.2358;
    Xmax.e(3, 0) = 27.3596;

    for (int i = 0; i < output.nrows() ; i++)
        for (int j = 0; j < output.ncols() ; j++) {
            output.e(i, j) -= -1;
            output.e(i, j) *= (Xmax.e(i, j) - Xmin.e(i, j)) / (2.0);
        }

    output = output + Xmin;
    return output;
}

Agent::Agent(int _ID) {
    srand48(time(nullptr));
    packetNum = 0;
    stopTrain = false;
    wh1 = wh2 = wh3 = wh4 = 0.0;
    startTrain = false;
    selfID = static_cast<short>(_ID);
    skill = nullptr;
    skillName = "";
    onOffState = true;
    commandID = selfID;

    chip = false;
    forceKick = false;
    canRecvPass = false;
    roller = 0;
    kickSpeed = 0;

    v1 = 0;
    v2 = 0;
    v3 = 0;
    v4 = 0;
    vforward  = 0;
    vnormal   = 0;
    vangular  = 0;
    lastVf = 0;
    lastVn = 0;
    lowSpeedMode = false;
    starter = false;
    beep = false;

    sumEX = sumEY = 0;
    lastEX = lastEY = 0;
    goalVisibility = -1;
    idle = false;

    //Both codes below do the same work!

    loadProfiles();

    agentAngelForGyro = Vector2D(1, 0);
    calibrateGyro = false;
    calibrated = 900;
    this->abilities.canChip = true;
    this->abilities.canKick = true;
    this->abilities.canSpin = true;
    this->abilities.hasGyro = false;

    homePos.assign(0 , 0);
    _ACC = 0;
    _DEC = 0;
    agentStopTime.start();

    changeIsNeeded = false;

}

void Agent::loadProfiles() {
    //Set shotProfile From File
    {
        for (auto &i : shotProfile) {
            i[0] = i[1] = -1.0;
        }
        QFile file(QString("profiles/robot%1.shot").arg(selfID));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            //            qDebug()<<(QString("Parsing profiles/robot%1.shot").arg(selfID));

            while (!file.atEnd()) {
                QByteArray line = file.readLine();

                QStringList l = QString(line).split(',');
                int index = l[0].toInt();
                shotProfile[index][0] = l[1].toDouble();
                shotProfile[index][1] = l[2].toDouble();
            }

            //            qDebug()<<"shot profile for robot "<<selfID<<":";
            for (int i = 0; i < 32; i++) {
                qDebug() << i << " " << shotProfile[i][0] << " " << shotProfile[i][1];
            }
            file.close();
        } else {
            //            qDebug()<<(QString("NotFound: profiles/robot%1.shot").arg(selfID))<<file.errorString();
        }
    }

    //Set chipProfile From File
    {
        for (auto &i : chipProfile) {
            i[0] = i[1] = -1.0;
        }
        QFile file(QString("profiles/robot%1.chip").arg(selfID));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            //            qDebug()<<(QString("Parsing profiles/robot%1.chip").arg(selfID));

            while (!file.atEnd()) {
                QByteArray line = file.readLine();

                QStringList l = QString(line).split(',');
                int index = l[0].toInt();
                chipProfile[index][0] = l[1].toDouble();
                chipProfile[index][1] = l[2].toDouble();
            }


            //            qDebug()<<"chip profile for robot "<<selfID<<":";
            for (int i = 0; i < 32; i++) {
                qDebug() << i << " " << chipProfile[i][0] << " " << chipProfile[i][1];
            }
            file.close();
        } else {
            //            qDebug()<<(QString("NotFound: profiles/robot%1.chip").arg(selfID))<<file.errorString();
        }
    }
}

int Agent::id() {
    return selfID;
}

double Agent::getVisibility() {
    return self()->inSight;
}

void Agent::setVisibility(const double &inSight) {
    self()->inSight = inSight;
}

bool Agent::isVisible() {
    return self()->inSight > 0;
}

bool Agent::notVisible() {
    return self()->inSight <= 0;
}

int Agent::commandId() {
    return commandID;
}

//RPM story
/// velocity type 1500
/// default mode  1300

static double decay_x = 0.2;
static double decay_y = 0.2;
static double decay_w = 20;
static double lastVelx[errlen];
static double lastVely[errlen];
static double lastOmega[errlen];
const int decay_accel = 10;
static int counter = 0;
//static double bvf=0.0,bvn=0.0,bva=0.0;
//static Matrix Epsilon(12,1);

bool Agent::trajectory(double& vf, double& vn, double& va, double w1, double w2, double w3, double w4, bool &stop) {
    static int error_set = true;
    if (error_set) {
        error_set = false;
        for (int i = 0 ; i < errlen ; i++) {
            lastVelx[i] = lastVely[i] = lastOmega[i] = 0;
        }
        counter = 0;
    }

    double cvx = getVar(lastVelx);
    double cvy = getVar(lastVely);
    double cvw = getVar(lastOmega);
    double thrx = 0.10;//boundTo( 78.0 + 74.0 * Vector2D(vf,vn).length() , 130.0 , 400.0 ) * (errlen/100);
    double thry = 0.10;
    double thrw = 10.0;


    if (cvx < thrx && cvy < thry && cvw < thrw && counter == errlen + 10) {
        QFile file1("./test/wheels.txt");
        QFile file2("./test/observed.txt");
        QFile file3("./test/input.txt");
        file1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        file2.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        file3.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        QTextStream ts1(&file1);
        QTextStream ts2(&file2);
        QTextStream ts3(&file3);

        double mVelx = 0;
        double mVely = 0;
        double mOmega = 0;
        for (int i = 0 ; i < errlen ; i++) {
            mVelx += lastVelx[i];
            mVely += lastVely[i];
            mOmega += lastOmega[i];
        }
        mVelx /= errlen;
        mVely /= errlen;
        mOmega /= errlen;
        //todo
        // ts1 << w1 << " " << w2 << " " << w3 << " " << w4 << endl;
        // ts2 << mVelx << " " << mVely << " " << mOmega << endl;
        // ts3 << vf << " " << vn << " " << va << endl;

        file1.close();
        file2.close();
        file3.close();


        for (int i = 0 ; i < errlen ; i++) {
            lastVelx[i] = lastVely[i] = lastOmega[i] = 0;
        }

        //    vf = (((double)(qrand()%201))/50.0)-2.0;
        //    vn = (((double)(qrand()%201))/50.0)-2.0;
        vf = (((double)(qrand() % 201)) / 100.0) - 1.0;
        vn = (((double)(qrand() % 201)) / 100.0) - 1.0;
        double speed = Vector2D(vf, vn).r();
        if (speed < 0.5) {
            speed = 0.5;
        }

        decay_x = fabs(vf) / decay_accel;
        decay_y = fabs(vn) / decay_accel;
        decay_w = fabs(va) / decay_accel;
        stop = true;
        counter = 0;
    } else {
        for (int counter = errlen - 2 ; counter > -1 ; counter--) {
            lastVelx[counter + 1] = lastVelx[counter];
            lastVely[counter + 1] = lastVely[counter];
            lastOmega[counter + 1] = lastOmega[counter];
        }
        double angle = wm->our[new_com_test_robot_id]->dir.th().radian();
        lastVelx[0] = (cos(-angle) * wm->our[new_com_test_robot_id]->vel.x) - (sin(-angle) * wm->our[new_com_test_robot_id]->vel.y);
        lastVely[0] = (sin(-angle) * wm->our[new_com_test_robot_id]->vel.x) + (cos(-angle) * wm->our[new_com_test_robot_id]->vel.y);
        lastOmega[0] = wm->our[new_com_test_robot_id]->angularVel;
        if (counter < errlen + 10) {
            counter++;
        }
    }
    return false;
}

void Agent::accelerationLimiter(double vf, bool diveMode) {
    ////first Stage Accelerate Limit
    double veltan = (vel().x) * cos(dir().th().radian()) + (vel().y) * sin(dir().th().radian());
    double velnorm = -1 * (vel().x) * sin(dir().th().radian()) + (vel().y) * cos(dir().th().radian());


    //    if(fabs(veltan - lastVf) > 3)
    //    {
    //        lastVf = veltan;
    //    }
    //    if(fabs(velnorm - lastVn) > 3)
    //    {
    //        lastVn = velnorm;
    //    }

    /* if(vel().length() > 0.2)
     {
         agentStopTime.restart();
         timerReset = false;
     }
     if(agentStopTime.elapsed() > 100 && ! timerReset)
     {
         lastVn = velnorm;
         lastVf = veltan;
         agentStopTime.restart();
         timerReset = true;
     }
    */
    double lastV, commandV;
    double vCoef = 1;
    double tempVf = vforward , tempVn = vnormal;
    double decCoef = 1.5;

    if (vf == 0) {
        decCoef = 2.5;
    }
    ///////////////////////first Stage Acc with true angle

    double accCoef = 1, realAcc = 4;
    accCoef = atan(fabs(vforward) / fabs(vnormal)) / _PI * 2;
    if (diveMode) {
        realAcc = 1.5 * accCoef * conf->AccMaxForward + (1 - accCoef) * conf->AccMaxNormal;
    } else {
        realAcc = accCoef * conf->AccMaxForward + (1 - accCoef) * conf->AccMaxNormal;

    }


    commandV = sqrt((vforward * vforward) + (vnormal * vnormal));
    lastV = sqrt((lastVf * lastVf) + (lastVn * lastVn));

    if (commandV > (lastV + realAcc * 0.0166667)) {
        commandV = lastV + (realAcc * 0.0166667);
    }
    vforward = commandV * sin(atan2(tempVf, tempVn));
    vnormal = commandV * cos(atan2(tempVf, tempVn));

    /////////////////Second order acc limit for trajectory planning
    if (!diveMode) {
        if (vforward >= 0) {
            if (vforward > (lastVf + conf->AccMaxForward * 0.0166667)) {
                vforward = lastVf + (conf->AccMaxForward * 0.0166667) * sign(vforward);
            }
            if (vforward < (lastVf - decCoef * conf->DecMax * 0.0166667)) {
                vforward = lastVf - (decCoef * conf->DecMax * 0.0166667);
            }
        } else {
            if (vforward < (lastVf - conf->AccMaxForward * 0.0166667)) {
                vforward = lastVf - (conf->AccMaxForward * 0.0166667);
            }
            if (vforward > (lastVf + decCoef * conf->DecMax * 0.0166667)) {
                vforward = lastVf + (decCoef * conf->DecMax * 0.0166667);
            }
        }
    }
////    debug(QString("vn: %1 , lVn :%2").arg(vnormal).arg(lastVn),D_MHMMD);
    if (vnormal >= 0) {
        if (diveMode) {
            if (vnormal > (lastVn + 10 * 0.0166667)) {
                vnormal = lastVn + (10 * 0.0166667) * sign(vnormal);
            }
        } else {
            if (vnormal > (lastVn + conf->AccMaxNormal * 0.0166667)) {
                vnormal = lastVn + (conf->AccMaxNormal * 0.0166667) * sign(vnormal);
            }
        }

        if (!diveMode && (vnormal < (lastVn - decCoef * conf->DecMax * 0.0166667))) {
            vnormal = lastVn - (decCoef * conf->DecMax * 0.0166667);
        }
    } else {
        if (diveMode) {
            if (vnormal < (lastVn - 10 * 0.0166667)) {
                vnormal = lastVn + (10 * 0.0166667) * sign(vnormal);
            }
        } else {
            if (vnormal < (lastVn - conf->AccMaxNormal * 0.0166667)) {
                vnormal = lastVn + (conf->AccMaxNormal * 0.0166667) * sign(vnormal);
            }
        }

        if (!diveMode && (vnormal > (lastVn + decCoef * conf->DecMax * 0.0166667))) {
            vnormal = lastVn + (decCoef * conf->DecMax * 0.0166667);
        }
    }


    if (vforward - lastVf < - 1) {
        vforward = lastVf - 0.085;
    }


    lastVf = vforward;
    lastVn = vnormal;
}

void Agent::setOnOffState(bool state) {
    onOffState = state;
}

void Agent::setCommandID(int ID) {
    commandID = ID;
}

Vector2D Agent::pos() {
    return wm->our[selfID]->pos;
}

Vector2D Agent::vel() {
    return wm->our[selfID]->vel;
}

Vector2D Agent::dir() {
    return wm->our[selfID]->dir;
}

bool Agent::shootSensor() {
    return false; //wm->our[selfID]->shootSensor;
}

void Agent::setShootSensor(bool b) {
    //wm->our[selfID]->shootSensor = b;
}

double Agent::angularVel() {
    return wm->our[selfID]->angularVel;
}

double Agent::dirDegree() {
    return wm->our[selfID]->dir.th().degree();
}

CRobot* Agent::self() {
    return wm->our[selfID];
}

Vector2D Agent::distToBall() {
    return wm->ball->pos - wm->our[selfID]->pos;
}

void Agent::setRobotAbsVel(double _vx, double _vy, double _w) {
    double ang = -dir().th().radian();
    //ROS_INFO_STREAM("ANG : " <<  ang);
    setRobotVel((cos(ang) * _vx) - (sin(ang) * _vy), (sin(ang) * _vx) + (cos(ang) * _vy), _w);
}

void Agent::setRobotVel(double _vtan , double _vnorm , double _w) {


    vforward = _vtan ;
    vnormal  = _vnorm ;
    vangular = _w * _RAD2DEG ;
    //ROS_INFO_STREAM("ANG :: " << vangular);
    double _v1, _v2, _v3, _v4;
    jacobian(_vtan , _vnorm , _w , _v1 , _v2 , _v3 , _v4);
    v1 = _v1;
    v2 = _v2;
    v3 = _v3;
    v4 = _v4;
}

void Agent::addRobotVel(double _vtan, double _vnorm, double _w) {

    double _v1, _v2, _v3, _v4;
    jacobian(_vtan , _vnorm , _w , _v1 , _v2 , _v3 , _v4);

    v1 += _v1;
    v2 += _v2;
    v3 += _v3;
    v4 += _v4;

    double vx , vy , w;
    jacobianInverse(v1 , v2 , v3 , v4 , vx , vy , w);

    vforward += _vtan;
    vnormal  += _vnorm;
    vangular += w * _RAD2DEG;
}

void Agent::waitHere() {
    setRobotVel(0.0, 0.0, 0.0);
    //  accelerationLimiter();
    setKick(0);
    setRoller(0);
    setBeep(false);
    setForceKick(false);
    idle = true;
    _ACC = 0;
    _DEC = 0;
}

void Agent::setForceKick(bool force) {
    forceKick = force;
}

void Agent::setKick(double _kickSpeed) {
    kickSpeed = _kickSpeed;
    chip = false;
}

void Agent::setChip(double _kickSpeed) {
    kickSpeed = _kickSpeed;
    chip = true;
}


void Agent::setBeep(bool _beep) {
    beep = _beep;
}

void Agent::setRoller(int state) {
    roller = state;
}

char Agent::getRoller() {
    return static_cast<char>(roller);
}

float Agent::getMotorMaxRadPerSec() {

    return static_cast<float>(1000 * 2 * M_PI / 60.0f);
}

float Agent::getvLimit() {
    return static_cast<float>((1.0f / (float)_BIT_RESOLUTION) * getMotorMaxRadPerSec() * self()->wheelRadius());
}

float Agent::getwLimit() {
    return static_cast<float>(((1.0f / (float)_BIT_RESOLUTION) * getMotorMaxRadPerSec() * self()->wheelRadius()) / self()->robotRadius());
}

void Agent::jacobian(double vx, double vy, double w, double &v1, double &v2, double &v3, double &v4) {
    float robotRadius = 0.0795;
    float wheelRadius = 0.0275;

    // Calculate Motor Speeds
    double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};

    double dw1 = (1.0 / wheelRadius) * (((robotRadius * w) - (vx * sin(motorAlpha[0])) + (vy * cos(motorAlpha[0]))));
    double dw2 = (1.0 / wheelRadius) * (((robotRadius * w) - (vx * sin(motorAlpha[1])) + (vy * cos(motorAlpha[1]))));
    double dw3 = (1.0 / wheelRadius) * (((robotRadius * w) - (vx * sin(motorAlpha[2])) + (vy * cos(motorAlpha[2]))));
    double dw4 = (1.0 / wheelRadius) * (((robotRadius * w) - (vx * sin(motorAlpha[3])) + (vy * cos(motorAlpha[3]))));

    float motorMaxRadPerSec = getMotorMaxRadPerSec();

    dw1 = (dw1 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw2 = (dw2 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw3 = (dw3 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw4 = (dw4 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    //todo wtp?
    v1 = (char)(round(dw1));
    v2 = (char)(round(dw2));
    v3 = (char)(round(dw3));
    v4 = (char)(round(dw4));
}

void Agent::jacobianInverse(double v1, double v2, double v3, double v4, double &vx, double &vy, double &w) {
    float motorMaxRadPerSec = getMotorMaxRadPerSec();
    double dw1 = v1 * motorMaxRadPerSec * self()->wheelRadius() / (double)_BIT_RESOLUTION;
    double dw2 = v2 * motorMaxRadPerSec * self()->wheelRadius() / (double)_BIT_RESOLUTION;
    double dw3 = v3 * motorMaxRadPerSec * self()->wheelRadius() / (double)_BIT_RESOLUTION;
    double dw4 = v4 * motorMaxRadPerSec * self()->wheelRadius() / (double)_BIT_RESOLUTION;
    vx = (-0.3464 * dw1) + (-0.2828 * dw2) + (0.2828 * dw3) + (0.3464 * dw4);
    vy = (0.4142 * dw1) + (-0.4142 * dw2) + (-0.4142 * dw3) + (0.4142 * dw4);
    w = (0.2929 * dw1) + (0.2071 * dw2) + (0.2071 * dw3) + (0.2929 * dw4);
}

#define _99 0.96f
double F(double d, double bnd) {
    d = fabs(d);
    double temp = tanh(d * atanh(_99) / bnd) / _99;
    return (temp);
}

Agent::Abilities::Abilities() {
    hasGyro = canChip = canKick = canSpin = highBattery = true;
}

Agent::Status::Status() {
    spin = shotSensor = fault = faild = halt = shotBoard = false;
    kickFault = chipFault = false;
    encoderFault[0] = encoderFault[1] = encoderFault[2] = encoderFault[3] = false;
    motorFault[0] = motorFault[1] = motorFault[2] = motorFault[3] = motorFault[4] = false;
    beep = false;
    shotSensorFault = false;
    boardID = -1;

}

double Agent::kickValueSpeed(double value, bool spinner) { //for onetouch
    return value; // TODO : fix
}

double Agent::kickSpeedValue(double speed, bool spinner) { //for pass speed
    // TODO : Move Profiler Here
    return speed;
}

double Agent::chipValueDistance(double value, bool spinner) { //for chip recieve
    // TODO : Move Profiler Here
    return sqrt(value * value / Gravity);
}

double Agent::getKickValue(bool chip, bool spin, double v) {
    double y, x, x2, x3, x4, x5, x6;
    int myId = selfID;



    if (chip) {
        x = v * 100;
    } else {
        x = v;
    }
    x2 = x * x;
    x3 = x2 * x;
    x4 = x2 * x2;
    x5 = x2 * x3;
    x6 = x3 * x3;

    switch (selfID) {
    case 3:
        if (chip && spin) {
            return 0.001 * x2 + 1.8639 * x + 58.865;
        }
        if (chip) {
            return 3E-05 * x2 + 2.8236 * x + 13.833;
        }
        return 0.0386 * x4 - 1.262 * x3 + 20.143 * x2 - 6.1246 * x + 37.614;
    case 4:
        if (chip && spin) {
            return 0.0015 * x2 + 2.9201 * x - 45.104;
        }
        if (chip) {
            return y = 0.0009 * x2 + 2.2283 * x + 16.247;
        }
        return -0.7326 * x4 + 14.181 * x3 - 83.857 * x2 + 272.69 * x - 205.66;
    case 5:
        if (chip && spin) {
            return -0.0009 * x2 + 3.422 * x + 38.807;
        }
        if (chip) {
            return 0.0015 * x2 + 2.8468 * x - 74.866;
        }
        return 0.0474 * x6 - 1.4049 * x5 + 15.668 * x4 - 80.694 * x3 + 198.6 * x2 - 155.04 * x + 75.944;
    case 2:
        if (chip && spin) {
            return 0.0004 * x2 + 2.0255 * x + 4.1774;
        }
        if (chip) {
            return 0.0032 * x2 + 1.1196 * x + 50.543;
        }
        return 0.0198 * x6 - 0.5191 * x5 + 4.9436 * x4 - 20.368 * x3 + 39.014 * x2 + 27.566 * x + 21.604;
    case 9:
        if (chip && spin) {
            return -0.0032 * x2 + 3.8923 * x + 6.5453;
        }
        if (chip) {
            return 0.0027 * x2 + 2.1671 * x + 54.724;
        }
        return -9E-14 * x6 + 2E-10 * x5 - 1E-07 * x4 + 4E-05 * x3 - 0.007 * x2 + 1.9532 * x - 1.6872;
    case 7:
        if (chip) {
            selfID = 5;
            y = getKickValue(true, 5, v);
            selfID = 7;
            return y;
        }
        return -0.0171 * x6 + 0.5062 * x5 - 5.7639 * x4 + 32.104 * x3 - 84.666 * x2 + 159.48 * x - 39.783;
    case 1:
        if (chip && spin) {
            return 0.0036 * x2 + 0.8745 * x + 88.982;
        }
        if (chip) {
            return 0.0031 * x2 + 2.2316 * x - 0.4071;
        }
        return 0.3568 * x4 - 6.2606 * x3 + 36.989 * x2 + 29.483 * x + 19.816;
    case 6:
        if (chip && spin) {
            return -0.001 * x2 + 3.8301 * x - 9.4641;
        }
        if (chip) {
            return -0.001 * x2 + 3.8301 * x - 9.4641;
        }
        return -0.0321 * x6 + 0.9598 * x5 - 11.038 * x4 + 61.438 * x3 - 163.39 * x2 + 254.1 * x - 71.466;
    default:
        myId = selfID;
        selfID = 1;
        y = getKickValue(chip, spin, v);
        selfID = static_cast<short>(myId);
        return y;
    }
}


// TODO : Fix it With Profiler
double Agent::chipDistanceValue(double distance, bool spinner) {
    return sqrt(distance * Gravity);
}

int Agent::kickValueForDistance(double dist, double finalVel) {
    double a = 0.66;//BallFriction()*Gravity;
    double temp, vel;
    temp = finalVel * finalVel + 2 * a * dist;
    if (temp < 0) {
        temp = 0;
    }
    vel = sqrt(temp);
    //  qDebug() << "vel=" << vel;
    return static_cast<int>(kickSpeedValue(vel, false));
}


Vector2D Agent::oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDirection) {
    Vector2D oneTouchDir = Vector2D::unitVector(CSkillKickOneTouch::oneTouchAngle(pos(), Vector2D(0, 0), (pos() - wm->ball->pos).norm(),
                           pos() - wm->ball->pos, wm->field->oppGoal(),
                           conf->Landa,
                           conf->Gamma));
    Vector2D q;
    q.invalidate();
    bool oneTouchKick = false;
    if (wm->ball->vel.length() > 0.1 && (wm->ball->pos - pos()).length() < 1.0) {
        oneTouchKick = true;
    }
    //todo ballComingSpeed
    if (1/*self()->ballComingSpeed()*/ > 0.1) {
        Line2D l(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm());
        q = l.projection(positioningPos);
        DBUG("case", D_ERROR);
        if (q.valid() && (q - positioningPos).length() < 1.0) {
            DBUG("case2", D_ERROR);
            if ((wm->ball->pos - pos()).length() < 1.0) {
                oneTouchKick = true;
            }
            q -= (self()->centerFromKicker() + CBall::radius) * oneTouchDir;
        }
    }
    if (oneTouchKick) {
        if (fabs(Vector2D::angleBetween(self()->dir, wm->field->oppGoal() - self()->pos).degree()) < 45) {
            DBUG("case3", D_ERROR);
            setKick(chipDistanceValue(8 , false));
        }
        DBUG("case4", D_ERROR);
    }
    *oneTouchDirection = oneTouchDir;
    return q;
}

//IMPORTANT
////CKS

bool Agent::canOneTouch() {
    drawer->draw(QString("%1 , %2").arg(1/*self()->ballComingSpeed()*/).arg(fabs(Vector2D::angleBetween(wm->ball->vel.norm(), (pos() - wm->ball->pos).norm()).degree())), Vector2D(0, 1));
    drawer->draw(Segment2D(Vector2D(0, 0) , (pos() - wm->ball->pos).norm()), "blue");
    drawer->draw(Segment2D(Vector2D(0, 0) , wm->ball->vel.norm()), "red");
    if (1/*self()->ballComingSpeed()*/ > 0.9) {
        if (fabs(Vector2D::angleBetween(wm->ball->vel.norm(), (pos() - wm->ball->pos).norm()).degree()) < 21) {
            return true;
        }
    }
    return false;
}

void Agent::setGyroZero() {
    //  if ( calibrated < 999)
    //    return;
    calibrated = 0;
    //    if( id() == 4)
    agentAngelForGyro = self()->dir;
    calibrateGyro = true;
    DEBUG(QString("Calibrated ! ang : %1").arg(agentAngelForGyro.dir().degree()), D_SEPEHR);
}
void Agent::initPlanner(const Vector2D &_target, const QList<int> &_ourRelaxList,
                        const QList<int> &_oppRelaxList, const bool &_avoidPenaltyArea, const bool &_avoidCenterCircle,
                        const double &_ballObstacleRadius) {
    parsian_msgs::parsian_get_planPtr plan{new parsian_msgs::parsian_get_plan};
    plan->robotID = this->id();
    plan->goal = _target.toParsianMessage();
    plan->start = this->pos().toParsianMessage();
    Q_FOREACH (const int &id, _ourRelaxList) {
        plan->ourRelaxList.push_back(id);
    }
    Q_FOREACH (const int&id, _oppRelaxList) {
        plan->oppRelaxList.push_back(id);
    }
    plan->avoidCenterCircle = _avoidCenterCircle;
    plan->ballObstacleRadius = _ballObstacleRadius;
    plan->avoidPenaltyArea = _avoidPenaltyArea;
    plan->header.stamp = ros::Time::now();
    // TODO : Add Virtual Obstacle to This
    planner_pub.publish(plan);
    ROS_INFO_STREAM("PUBLISHED");
    // TODO : remove below kindly
//    planner.initPathPlanner(_target , _ourRelaxList , _oppRelaxList ,  _avoidPenaltyArea, _avoidCenterCircle, _ballObstacleRadius);
//    getPathPlannerResult(planner.getResultModified(), planner.getAverageDir());
}

void Agent::getPathPlannerResult(vector<Vector2D> _result , Vector2D _averageDir) {
    pathPlannerResult.clear();
    pathPlannerResult.assign(_result.begin() , _result.end());
    plannerAverageDir = _averageDir.norm();
    ROS_INFO_STREAM("SUBS");
}

void Agent::execute() {

    skill->execute();

    //planner.generateObstacleSpace(obst  , ourRelaxList , oppRelaxList , avoidPenaltyArea, avoidCenterArea , ballObstacleRadius,ID,goal);
    //planner.runPlanner();
    //emit pathPlannerResult(resultModified ,averageDir); get this variables
}

parsian_msgs::parsian_robot_commandPtr Agent::getCommand() {
    //  ROS_INFO("CommunicationCommand_generated");
    parsian_msgs::parsian_robot_commandPtr command{new parsian_msgs::parsian_robot_command};


    command->robot_id = static_cast<unsigned char>(id());
    command->chip = static_cast<unsigned char>(chip);
    command->packet_id = static_cast<unsigned char>(1);
    command->roller_speed = static_cast<unsigned char>(roller);
//    command.forceKick= static_cast<unsigned char>(forceKick);
    command->kickSpeed = static_cast<unsigned short>(kickSpeed);
    command->vel_F = vforward;
    command->vel_N = vnormal;
    command->vel_w = vangular;
    command->release = false;//static_cast<unsigned char>(onOffState);
    ///////////////////////////////merge with grsim///////////////////////////////////////////

    double w1, w2, w3, w4;
    jacobian(vforward, vnormal, vangular * _DEG2RAD, w1, w2, w3, w4);
    command->wheelsspeed = static_cast<unsigned char>(true);
    command->wheel1 = static_cast<float>(w1);
    command->wheel2 = static_cast<float>(w2);
    command->wheel3 = static_cast<float>(w3);
    command->wheel4 = static_cast<float>(w4);

    if (chip) {
        command->kickspeedz = static_cast<float>(kickSpeed);
    } else {
        command->kickspeedz = 0;
    }
    command->spinner = static_cast<unsigned char>(false);
    command->header.stamp = ros::Time::now();
    return command;
}
