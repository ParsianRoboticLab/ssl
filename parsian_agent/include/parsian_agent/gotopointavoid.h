//
// Created by parsian-ai on 9/29/17.
//

#ifndef PARSIAN_SKILLS_GOTOPOINTAVOID_H
#define PARSIAN_SKILLS_GOTOPOINTAVOID_H

#include <parsian_agent/gotopoint.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>

class CSkillGotoPointAvoid : public CSkill, public GotopointavoidAction {
private:
    CNewBangBang *bangBang;

protected:

    Vector2D targetVel;
    Vector2D agentPos;
    Vector2D agentVel;

    CSkillGotoPoint* gotopoint;
    QList<int> ourRelaxList , oppRelaxList;
    QList <Vector2D> pathPoints;
    int counter;
    Vector2D averageDir;

public:
    Vector2D ballPos;
    void init(Vector2D _target, Vector2D _targetDir, Vector2D _targetVel = Vector2D(0.0, 0.0));
    static double timeNeeded(const Agent *_agentT, const Vector2D& posT,const double& vMax);
    DEF_SKILL(CSkillGotoPointAvoid);
    CSkillGotoPointAvoid* noRelax();
    CSkillGotoPointAvoid* ourRelax(int element);
    CSkillGotoPointAvoid* oppRelax(int element);
    CSkillGotoPointAvoid* setTargetLook(Vector2D finalPos, Vector2D lookAtPoint);
    CSkillGotoPointAvoid* setTarget(Vector2D finalPos, Vector2D finalDir);
    QList<Vector2D> result;

};

#endif //PARSIAN_SKILLS_GOTOPOINTAVOID_H
