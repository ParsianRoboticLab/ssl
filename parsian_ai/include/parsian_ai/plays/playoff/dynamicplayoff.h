//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_DYNAMICPLAYOFF_H
#define PARSIAN_AI_DYNAMICPLAYOFF_H

#include <parsian_ai/plays/playoff.h>


enum class DynamicSelect {
    NoSelect = 0,
    Khafan = 1,
    Chip = 2,
    Kick = 3,
    Blocker = 4
};


class CDynamicPlayoff {
public:
    CDynamicPlayoff();
    ~CDynamicPlayoff() override;
    void reset() override;
    void execute_x() override;
    void init(const QList<Agent*>& _agents) override;
    QString whoami() override { return "Dynamic Playoff"; }
    CRolePlayOff *roleAgents[_NUM_PLAYERS];

private:
    bool initial;

    Vector2D lastBallPos;
    unsigned int lastTime;


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

    void initDynamicPlay(const QList<int> &_ourplayers);

};


#endif //PARSIAN_AI_DYNAMICPLAYOFF_H
