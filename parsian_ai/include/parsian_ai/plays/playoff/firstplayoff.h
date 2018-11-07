//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_FIRSTPLAYOFF_H
#define PARSIAN_AI_FIRSTPLAYOFF_H

#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <parsian_ai/config.h>
#include <QString>

enum FirstStep {Stay, Move, Done};

enum SHOT_SPOT {
    EveryWhere  = 0b11111111,
    KillSpot    = 0b00000001,
    CloseNear   = 0b00000010,
    CloseCenter = 0b00000100,
    CloseFar    = 0b00001000,
    FarNear     = 0b00010000,
    FarCenter   = 0b00100000,
    FarFar      = 0b01000000
};

class CFirstPlayOff {
public:
    CFirstPlayOff();

    ~CFirstPlayOff();

    void execute();

    void init(const QList<Agent *> &_agents);

    void reset();

    int getShotSpot();

private:

    QList<Agent *> agents;
    int dynamicMatch[_NUM_PLAYERS];
    CRolePlayOff *newRoleAgent[_NUM_PLAYERS];

    void kickoffPositioning(int playersNum);

    void firstExecute();

    void firstPlayForOppCorner(int _agentSize);

    void kickOffStopModePlay(int tagentSize);

    Vector2D kickOffPos[_NUM_PLAYERS];

    bool isFirstFinished();

    void resetFirstPlayFinishedFlag();

    FirstStep firstStepEnums;
    int shotSpot;

    void stayPoisitioning();

    void movePositioning();

    void donePositioning();
};


#endif //PARSIAN_AI_FIRSTPLAYOFF_H
