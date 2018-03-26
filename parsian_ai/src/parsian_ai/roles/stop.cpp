#include "parsian_ai/roles/stop.h"


const double StopRadius = 0.55;//0.70;//1.02;//0.78;
CRoleStopInfo* CRoleStop::m_info = new CRoleStopInfo("stop");

CRoleStopInfo::CRoleStopInfo(QString _roleName)
    : CRoleInfo(_roleName) {
    //  inCorner = -1;
}

void CRoleStopInfo::findPositions() {

    double sRadius = StopRadius;
    Vector2D c = wm->ball->pos;
    const double radius = 1.8 + 2.0 * Robot::robot_radius_new;

    TA = wm->field->ourGoal();
    if ((wm->ball->pos - wm->field->ourGoal()).length() < radius) {
        TA = wm->field->oppGoal();
    } else {
        TA = wm->field->ourGoal();
    }

    Vector2D baseDirVec;
    baseDirVec = (TA - c).norm();


    Ps.clear();
    int coef = 1;
    if (count() > 0) {
        Ps.append(c + baseDirVec * (sRadius + Robot::robot_radius_new));
    }
    for (int i = 1 ; i < count() ; i++) {
        Ps.append(c + baseDirVec.rotatedVector((coef * ((i + 1) / 2))*_RAD2DEG * 0.2 / 0.6) * (sRadius + Robot::robot_radius_new));
        coef *= -1;
    }

    thR.clear();
    thP.clear();
    robotId.clear();
    for (int i = 0; i < count(); i++) {
        thP.append(Vector2D::angleBetween(Ps[i] - c, baseDirVec).degree());
        robotId.append(robot(i)->id());
        thR.append(Vector2D::angleBetween(robot(i)->pos() - c, baseDirVec).degree());
    }

    for (int i = 0; i < thP.size(); i++) {
        for (int j = i + 1; j < thP.size(); j++) {
            if (thP[i] < thP[j]) {
                thP.swap(i, j);
                Ps.swap(i, j);
            }
        }
    }
    for (int i = 0; i < thR.size(); i++) {
        for (int j = i + 1; j < thR.size(); j++) {
            if (thR[i] < thR[j]) {
                thR.swap(i, j);
                robotId.swap(i, j);
            }
        }
    }

}

CRoleStop::CRoleStop(Agent *_agent) : CRole(_agent) {
    gotopoint = new GotopointavoidAction();
    noAction  = new NoAction();
}

CRoleStop::~CRoleStop() {
    delete gotopoint;
    delete noAction;
}

void CRoleStop::execute() {

    Vector2D target;
    info()->findPositions();
    int kkk = 0;
    for (int k = 0; k < info()->count(); k++) {
        if (agent->id() == info()->robotId[k]) {
            target = info()->Ps[k];
            kkk = k;
        }
    }


    gotopoint->setSlowmode(true);
    if (wm->ball->inSight <= 0 || !wm->ball->pos.valid() || !wm->field->isInField(wm->ball->pos)) {
        noAction->setWaithere(true);
        agent->action = noAction;
        return;
    }
    gotopoint->setTargetpos(target);
    gotopoint->setTargetdir(Vector2D(1.0, 0.0));
    gotopoint->setLookat(wm->ball->pos);
    gotopoint->setAvoidpenaltyarea(true);

    gotopoint->setBallobstacleradius(0.50);
    drawer->draw(Circle2D(target , 0.03) , "magenta" , true);
    drawer->draw(QString("%1").arg(kkk) , target);
    ROS_INFO_STREAM("DDD " << agent->id());
    agent->action = gotopoint;


}

void CRoleStop::parse(QStringList params) {
    switchAgent = false;
    for (int i = 0; i < params.length(); i++) {
        if (params[i].toLower() == "switch") {
            switchAgent = true;
        }
    }
}

double CRoleStop::progress() {
    return 0.0;
}

CRoleStopInfo* CRoleStop::info() {
    return m_info;
}

void CRoleStop::assign(Agent *_agent) {
    CRole::assign(_agent);
    if (agent != nullptr) {
        agent->roleName = m_info->getRoleName();
        m_info->addAgent(agent);
    }
}


//--------------------------------Halt--------------------------------//

CRoleHaltInfo::CRoleHaltInfo(QString _roleName) : CRoleInfo(_roleName) {

}

CRoleHalt::CRoleHalt(Agent *_agent) : CRole(_agent) {
    noAction = new NoAction();
}

CRoleHalt::~CRoleHalt() {
    delete noAction;
}

void CRoleHalt::execute() {
    noAction->setWaithere(true);
    agent->action = noAction;
}

double CRoleHalt::progress() {
    return 1.0;
}
