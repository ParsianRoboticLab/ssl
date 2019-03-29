//
// Created by atiyeh on 3/27/19.


#ifndef PARSIAN_AGENT_MOVEFORWARD_H
#define PARSIAN_AGENT_MOVEFORWARD_H

#include <parsian_agent/skill.h>
#include <parsian_agent/agent.h>
#include <parsian_util/action/autogenerate/moveforwardaction.h>
#include <parsian_agent/gotopointavoid.h>
#include "receivepass.h"

class CSkillMoveForward : public CSkill, public MoveforwardAction {
private:
    void kickforward(double kickSpeed);
    CSkillGotoPointAvoid *gtpAvoid;
    CSkillReceivePass *recPass;
public:
    explicit CSkillMoveForward(Agent* _agent);
    ~CSkillMoveForward();
    void execute() override;
};

#endif //PARSIAN_AGENT_MOVEFORWARD_H