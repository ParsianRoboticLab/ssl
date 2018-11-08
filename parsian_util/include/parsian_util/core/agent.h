#ifndef AGENT_H
#define AGENT_H

#include "parsian_msgs/parsian_agent.h"
#include <parsian_util/core/robot.h>
#include <parsian_util/action/action.h>

class CAgent {
public:
    CAgent(const parsian_msgs::parsian_agent& _agent);
    CAgent(int id);
    Vector2D pos();
    Vector2D vel();
    Vector2D dir();
    Vector2D acc();
    int       id();
    bool isVisible() {
        return self.getActive();
    }

    Action* action;
    CRobot self;
};

#endif // AGENT_H
