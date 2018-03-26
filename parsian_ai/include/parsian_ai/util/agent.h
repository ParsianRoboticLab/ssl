//
// Created by parsian-ai on 12/7/17.
//

#ifndef PARSIAN_AI_AGENT_H
#define PARSIAN_AI_AGENT_H

#include <parsian_util/core/agent.h>

class Agent : public CAgent {
public:
    explicit Agent(const parsian_msgs::parsian_agent& _agent);
    explicit Agent(int id);
    Vector2D oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDirection);
    QString roleName;
    bool changeIsNeeded = false;
    bool shootSensor = false;
protected:
private:
};


#endif //PARSIAN_AI_AGENT_H
