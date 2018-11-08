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

    int counter;
    Vector2D averageDir;
    QList<int> ourRelaxList , oppRelaxList;
    CNewBangBang *bangBang;
    CSkillGotoPoint* gotopoint;
    QList <Vector2D> pathPoints;

public:
    explicit CSkillGotoPointAvoid(Agent* _agent);
    ~CSkillGotoPointAvoid();

    void execute() override;
    void init(Vector2D _target, Vector2D _targetDir, Vector2D _targetVel = Vector2D(0.0, 0.0));
    static double timeNeeded(const Agent *_agentT, const Vector2D& posT,const double& vMax);

};

#endif //PARSIAN_SKILLS_GOTOPOINTAVOID_H
