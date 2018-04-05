#ifndef MASTERPLAY_H
#define MASTERPLAY_H

#include <queue>
#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <QPair>
#include <queue>
#include <regex>

enum PlaysEnum {
    OurKickOffPlay,
    OurIndirectPlay,
    OurDirectPlay,
    OurPenaltyPlay,
    OurBallPlacement,//added

    TheirKickOffPlay,
    TheirIndirectPlay,
    TheirDirectPlay,
    TheirPenaltyPlay,
    TheirBallPlacement,//added
    StartPlay,
    StopPlay,
    HaltPlay,
    HalfTimeLineUp
};

class CMasterPlay {

public:

    bool playOnFlag;
    int playoff_badPasserID;
    bool lockAgents;
    CMasterPlay();
    virtual ~CMasterPlay();
    virtual void reset() = 0;
    virtual void execute_x() = 0;
    virtual void init(const QList <Agent*>& _agents) = 0;

    void execute();
    virtual QString whoami() {
        return "MasterPlay";
    }
    bool canScore();
    QList <Agent *> markAgents;
    //////////////////////////////////////
    bool noPlanException;

    /* static */ PositioningPlan positioningPlan;
    /* static */ CMarkPlan markPlan;
    /* static */ DefensePlan defensePlan;

private:
    void execPlay();
    QString formationName;
    bool staticInited;
    QList< holdingPoints > masterStaticPoints;

protected:

    QList <Agent*> agents;

    int executedCycles;
    int defenseN ;
    bool f;

    QList <Agent *> positionAgents;

    QList <Agent *> stopAgents;
    Agent *playMakeAgent;
    Agent *blockAgent;


    /*static*/ CRolePlayMake playMakeRole;
    /*static*/ CRoleBlock blockRole;

    void initMaster();
    void setAgentsID(const QList <Agent*>& _agentsID);
    void setFormation(QString _formationName);
    void setStaticPoints(QList< holdingPoints > _staticPoints);
    void resetPositioning();
    void resetPlayMaker();
    void appendRemainingsAgents(QList <Agent *> &_list);

    void choosePlayMaker();
    void chooseBlocker();
    bool canOneTouch(QList<Agent*> positionAgents , Agent *playMake);
    double coveredArea(std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar >& obstacles);
    double getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
};

#endif // MASTERPLAY_H
