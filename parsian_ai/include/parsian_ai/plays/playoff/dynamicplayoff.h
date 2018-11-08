//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_DYNAMICPLAYOFF_H
#define PARSIAN_AI_DYNAMICPLAYOFF_H

#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <parsian_ai/plays/playoff/abstractplayoff.h>
#include <parsian_ai/config.h>
#include <QString>

enum class DynamicSelect {
    NoSelect = 0,
    Khafan = 1,
    Chip = 2,
    Kick = 3
};

enum class DynamicState {
    None = 0,
    Ready = 1,
    Pass = 2,
    Shot = 3
};

class CDynamicPlayOff : public CAbstractPlayOff {
public:
    CDynamicPlayOff();
    ~CDynamicPlayOff() override;
    void reset() override;
    void execute() override;
    void init(const QList<Agent*>& _agents) override;

private:
    int dynamicMatch[_NUM_PLAYERS];
    DynamicSelect dynamicSelect;
    void dynamicPlayKhafan();
    void dynamicPlayChipToGoal(bool isChip);

    void checkEndKhafan();
    void checkEndChipToGoal();

    int dynamicAgentSize;
    DynamicState state;
    unsigned int dynamicStartTime;
    Vector2D dummyPositions[_NUM_PLAYERS];

};


#endif //PARSIAN_AI_DYNAMICPLAYOFF_H
