#ifndef DEFENSE_H
#define DEFENSE_H

#include <cmath>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/plans/plan.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_ai/util/defpos.h>
#include <QList>
#include <parsian_ai/gamestate.h>
#include <parsian_ai/config.h>
#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/action/autogenerate/noaction.h>
#include "parsian_util/tools/blackboard.h"
#include "parsian_util/geom/polygon_2d.h"
#define LOOP_TIME_BYKK 0.016
#define MIN_TWO_ROBOTS_DIST 0.02
#define MIN_MORE_ROBOTS_DIST 0.05

enum class shootOutMode {
    beforeTouch,
    shootOutClear,
    ballBisector,
    skyDive

};

enum class GKState{
    GKReciveBallInTS,
    GKPredictInTs,
    playoff,
    Stop,
    ballIsOutOfField,
    ballIsBesidePoles,
    clearMode,
    clearSlowBall,
    oneTouch,
    dangerForClear,
    strictFollow
};


class DefensePlan : public Plan {
protected:
    int defenseCount;

    GotopointAction* gps[_MAX_NUM_PLAYERS];
    GotopointavoidAction *gpa[_MAX_NUM_PLAYERS];
    KickAction* kickSkill;
    Action* AHZSkills;
    Vector2D pointForKick, oneToucherDir;
    Vector2D goalKeeperTarget , defensePoints[12], defenseTargets[12];
    void setPointToKick();
    GKState setGoalKeeperState();
    Vector2D setGoalKeeperTargetPoint(GKState);
    double differentialTime = 0;
    int oneTouchCnt;
    ////////////////////////////// AHZ ///////////////////
    Line2D getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    void manToManMarkBlockPassInPlayOff(QList<Vector2D> opponentAgentsToBeMarkePossition , int ourMarkAgentsSize , double proportionOfDistance);
    void manToManMarkBlockShotInPlayOff(int _markAgentSize);

    bool areAgentsStuckTogether(const QList<Vector2D> &agentsPosition);
    void agentsStuckTogether(const QList<Vector2D> &agentsPosition , QList<Vector2D> &stuckPositions , QList<int> &stuckIndexs);
    void correctingTheAgentsAreStuckTogether(QList<Vector2D> &agentsPosition, QList<Vector2D> &stuckPositions , QList<int> &stuckIndexs);

