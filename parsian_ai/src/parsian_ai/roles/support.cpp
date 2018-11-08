#include "support.h"

INIT_ROLE(CRoleSupport, "support");

CRoleSupport::CRoleSupport(Agent *_agent) : CRole(_agent) {
    gotopoint = new CSkillGotoPointAvoid(_agent);
    kick = new CSkillKick(_agent);
}

CRoleSupport::~CRoleSupport() {
    delete gotopoint;
}

void CRoleSupport::execute() {
    kick->setAgent(agent);
    info()->findPos();
    gotopoint->setAgent(agent);

    draw(info()->supportPosition, 0, "cyan");
    if (agent->self()->isBallOwner(0.03)) {
        Vector2D kickTar;
        double kickW;
        QList<int> ourRelId, oppRelId;
        oppRelId.clear();
        ourRelId.clear();
        ourRelId.append(agent->id());
        agent->setRoller(3);
        kickTar = knowledge->getEmptyPosOnGoal(agent->self()->getKickerPos(), kickW, true, ourRelId, oppRelId);
        kick->setTarget(kickTar);
        kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.5 , false));
        kick->setTolerance(0.2);
        kick->setInterceptMode(true);
        kick->setSlow(false);
        kick->execute();
        debug("supporting kick", int(D_SEPEHR));
    }
    if (!info()->supportPosition.valid() || info()->supportPosition.dist(wm->field->ourGoal()) < 1.8) {
        info()->supportPosition = (wm->ball->pos - wm->field->ourGoal()).norm() * 1.8 + wm->field->ourGoal();
    } else {
        if (knowledge->getSupporPlaymaker() == CKnowledge::Back) {
            gotopoint->setTargetLook(info()->supportPosition, wm->ball->pos)->execute();
        } else {
            gotopoint->setTargetLook(info()->supportPosition, wm->field->oppGoal())->execute();
        }
    }
}

double CRoleSupport::progress() {
    return 0.0;
}

void CRoleSupport::parse(QStringList params) {
    //    setStop(false);
    //    setBlockGoal(false);
    //    for (int i=0;i<params.count();i++)
    //    {
    //        if (params[i].trimmed().toLower()=="stop") setStop(true);
    //        else if (params[i].trimmed().toLower()=="goal") setBlockGoal(true);
    //    }
}

CRoleSupportInfo::CRoleSupportInfo(QString _roleName) : CRoleInfo(_roleName) {

}

void CRoleSupportInfo::findPos() {
    if (calculated) {
        return;
    }
    calculated = true;
    Vector2D playmakerPos;
    if (knowledge->getPlayMaker() == NULL) {
        supportPosition = ((wm->field->ourGoal() - wm->ball->pos).norm() * (Robot::robot_radius_old * 3.0 + 0.3)) + wm->ball->pos;
        return;
    }
    playmakerPos = knowledge->getPlayMaker()->pos();
    if (knowledge->getSupporPlaymaker() == CKnowledge::Back) {
        debug("supporting back", int(D_SEPEHR));
        supportPosition = ((wm->field->ourGoal() - wm->ball->pos).norm() * (Robot::robot_radius_old * 4.5 + playmakerPos.dist(wm->ball->pos))) + wm->ball->pos;
        return;
    }
    if (knowledge->getSupporPlaymaker() == CKnowledge::Left) {
        debug("supporting left", int(D_SEPEHR));
        supportPosition.assign(playmakerPos.x - 0.03, playmakerPos.y + Robot::robot_radius_old * 2 + 0.06);
        return;
    }
    if (knowledge->getSupporPlaymaker() == CKnowledge::Right) {
        debug("supporting right", int(D_SEPEHR));
        supportPosition.assign(playmakerPos.x - 0.03, playmakerPos.y - Robot::robot_radius_old * 2 - 0.06);
        return;
    }
    if (knowledge->getSupporPlaymaker() == CKnowledge::Front) {
        debug("supporting front", int(D_SEPEHR));
        supportPosition = ((wm->field->oppGoal() - wm->ball->pos).norm() * 0.45) + wm->ball->pos;
        return;
    }
    supportPosition = (wm->field->oppGoal() - playmakerPos).norm() * 0.4 + playmakerPos;
}
