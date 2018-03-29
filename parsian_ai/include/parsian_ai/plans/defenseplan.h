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

#define LOOP_TIME_BYKK 0.016
#define MIN_ROBOTS_DIST 0.02
struct velAndAccByKK {
    double vel;
    double acc;
};
enum { OneTouchState , ClearState , NoState };

class DefensePlan : public Plan {
protected:            
    int defenseCount;                  
    GotopointAction* gps[_MAX_NUM_PLAYERS];
    GotopointavoidAction *gpa[_MAX_NUM_PLAYERS];
    KickAction* kickSkill;
    Action* AHZSkills;
    CDefPos defPos;
    Vector2D pointForKick, oneToucherDir;    
    Vector2D goalKeeperTarget,lastTarget, goalieDirection , defensePoints[12], defenseTargets[12];
    void setPointToKick();
    void setGoalKeeperState();
    void setGoalKeeperTargetPoint();
    bool goalKeeperOneTouch, goalKeeperClearMode, ballIsOutOfField, ballIsBesidePoles;
    double strictfollowThr;
    bool dangerForGoalKeeperClear;
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
    int findNeededDefense();
    int defenseNumber();
    double findBestOffsetForDefenseArea(Line2D bestLineWithTalles, double downLimit , double upLimit);
    double findBestRadiusForDefenseArea(Line2D bestLineWithTalles , double downLimit , double upLimit);
    Line2D getBestLineWithTallesForRecatngularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Line2D getBestLineWithTallesForCircularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBestSegmentWithTallesForRectangularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    QList<Segment2D> getLinesOfBallTriangle();
    QList<Vector2D> defenseFormationForRectangularPositioning(int neededDefenseAgents , int allOfDefenseAgents , double downLimit, double upLimit);
    QList<Vector2D> defenseFormationForCircularPositioning(int neededDefenseAgents, int allOfDefenseAgents , double downLimit , double upLimit);
    QList<Vector2D> twoDefenseFormationForRectangularPositioning(double downLimit , double upLimit);
    QList<Vector2D> twoDefenseFormationForCircularPositioning(double downLimit , double upLimit);
    QList<Vector2D> threeDefenseFormationForRecatangularPositioning(double downLimit , double upLimit);
    QList<int> detectOpponentPassOwners(double downEdge , double upEdge);
    Vector2D oneDefenseFormationForRecatngularPositioning(double downLimit , double upLimit);
    Vector2D oneDefenseFormationForCircularPositioning(double downLimit , double upLimit);
    QList<Vector2D> defenseFormation(QList<Vector2D> circularPositions, QList<Vector2D> rectangularPositions);
    //atousa
    Vector2D getGoaliePositionInOneDef(Vector2D _ballPos, double _limit1, double _limit2);
    double goalieThr;
    ////////
    int angleDegreeThrNotStop = 0;
    double besideCounter;
    int lastOpponentAgentsToBeMarkSize;
    double threshOld = 0.0;
    double ballCircleR = 0.5;
    double xLimitForblockingPass;
    double suitableRadius;
    bool isPermissionToKick;
    bool isCrowdedInFrontOfPenaltyAreaByOurAgents;
    bool isCrowdedInFrontOfPenaltyAreaByOppAgents;
    bool ballISInpenaltyAreaAndDangerCircle;
    bool ballIsNotInPenaltyAreaAndIsInDangerCircle;
    bool ballIsInPenaltyAreaAndIsNotInDangerCircle;
    bool dangerForGoalKeeperClearByOurAgents;
    bool dangerForGoalKeeperClearByOppAgents;
    bool dangerForInsideOfThePenaltyArea;
    bool stopMode;
    bool playOffMode;
    bool playOnMode;
    bool dangerModeThresholdForClear;
    bool dangerModeThresholdForDanger;
    bool manToManMarkBlockPassFlag;
    bool goalKeeperPredictionModeInPlayOff;
    QList <QString> markRoles;
    QList <QString> lastMarkRoles;
    Vector2D opponentPasserDirection;
    Vector2D tempBallRectanglePoint;
    Vector2D oppNearestToBallPossition;
    Vector2D tempAHZ;
    QString lastStateForGoalKeeper;
    QList<Vector2D> AHZDefPoints;
    ///////////////////////////////////////////////////
    void executeGoalKeeper();
    Vector2D strictFollowBall(Vector2D _ballPos);
    Vector2D checkDefensePoint(Agent* agent, const Vector2D& point);    
    int decideNumOfMarks();
    kkDefPos tempDefPos;
    void matchingDefPos(int _defenseNum);
    bool defenseOneTouchOrNot();        
    bool agentEffectOnBallProbability(Vector2D ballPos, Vector2D ballVel, Vector2D agentPos, Vector2D agentVel, bool isTowardOurgoal);
    Vector2D getGoalieShootOutTarget(bool isSkyDive);
    bool canReachToBall(int agentId, int theirAgentId);
    int decideShootOutMode();
    QList <Vector2D> lastBallPos;
    int penaltyShootoutMode = beforeTouch;
    void penaltyShootOutMode();
    GotopointavoidAction* striker_Robot;
    void penaltyMode();
    enum exepMode {
        defOneTouch = 1,
        defClear = 2,
        NoneExep = 3
    };
    enum shootOutMode {
        beforeTouch,
        shootOutClear,
        ballBisector,
        skyDive

    };