    bool isInIndirectArea(Vector2D);
    int defenseNumber();
    QList<Vector2D> getPositionJustForZJU(int numberOfOverDefenders);
    double findBestOffsetForDefenseArea(Line2D bestLineWithTalles, double downLimit , double upLimit);
    double findBestRadiusForDefenseArea(Line2D bestLineWithTalles , double downLimit , double upLimit);
    Line2D getBestLineWithTallesForRecatngularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Line2D getBestLineWithTallesForCircularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBestSegmentWithTallesForRectangularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBestSegmentWithTallesForCircularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    QList<Segment2D> getLinesOfBallTriangle();
    QList<Vector2D> defenseFormationForRectangularPositioning(int neededDefenseAgents , int allOfDefenseAgents , double downLimit, double upLimit);
    QList<Vector2D> defenseFormationForCircularPositioning(int neededDefenseAgents, int allOfDefenseAgents , double downLimit , double upLimit);
    QList<Vector2D> twoDefenseFormationForRectangularPositioning(double downLimit , double upLimit);
    QList<Vector2D> twoDefenseFormationForCircularPositioning(double downLimit , double upLimit);
    QList<Vector2D> threeDefenseFormationForRecatangularPositioning(double downLimit , double upLimit);
    QList<Vector2D> threeDefenseFormationForCircularPositioning(double downLimit , double upLimit);
    QList<int> detectOpponentPassOwners(double downEdge , double upEdge);
    Vector2D oneDefenseFormationForRecatngularPositioning(double downLimit , double upLimit);
    Vector2D oneDefenseFormationForCircularPositioning(double downLimit , double upLimit);
    Vector2D getGKPositionInOneDefense(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint , double downLimit , double upLimit);
    Vector2D getGKPositionWithoutDefense(double downLimit , double upLimit);
    Vector2D getGKPositionAccordingToTheDefense(int numberOfDefenders , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Vector2D getGKPositionInMoreThanTwoDefense(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint , double downLimit , double upLimit);
    Line2D getBestLineWithTallesForGK(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    QList<Vector2D> defenseFormation(QList<Vector2D> circularPositions, QList<Vector2D> rectangularPositions);
    double timeNeeded(Agent *_agentT, Vector2D posT, double vMax, QList <int> _ourRelax, QList <int> _oppRelax , bool avoidPenalty, double ballObstacleReduce, bool _noAvoid);    
    double findBestRadiusForGK(Line2D bestLineWithTalles ,Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint , double downLimit , double upLimit);
    /// \brief angleDegreeThrNotStop
    Vector2D lastTargetForStrictFollow;
    double AHZDegThreshOld = 0;
    double ballCircleR = 0.5;
    double xLimitForblockingPass;
    double suitableRadius;
    bool manToManMarkBlockPassFlag;
    QList <QString> markRoles;
    QList <QString> lastMarkRoles;
    QString lastStateForGoalKeeper;
    QList<Vector2D> AHZDefPoints;
    ///////////////////////////////////////////////////
    Vector2D strictFollowBall(Vector2D _ballPos);
    Vector2D avoidCircularPenaltyAreaByMasoud(Agent* agent, const Vector2D& point);
    int decideNumOfMarks();
    void matchingDefPos(int _defenseNum);
    bool defenseOneTouchOrNot();
    enum exepMode {
        defOneTouch = 1,
        defClear = 2,
        NoneExep = 3
    };


    bool shootOutClearModeSelected = false;
    bool agentEffectOnBallProbabilityRes;
    double shootOutDiam = 2.5;

    struct defenseExeptions {
        bool active;
        exepMode exeptionMode;
        int exepAgentId;
    };

public:
    DefensePlan();
    void execute() override;
    void initGoalKeeper(Agent *_goalieAgent = NULL);
    void initDefense(QList <Agent*> _defenseAgents = QList<Agent*>());
    void fillDefencePositionsTo(Vector2D *poses);
    ////////////////////// AHZ ////////////////
    int findNeededDefense();
    //////////////////HMD/////////////////
    QList<Vector2D> markPoses;
    QList<Vector2D> markAngs;
    double segmentpershoot;
    double segmentperpass;
    Vector2D dir;
    /// ALI GAVAHI
    bool ballIsBounced;
    Vector2D ballBouncePos, playOffStartBallPos, playOffPassDir,beforeTransientPassDir;
    ///////////////////////////////////
private:
    ///////////////////////Lhum checked them//////////////
    void drawGameState();
    void penaltyMode();
    bool canReachToBall(const int& agentId, const int& theirAgentId);
    void penaltyShootOutMode();
    Vector2D getGoalieShootOutTarget(bool isSkyDive);
    bool agentEffectOnBallProbability(const Vector2D& agentPos);
    shootOutMode decideShootOutMode();
    bool dangerForGK();
    Vector2D movePointToPenaltyArea(const Vector2D&);
    Vector2D ballIsBesidePoles();
    void executeGoalKeeper(const Vector2D& , const GKState&);
    int stateBallBesidepoles;
    ///////////////////////HMD///////////////
    Vector2D ballPrediction(const bool);
    void findPos(int _markAgentSize);
    void findOppAgentsToMark();
    bool isInTheIndirectAreaShoot(Vector2D);
    bool isInTheIndirectAreaPass(Vector2D);
    QList<Vector2D> ShootBlockRatio(double, Vector2D);
    QList<Vector2D> PassBlockRatio(double, Vector2D);
    QList<Vector2D> indirectAvoidShoot(Vector2D);
    QList<Vector2D> indirectAvoidPass(Vector2D);
    QList<Vector2D> oppAgentsToMarkPos;
    QList<Vector2D> oppmarkedpos;
    QList<CRobot*>  oppAgentsToMark;
    Vector2D posvel(CRobot*, double);
    QList<QPair<Vector2D, double> > sortdangerpassplayon(QList<Vector2D> oppposdanger);
    QList<QPair<Vector2D, double> > sortdangerpassplayoff(QList<Vector2D> oppposdanger);
    Vector2D getMarkPlayoffPredictWaitPos();
    ////////////////////////////////////////
    rcsc::Circle2D defenseAreaBottomCircle, defenseAreaTopCircle;
    rcsc::Segment2D defenseAreaLine;
    rcsc::Vector2D* getIntersectWithDefenseArea(const Line2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Segment2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Circle2D& circle, bool upperPoint);
    void assignSkill(Agent *_agent , Action *_skill);
    Agent *goalKeeperAgent;
    QList <Agent *> defenseAgents;
    int lastMarker[10];
    int oneToucher;
    Vector2D defenseDirs[_MAX_NUM_PLAYERS];
    bool doOneTouch;
    double thr;
    void calcPointForOneTouch();
    bool isInOneTouch;
    int oneTouchCycleTest;
    bool checkStillBeingInOneTouch();
    int cycleCounter;
    bool oneTouchPointFlag;
    int predictMostDangrousOppToBall();
    Vector2D NearestDistanceToBallSegment(Vector2D point);
    defenseExeptions defExceptions;
    void checkDefenseExeptions();
    void runDefenseExeptions();
    Vector2D runDefenseOneTouch();
    bool defenseCheckBallDangerForOneTouch();
    int counterBallWasBesidePoles = 0;

};//tavabei ke vabaste be vorodi and static she
// const &
// moteghayer ha kam she

#endif // DEFENSE_H
