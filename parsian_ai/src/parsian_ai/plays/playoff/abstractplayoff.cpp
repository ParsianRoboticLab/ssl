//
// Created by parsian-ai on 11/8/18.
//

#include <parsian_ai/plays/playoff/abstractplayoff.h>

#include "parsian_ai/plays/playoff/abstractplayoff.h"

CAbstractPlayOff::CAbstractPlayOff() {
    for (auto &roleAgent : roleAgents) roleAgent = new CRolePlayOff();

}

CAbstractPlayOff::~CAbstractPlayOff() {
    for (auto &roleAgent : roleAgents) delete roleAgent;

}

bool CAbstractPlayOff::getPlayonFlag() {
    return playOnFlag;
}

void CAbstractPlayOff::matchAgent() {
    MWBM matcher;
    matcher.create(agents.size(), agents.size());
    for (int i = 0; i < agents.size(); i++) {
        for (int j = 0; j < agents.size(); j++) {
            Vector2D target;
            switch (roleAgents[i]->getSelectedSkill()) {
                case RoleSkill::Gotopoint:
                case RoleSkill::GotopointAvoid:
                case RoleSkill::ReceivePass:
                    target = roleAgents[i]->getTarget();
                    break;
                case RoleSkill::Kick:
                    target = wm->ball->pos;
                    break;
                case RoleSkill::OneTouch:
                    target = roleAgents[i]->getWaitPos();
                    break;
                case RoleSkill::Mark:
                case RoleSkill::Support:
                case RoleSkill::Defense:
                    target.invalidate();
                    break;
            }
            double weight = target.dist(agents[j]->pos());
            matcher.setWeight(i, j, -(weight));
        }
    }
    matcher.findMatching();
    for (int i = 0; i < agents.size(); i++) {
        roleAgents[i]->setAgent(agents[matcher.getMatch(i)]);
    }
}
