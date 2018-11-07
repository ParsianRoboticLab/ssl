//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_DYNAMICPLAYOFF_H
#define PARSIAN_AI_DYNAMICPLAYOFF_H

#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <parsian_ai/config.h>
#include <QString>

enum class DynamicSelect {
    NoSelect = 0,
    Khafan = 1,
    Chip = 2,
    Kick = 3,
    Blocker = 4
};


class CDynamicPlayOff {
public:
    CDynamicPlayOff();
    ~CDynamicPlayOff();
    void reset();
    void execute();
    void init(const QList<Agent*>& _agents);
    CRolePlayOff *roleAgents[_NUM_PLAYERS];
    void initDynamicPlay(const QList<Agent*> &_ourplayers);

private:
    bool initial;
    QList<Agent*> agents;
    Vector2D lastBallPos;
    unsigned int lastTime;

    bool playOnFlag;
    void dynamicExecute();
    int dynamicMatch[_NUM_PLAYERS];
    DynamicSelect dynamicSelect;
    void dynamicAssignID();
    void dynamicPlayKhafan();
    void dynamicPlayBlocker();
    void dynamicPlayChipToGoal(bool isChip);

    void checkEndKhafan();
    void checkEndBlocker();
    void checkEndChipToGoal();
    Vector2D getDynamicTarget(int i);

    int dynamicAgentSize;
    bool ready, pass, shot;
    int dynamicState;
    unsigned int dynamicStartTime;


};


#endif //PARSIAN_AI_DYNAMICPLAYOFF_H
