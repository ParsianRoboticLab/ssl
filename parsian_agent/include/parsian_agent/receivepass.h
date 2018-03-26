//
// Created by parsian-ai on 9/21/17.
//

#ifndef PARSIAN_SKILLS_RECEIVEPASS_H
#define PARSIAN_SKILLS_RECEIVEPASS_H

#include <parsian_agent/skill.h>
#include <parsian_agent/gotopoint.h>
#include <parsian_agent/kick.h>
#include <parsian_util/action/autogenerate/receivepassaction.h>

/////////////////////////////////////////////////////////////// receive pass skill created by DON MHMMD SHIRAZI
enum kkRPMode {
    RPWAITPOS = 1,
    RPINTERSECT = 2,
    RPDAMP = 3,
    RPRECEIVE = 4,
    RPNONE = 5
};

class CSkillReceivePass : public CSkill, public ReceivepassAction {
private:
    CSkillGotoPointAvoid* gotopointavoid;
    Vector2D ballPos;
    Vector2D kkAgentPos;
    kkRPMode receivePassMode;
    kkRPMode decideMode();
    Circle2D receiveArea;
    double cirThresh;
    double kickCirThresh;
public:
    double ballRealVel;
    DEF_SKILL(CSkillReceivePass);
};




#endif //PARSIAN_SKILLS_RECEIVEPASS_H
