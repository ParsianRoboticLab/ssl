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
#include <behavior/mahi/mahi.h>
#include <behavior/direct/direct.h>
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

    bool inited;
    double playOnTime;

    explicit CCoach(Agent **_agents);

    ~CCoach();

    void execute();

    DefensePlan &getDefense();

    BallPossesion lastBallPossesionState;

    BallPossesion isBallOurs();

    BallPossesion ballPState;

    ////////////////////////////////////////////////////// PLAYOFF PLAN
    parsian_msgs::plan_serviceRequest planRequest;
    parsian_msgs::plan_serviceResponse receivedPlan;

    ros::Publisher *ai_status_pub;
    ros::ServiceClient plan_client;

    bool gotplan;

    void setPlanClient(const ros::ServiceClient &_plan_client);

    void setBehaviorPublisher(ros::Publisher &_behaver_publisher);

    int findGoalie();

    parsian_msgs::plan_serviceResponse getLastPlan();

    void updateBehavior(const parsian_msgs::parsian_behaviorConstPtr _behav);

    void generateWorkingRobotIds();
    QList<int> workingIDs;
    void replacefaultedrobots();
    CRoleFault *faultRoles[_MAX_NUM_PLAYERS];
    void resetnonVisibleAgents();




private:
    /////////////////////transition to force start
    void checkTransitionToForceStart();
    QList <Vector2D> ballHist;

    double findMostPossible(Vector2D agentPos);

    States lastState;
    Agent *goalieAgent;

    QList<Agent *> defenseAgents;
    int preferedDefenseCounts, lastPreferredDefenseCounts;
    int preferedGoalieID;
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

    Behavior *selectedBehavior;

    BehaviorMahi *behaviorMahi;

public:
    CRoleStop *stopRoles[_MAX_NUM_PLAYERS];
private:
    QTime goalieTimer;
    bool goalieTrappedUnderGoalNet;

    Agent **agents;
    int lastAssignCycle;

    void checkRoleAssignments();

    ///////manage over number of agents
    ///
    ///
    QList<int> robotsIdHist;
    bool first;
    QList<int> missMatchIds;

    ///////////////////////////////////////
    int cyclesWaitAfterballMoved;
    QList<Agent *> lastDefenseAgents;

    void matchPlan(NGameOff::SPlan *_plan, const QList<int> &_ourplayers);
    void getBadsAndGoods(const QList<int>& _ourplayers);
    QList<int> badshooters;
    QList<int> goodshooters;

    NGameOff::SPlan *planMsgToSPlan(parsian_msgs::plan_serviceResponse planMsg, int _currSize);

    void assignGoalieAgent(int goalieID);

    void assignDefenseAgents(int defenseCount);

    void checkGoalieInsight();

    void decidePreferredDefenseAgentsCount();

    void decideAttack();

    void decideDefense();

    void decidePlayOff(QList<int> &_ourPlayers, POMODE _mode = INDIRECT);

    void decidePlayOn(QList<int> &ourPlayers, QList<int> &lastPlayers);


    QTime defenseTimeForVisionProblem[2];
    double shotToGoalthr;

    void virtualTheirPlayOffState();

    QTime trasientTimeOut;
    int translationTimeOutTime;

    bool isBallcollide(int frameCount = 5, double diffDir = 15);

    void calcDesiredMarkCounts(); // not used at all
    ///////////////////////new play make and supporter chooser
    int playmakeId;
    int supporterId;
    int lastSupporterId;
    int lastPlayMake;

    void choosePlaymakeAndSupporter(bool defenseFirst);

    ///////////////////////////////////////////////

    enum attackState {
        SAFE     = 0,
        FAST     = 1,
        CRITICAL = 2
    };
    attackState ourAttackState;

    void updateAttackState();


    ///////////////////////New Play Off
    void selectPlayOffMode(int agentSize, NGameOff::EMode &_mode);

    void initPlayOffMode(NGameOff::EMode _mode,
                         POMODE _gameMode,
                         const QList<int> &_agentSize);

    void setPlayOff(NGameOff::EMode _mode);

    void initStaticPlay(POMODE _mode, const QList<int> &_agentSize);

    void initDynamicPlay(const QList<int> &_ourplayers);

    void initFastPlay(const QList<int> &_ourplayers);

    void initFirstPlay(const QList<int> &_ourplayers);

    void setStaticPlay();

    void setDynamicPlay();

    void setFirstPlay();

    void setFastPlay();

    bool firstTime, firstPlay, firstIsFinished;

    void checkGUItoRefineMatch(SPlan *_plan, const QList<int> &_ourplayers);


    int preferedShotSpot;

    QList<int> lastPlayers;

    //////////////////////////////////// ALI GAVAHI
    double lastNearestBallDist;
    double averageVel;
    QList<Vector2D> lastBallVels;
    Vector2D startTransientBallPos;

    void removeLastBallVel(int frameCount = 5);
    void clearBallVels();

    //////////////Decide Attack functions

    void decideHalt(QList<int> &);

    void decideStop(QList<int> &);

    void decideOurKickOff(QList<int> &);

    void decideTheirKickOff(QList<int> &);

    void decideOurIndirect(QList<int> &);

    void decideTheirIndirect(QList<int> &);

    void decideOurDirect(QList<int> &);

    void decideTheirDirect(QList<int> &);

    void decideOurPenalty(QList<int> &);

    void decideTheirPenalty(QList<int> &);

    void decideOurPenaltyshootout(QList<int> &);

    void decideTheirPenaltyshootout(QList<int> &);

    void decideStart(QList<int> &);

    void decideOurBallPlacement(QList<int> &);

    void decideTheirBallPlacement(QList<int> &);

    void decideHalfTimeLineUp(QList<int> &);

    void decideNull(QList<int> &);

    /////////////////////////////////////
    QTextStream out;

    bool isFastPlay();

    double overDefThr;

    // inter change
    void checkSensorShootFault();

    int faultDetectionCounter[_MAX_NUM_PLAYERS];

    double kickTimeEstimation(Agent * _agent, const Vector2D& target);
    double timeNeeded(Agent *_agentT,const Vector2D& posT, double vMax);
    // MAHI ADD IN ROS
    QList<CRobot *> toBeMopps;
    int desiredDefCount;
    QString stateForMark;
    QPair<int, parsian_msgs::parsian_robot_task>** defenseMatched[2];
    parsian_msgs::parsian_behaviorConstPtr m_behavior;

    POffSkills strToEnum(const std::string &_str);

    void sendBehaviorStatus();

    parsian_msgs::parsian_ai_statusPtr fillAIStatus();

    void findDefneders(const int &max_number, const int& min_number);
    NoAction* haltAction;

//    QList<Vector2D> lastBallVel;
    QList<Vector2D> lastBallDir;

};
#endif //PARSIAN_AI_COACH_H
