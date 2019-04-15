
#include <parsian_ai/roles/support.h>

#include "parsian_ai/roles/support.h"

//INIT_ROLE(INIT_ROLECRoleSupport, "support");

CRoleSupport::CRoleSupport(Agent *_agent) : CRole(_agent) {
}

CRoleSupport::~CRoleSupport() {
}

void CRoleSupport::execute() {
    update();
    switch (supporterSkill) {

        case SupporterSkill::NoSkill:
            break;
        case SupporterSkill::Ready:
            break;
        case SupporterSkill::OneTouch:
            break;
        case SupporterSkill::Move:
            agent->action = moveSkill;
            break;
    }


    return;
    Vector2D kickTar;
    double kickW;
    QList<int> ourRelId, oppRelId;
    oppRelId.

            clear();

    ourRelId.

            clear();

    ourRelId.
            append(agent
                           ->

                                   id()

    );
    kickTar = know->getEmptyPosOnGoal(agent->pos(), kickW, true, ourRelId, oppRelId);
//    kick->setTarget(kickTar);
//    kick->setKickspeed(7.5); // todo: check this
//    kick->setTolerance(0.2);
//    kick->setInterceptmode(true);
//    kick->setSlow(false);
    DEBUG("supporting kick", int(D_SEPEHR));
    if (supportPosition.
            dist(wm
                         ->field->

            ourGoal()

    ) < 2) {
        supportPosition = (wm->ball->pos - wm->field->ourGoal()).norm() * 1.8 + wm->field->ourGoal();
//        gotopoint->setLookat(wm->ball->pos - supportPosition);
//        gotopoint->setTargetpos(supportPosition);
    }
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

void CRoleSupport::update() {
    moveSkill->setTargetpos(target);
}
