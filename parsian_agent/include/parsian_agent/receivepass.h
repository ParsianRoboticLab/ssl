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
enum class RPMode {
    RPNONE = 0,
    RPWAITPOS = 1,
    RPINTERSECT = 2,
    RPRECEIVE = 3
};

class CSkillReceivePass : public CSkill, public ReceivepassAction {
private:
    RPMode decideMode();
    void waitPos();
    void intersect();
    void receive();

    Vector2D bestPointToIntersect();
    void validatePoint(Vector2D& _point);
    void validatePointFromPenalty(Vector2D& _point, const Rect2D& _penalty);
    void validatePointOutofField(Vector2D& _point);

    CSkillGotoPointAvoid* gotopointavoid;
public:

    explicit DEF_SKILL(CSkillReceivePass);
};




#endif //PARSIAN_SKILLS_RECEIVEPASS_H
