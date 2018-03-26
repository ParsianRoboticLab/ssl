#include "gaurd.h"

INIT_ROLE(CRoleGaurd, "gaurd");

CRoleGaurd::CRoleGaurd(Agent *_agent) : CRole(_agent) {
    gotopoint = new CSkillGotoPointAvoid(_agent);
}

CRoleGaurd::~CRoleGaurd() {
    delete gotopoint;
}

void CRoleGaurd::execute() {
    gotopoint->setAgent(agent);
    Vector2D point = wm->ball->pos - (wm->ball->pos - wm->field->ourGoal()).norm() * 1.5;
    if (opposite) {
        point = wm->ball->pos - (wm->ball->pos - wm->field->oppGoal()).norm() * 1.5;
    }
    gotopoint->setTargetLook(point, wm->ball->pos)->execute();
}

double CRoleGaurd::progress() {
    return 0.0;
}

void CRoleGaurd::parse(QStringList params) {
    setOpposite(false);
    for (int i = 0; i < params.length(); i++) {
        if (params[i].toLower() == "opposite") {
            setOpposite(true);
        }
    }
}

CRoleGaurdInfo::CRoleGaurdInfo(QString _roleName) : CRoleInfo(_roleName) {

}

