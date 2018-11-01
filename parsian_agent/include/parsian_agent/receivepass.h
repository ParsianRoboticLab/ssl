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
    void validatePointFromPenalty(Vector2D& _point, const Rect2D& _penalty);
    void validatePoint(Vector2D& _point);
    CSkillGotoPointAvoid* gotopointavoid;
public:
    static void validatePointOutofField(Vector2D& _point);
    static void validatePoint(Vector2D& _point, const Vector2D& _default);
    static Vector2D bestPointToIntersect(const Agent* _agent, const double& reachBeforeBall = 0.5);
    static void validatePointFromPenalty(Vector2D &_point, const Rect2D &_penalty, const Vector2D &_target);

    explicit DEF_SKILL(CSkillReceivePass);
};




#endif //PARSIAN_SKILLS_RECEIVEPASS_H
