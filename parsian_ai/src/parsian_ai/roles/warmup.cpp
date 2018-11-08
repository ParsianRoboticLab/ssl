#include "warmup.h"

INIT_ROLE(CRoleWarmup, "warmup");

CRoleWarmup::CRoleWarmup(Agent *_agent) : CRole(_agent) {
    gotopoint = new CSkillGotoPointAvoid(_agent);
}

CRoleWarmup::~CRoleWarmup() {
    delete gotopoint;
}

void CRoleWarmup::execute() {
    Vector2D pos;
    info()->findPos2();
    for (int k = 0; k < info()->count(); k++) {
        if (agent->id() == info()->robotId[k]) {
            pos = info()->Ps[k];
        }
    }
    gotopoint->setAgent(agent);
    gotopoint->setTargetLook(pos, info()->cent);
    gotopoint->execute();
}

double CRoleWarmup::progress() {
    return 0.0;
}

void CRoleWarmup::parse(QStringList params) {
    for (int i = 0; i < params.count(); i++) {
//        if (params[i].trimmed().toLower()=="stop") setStop(true);
//        else if (params[i].trimmed().toLower()=="goal") setBlockGoal(true);
    }
}


CRoleWarmupInfo::CRoleWarmupInfo(QString _roleName) : CRoleInfo(_roleName) {
    cent.assign(0.0, 0.0);

}

void CRoleWarmupInfo::findPos1() {
    this->Ps.clear();
    for (int i = 0; i < count(); i++) {
        this->Ps.append(Vector2D(-2 - (0.35 * i), 1.5));
    }

    distR.clear();
    distP.clear();
    robotId.clear();
    for (int i = 0; i < count(); i++) {
        distP.append(Ps[i].dist(Vector2D(-2, 1.5)));
        robotId.append(robot(i)->id());
        distR.append(robot(i)->pos().dist(Vector2D(2, 1)));
    }

    for (int i = 0; i < distP.size(); i++) {
        for (int j = i + 1; j < distP.size(); j++) {
            if (distP[i] < distP[j]) {
                distP.swap(i, j);
                Ps.swap(i, j);
            }
        }
    }
    for (int i = 0; i < distR.size(); i++) {
        for (int j = i + 1; j < distR.size(); j++) {
            if (distR[i] < distR[j]) {
                distR.swap(i, j);
                robotId.swap(i, j);
            }
        }
    }
}


void CRoleWarmupInfo::findPos2() {
    static double ang = 0;
    static double rad = 0.3;
    Circle2D cir(cent, 0.3);
    this->Ps.clear();
    Vector2D dire(0 , 1);
    dire.rotate(ang);
    for (int i = 0; i < count(); i++) {
        dire.rotate((360 / count()));
        Segment2D seg(cent, cent + (dire.norm() * 0.5));
        draw(seg, "orange");
        Vector2D s1, s2;
        cir.intersection(seg, &s1, &s2);
        if (s1.valid()) {
            this->Ps.append(s1);
        } else if (s2.valid()) {
            this->Ps.append(s2);
        }
    }

    distR.clear();
    distP.clear();
    robotId.clear();
    for (int i = 0; i < count(); i++) {
        distP.append(Ps[i].dist(Vector2D(-2, 1.5)));
        robotId.append(robot(i)->id());
        distR.append(robot(i)->pos().dist(Vector2D(2, 1)));
    }

    ang += 0.25;
    static int sx = 1;
    if (fabs(cent.x) > 2.5) {
        sx *= -1;
    }
    static int sy = 1;
    if (fabs(cent.y) > 1.5) {
        sy *= -1;
    }
    wm->ball->setReplace(cent - Vector2D(sx, sy) * 0.1, Vector2D(0, 0));
    cent.x += (rad + Robot::robot_radius_old) * 0.25 * _DEG2RAD * sx;
    cent.y += (rad + Robot::robot_radius_old) * 0.25 * _DEG2RAD * sy;

}
