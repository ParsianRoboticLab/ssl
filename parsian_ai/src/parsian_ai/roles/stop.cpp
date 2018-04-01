#include "parsian_ai/roles/stop.h"


const double StopRadius = 0.55;//0.70;//1.02;//0.78;
CRoleStopInfo* CRoleStop::m_info = new CRoleStopInfo("stop");

CRoleStopInfo::CRoleStopInfo(QString _roleName)
        : CRoleInfo(_roleName) {
    //  inCorner = -1;
}

Vector2D CRoleStopInfo::getEmptyTarget(const Vector2D& _position, const double& _radius) {
    Vector2D tempTarget, finalTarget;
    bool opp;
    finalTarget = _position;
    for (double dist = 0.2 ; dist <= _radius ; dist += 0.2) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 18.0/dist) {
            opp = false;
            tempTarget = _position + Vector2D::polar2vector(dist, ang);
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.25).contains(tempTarget)
                    || !wm->field->isInField(tempTarget)
                    || wm->field->isInOppPenaltyArea(tempTarget)
                    || wm->field->isInOurPenaltyArea(tempTarget)) {
                    opp = true;
                    break;
                }

            }
            if (!opp) {
                finalTarget = tempTarget;
                dist = _radius*2; // to break upper loop
                break;
            }
        }
    }

    return finalTarget;
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
    robotId.clear();
    if (count() > 0) {
        Ps.append(c + baseDirVec * (sRadius + Robot::robot_radius_new));
    }
    Vector2D startPos{0, -wm->field->_FIELD_WIDTH/2 + 1};
    Vector2D endPos  {0,  wm->field->_FIELD_WIDTH/2 - 1};
    for (int i = 1; i < count(); i++) {
        Ps.append(getEmptyTarget(startPos*(1.0 - (double)(i)/count()) + endPos*((double)(i)/count()), 1));
    }

    MWBM matcher;
    matcher.create(count(), Ps.count());
    for (int i = 0; i < count(); i++)
        for (int j = 0 ;j < Ps.count(); j++)
            matcher.setWeight(i, j, -Ps[j].dist(robot(i)->pos()));

    matcher.findMatching();
    for (int i = 0; i < count(); i++) {
        robotId.append(robot(matcher.getMatch(i))->id());
    }

    return;

    //// OLD STOP
    /*
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
    */
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
    for (int k = 0; k < info()->count(); k++) {
        if (agent->id() == info()->robotId[k]) {
            target = info()->Ps[k];
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
    gotopoint->setLookat(wm->field->oppGoal());
    gotopoint->setAvoidpenaltyarea(true);

    gotopoint->setBallobstacleradius(0.50);
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
