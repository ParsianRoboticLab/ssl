//
// Created by parsian-ai on 9/22/17.
//

#ifndef PARSIAN_AI_COACH_H
#define PARSIAN_AI_COACH_H

#include <QStringList>

#include <algorithm>

#include <QtCore/QMap>
#include <parsian_util/core/agent.h>
#include <QtCore/QTime>
#include <QtCore/QFile>
#include <QPair>
#include <QTextStream>
#include "parsian_util/base.h"
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/plans/plans.h>
#include <parsian_ai/plays/plays.h>
#include <parsian_ai/roles/stop.h>
#include <parsian_msgs/plan_service.h>
#include <parsian_msgs/parsian_ai_status.h>
#include <parsian_msgs/parsian_pair_roles.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_skill_gotoPointAvoid.h>
#include <parsian_msgs/parsian_skill_gotoPoint.h>
#include <parsian_ai/roles/fault.h>


#include <parsian_util/geom/angle_deg.h>
#include <parsian_util/geom/circle_2d.h>
#include <parsian_util/geom/line_2d.h>
#include <parsian_util/geom/matrix_2d.h>
#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/geom/ray_2d.h>
#include <parsian_util/geom/rect_2d.h>
#include <parsian_util/geom/sector_2d.h>
#include <parsian_util/geom/segment_2d.h>
#include <parsian_util/geom/size_2d.h>
#include <parsian_util/geom/triangle_2d.h>
#include <parsian_util/geom/vector_2d.h>



enum class BallPossesion {
    WEDONTHAVETHEBALL = 0,
    WEHAVETHEBALL = 1,
    SOSOOUR = 2,
    SOSOTHEIR = 3
};

class CCoach {

public:

    explicit CCoach(Agent **_agents);

    ~CCoach();

    void execute();

    void init();

    DefensePlan &getDefense();

    BallPossesion lastBallPossesionState;

    BallPossesion isBallOurs();

    BallPossesion ballPState;

    ////////////////////////////////////////////////////// PLAYOFF PLAN

    void setPlanClient(ros::ServiceClientPtr _plan_client);

    static int findGoalie();
    static bool useGoalieInPlayOff();

    void seperateHealthyAndDamagedRobots();
    QList<int> healthyIDs;
    QList<int> damagedIDs;
    void replaceFaultedRobots();
    CRoleFault *faultRoles[_MAX_NUM_PLAYERS];
    void resetNonVisibleAgents();

private:
    /////////////////////transition to force start
    void checkTransitionToForceStart();
    QList <Vector2D> ballHist;

    double findMostPossible(Vector2D agentPos);
    QList<int> remainingAgent();
    States lastState;
    Agent *goalieAgent;

    QList<Agent *> defenseAgents;
    int preferredDefenseCounts;
    Vector2D defenseTargets[_MAX_NUM_PLAYERS];
    QTime intentionTimePossession;
    QTime playMakeIntention;
    QTime playOnExecTime;

    CMasterPlay *selectedPlay;

    CPlayOff *ourPlayOff;
    COurPenalty *ourPenalty;
    COurPenaltyShootout* ourPenaltyShootout;
    COurBallPlacement *ourBallPlacement;
    CTheirDirect *theirDirect;
    CTheirPenalty *theirPenalty;
    CTheirKickOff *theirKickOff;
    CTheirIndirect *theirIndirect;
    CTheirBallPlacement *theirBallPlacement;
    CDynamicAttack *dynamicAttack;
    CStopPlay *stopPlay;
    CHalftimeLineup *halftimeLineup;
    CSubstitution *substitution;

public:
    CRoleStop *stopRoles[_MAX_NUM_PLAYERS];
private:
    QTime goalieTimer;

    Agent **agents;

    ///////manage over number of agents
    ///
    ///
    QList<int> robotsIdHist;
    bool first;
    QList<int> missMatchIds;
    bool firsttime_forsubstitution;

    ///////////////////////////////////////
    int cyclesWaitAfterballMoved;
    QList<Agent *> lastDefenseAgents;

    void assignGoalieAgent(int goalieID);

    void assignDefenseAgents(int defenseCount);

    void decidePreferredDefenseAgentsCount();

    void decideAttack();

    void decideDefense();

    void decidePlayOn(QList<int> &ourPlayers, QList<int> &lastPlayers);


    QTime defenseTimeForVisionProblem[2];
    double shotToGoalthr;

    void virtualTheirPlayOffState();

    QTime trasientTimeOut;
    int translationTimeOutTime;

    bool isBallcollide(int frameCount = 5, double diffDir = 15);

    ///////////////////////new play make and supporter chooser
    int playmakeId;
    int lastPlayMake;

    int choosePlayMake(const QList<int> &_agentsID);
    void handlePlayMake(const QList<int> &_agentsID);

    ///////////////////////////////////////////////

    enum attackState {
        SAFE     = 0,
        FAST     = 1,
        CRITICAL = 2
    };
    attackState ourAttackState;

    void updateAttackState();

    bool firstTime;

    QList<int> lastPlayers;

    //////////////////////////////////// ALI GAVAHI
    double averageVel;
    QList<Vector2D> lastBallVels;

    void removeLastBallVel(int frameCount = 5);
    void clearBallVels();

    //////////////Decide Attack functions

    void decideHalt(QList<int> &);

    void decideStop(const QList<int> &);

    void decideOurFreeKick(const QList<int> &);

    void decideTheirKickOff(const QList<int> &);

    void decideTheirIndirect(const QList<int> &);

    void decideTheirDirect(const QList<int> &);

    void decideOurPenalty(QList<int> &);

    void decideTheirPenalty(const QList<int> &);

    void decideOurPenaltyshootout(QList<int> &);

    void decideTheirPenaltyshootout(const QList<int> &);

    void decideStart(QList<int> &);

    void decideOurBallPlacement(const QList<int> &);

    void decideTheirBallPlacement(const QList<int> &);

    void decideHalfTimeLineUp(const QList<int> &);

    void decideNull(const QList<int> &);

    /////////////////////////////////////
    QTextStream out;

    int faultDetectionCounter[_MAX_NUM_PLAYERS];

    double kickTimeEstimation(Agent * _agent, const Vector2D& target);
    double timeNeeded(Agent *_agentT,const Vector2D& posT, double vMax);

    NoAction* haltAction;

};
#endif //PARSIAN_AI_COACH_H
