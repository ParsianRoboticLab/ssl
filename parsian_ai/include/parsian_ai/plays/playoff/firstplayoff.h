//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_FIRSTPLAYOFF_H
#define PARSIAN_AI_FIRSTPLAYOFF_H

#include <parsian_ai/plays/playoff/abstractplayoff.h>

enum FirstStep {Stay, Done};

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

class CFirstPlayOff : public CAbstractPlayOff {
public:
    CFirstPlayOff();

    ~CFirstPlayOff() override;

    void execute() override;

    void init(const QList<Agent *> &_agents) override;

    void reset() override;

    int getShotSpot();

    bool isFirstFinished();

private:

    int dynamicMatch[_NUM_PLAYERS];
    CRolePlayOff *roleAgents[_NUM_PLAYERS];

    void firstPlayForOppCorner(int _agentSize);

    void kickOffStopModePlay(int tagentSize);

    Vector2D kickOffPos[_NUM_PLAYERS];

    void resetFirstPlayFinishedFlag();

    FirstStep firstStepEnums;
    int shotSpot;

    void stayPoisitioning();

    void movePositioning();

    void donePositioning();

    int shotBlockers();

    int passBlockers();

    double distAverageOppMark();

    void matchAgent();

    bool firstFinished;
};


#endif //PARSIAN_AI_FIRSTPLAYOFF_H