    bool shootOutClearModeSelected = false;
    bool agentEffectOnBallProbabilityRes;
    double shootOutDiam = 2.5;

    struct defenseExeptions {
        bool active;
        exepMode exeptionMode;
        int exepAgentId;
    };
    Vector2D ballPrediction(bool _isGoalie);

    Vector2D lastBallPosition;
public:
    DefensePlan();
    void execute() override;
    void initGoalKeeper(Agent *_goalieAgent = NULL);
    void initDefense(QList <Agent*> _defenseAgents = QList<Agent*>());
    void fillDefencePositionsTo(Vector2D *poses);

    //////////////////HMD/////////////////
    QList<Vector2D> markPoses;
    QList<Vector2D> markAngs;
    double markRadius;
    double markRadiusStrict;
    double segmentpershoot;
    double segmentperpass;
    bool MantoManAllTransientFlag;
    Vector2D dir;
    ///////////////////////////////////


private:
    ///////////////////////HMD///////////////
    void findPos(int _markAgentSize);
    void findOppAgentsToMark();
    bool isInTheIndirectAreaShoot(Vector2D);
    bool isInTheIndirectAreaPass(Vector2D);
    QList<Vector2D> ShootBlockRatio(double, Vector2D);
    QList<Vector2D> PassBlockRatio(double, Vector2D);
    QList<Vector2D> indirectAvoidShoot(Vector2D);
    QList<Vector2D> indirectAvoidPass(Vector2D);
    int numberOfMarkers;
    QList<Vector2D> oppAgentsToMarkPos;
    QList<Vector2D> oppmarkedpos;
    QList<CRobot*>  oppAgentsToMark;
    Vector2D posvel(CRobot*, double);
    QList<QPair<Vector2D, double> > sortdangerpassplayon(QList<Vector2D> oppposdanger);
    QList<QPair<Vector2D, double> > sortdangerpassplayoff(QList<Vector2D> oppposdanger);
    ////////////////////////////////////////
    rcsc::Circle2D defenseAreaBottomCircle, defenseAreaTopCircle;
    rcsc::Segment2D defenseAreaLine;
    rcsc::Vector2D* getIntersectWithDefenseArea(const Line2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Segment2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Circle2D& circle, bool upperPoint);
    void assignSkill(Agent *_agent , Action *_skill);    
    void initVars(float goalCircleRad = 0.9); // default is 0.8    
    bool defenderForMark;
    bool doubleMarking;
    bool isDefenseFastest;
    bool clearflag;
    Agent *goalKeeperAgent;
    QList <Agent *> defenseAgents;
    int oneDefUpOrDown;
    int twoDefCurState;
    int lastStateOffPlay;
    int lastMarker[10];
    int markPointNum;
    int oneToucher;
    Vector2D defenseDirs[_MAX_NUM_PLAYERS];
    Vector2D ballPos;
    bool doOneTouch;
    bool doClear;
    int lastClearID;
    int defenseClearIndex;
    double lastClearDist;
    int clearFrameCnt;
    int lastTouchTheGoalie;
    bool distClearHysteresis;
    int lastOneTouchClearState;
    int histOneTouchClearCnt;
    double GOTThresh;
    int GOTCounter;
    double thr;
    double noDefThr;
    QList<Vector2D> ballPosHistory;    
    void calcPointForOneTouch();    
    bool isInOneTouch;
    bool isOnetouch;
    int oneTouchCycleTest;
    bool checkStillBeingInOneTouch();
    int cycleCounter;
    Vector2D oneTouchPoint[2];
    bool oneTouchPointFlag;
    bool oneTouchPointFlagG;        
    bool doBlockPass;
    double timeToReach;
    Vector2D blockPassPoint;
    QList<int> dangerousOpp;
    double goalieAreaHis;
    Vector2D goalieTargetDir;    
    int isBallGoingToOppAreaCnt;
    double pushBallHist;
    int failureAtempCnt;
    int clearCnt;
    double savedClearDist;
    int goaliePassBlockCnt;
    Vector2D gBassBlockTargetSave;
    double predictThresh;    
    bool inPenaltyAreaFlag;
    int predictMostDangrousOppToBall();
    Vector2D NearestDistanceToBallSegment(Vector2D point);
    kkDefPos defPosDecision;
    defenseExeptions defExceptions;
    void checkDefenseExeptions();
    void runDefenseExeptions();
    Vector2D runDefenseOneTouch();
    double defClearThr;
    bool defenseCheckBallDangerForOneTouch();
    bool defClearFlag;
    double overDefThr;
    int decideNumOfMarksInPlayOff(int _defenseCount);
    bool FlagBesidePoles;
    int f = 0;
};

#endif // DEFENSE_H
