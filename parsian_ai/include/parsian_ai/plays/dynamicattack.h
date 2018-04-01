#ifndef DYNAMICATTACK_H
#define DYNAMICATTACK_H

#include <parsian_ai/plays/masterplay.h>
#include <ctime>

//#define _MAX_REGION 7

// NEW PASS ZONE
#define KILL_ZONE 5
#define _GOAL_STEP 35
#define _GOAL_WIDTH 1.2
#define ROBOT_RADIUS 0.0890


struct FieldRegion
{
    Rect2D rectangle;
    QList<Vector2D> points;
    int id;

    FieldRegion(){};

    FieldRegion(Rect2D r, QList<Vector2D> p)
    {
        rectangle = r;
        for(auto& point : p)
            points.push_back(point);
    }
};


// END NEW PASS ZONE


namespace AttackAgent{
    struct SPositioningAgent {

        void init(PositionSkill _skill,
                  DynamicRegion _region) {
            skill  = _skill;
            region = _region;
        }

        PositionSkill skill;
        DynamicRegion region;
    };

    struct SPlayMakeAgent {

        void init(PlayMakeSkill _skill,
                  DynamicRegion _region) {
            skill  = _skill;
            region = _region;
        }

        PlayMakeSkill skill;
        DynamicRegion region;
    };
}

struct SDynamicPlan {
    int agentSize;
    DynamicMode mode;
    AttackAgent::SPositioningAgent positionAgents[_NUM_PLAYERS];
    AttackAgent::SPlayMakeAgent playmake;
    Vector2D passPos;
    int passID;
    void reset() {
        agentSize = -1;
        mode = DynamicMode::NoMode;
        passID = -1;
        passPos.invalidate();
    }
    void set(const int& _agentSize,
             const DynamicMode& _mode,
             const PlayMakeSkill& _pm,
             const DynamicRegion& _pmreg,
             const PositionSkill& _ps,
             const DynamicRegion& _reg) {
        agentSize = _agentSize;
        mode = _mode;
        for (auto& p : positionAgents) {p.region = _reg, p.skill = _ps;}
        playmake.region = _pmreg; playmake.skill = _pm;
        passPos.invalidate();
        passID = -1;

    }
};


class CDynamicAttack : public CMasterPlay {

    typedef CMasterPlay super;

public:

    // NEW PASS ZONE
    void chooseBestPositons_new();
    void assignId_new();
    void chooseBestPosForPass_new(QList<Vector2D> semiDynamicPosition);
    void assignTasks_new();
    bool getPMfromCaoch(){return PMfromCoach;};
    int getReceiverID(){return receiver->id();};


    void createRegions(); // splits the opp field into a grid of regions
    Vector2D getBestPosToShootToGoal(Vector2D from, double &regionWidth, bool oppGaol );
    bool isPathClear(Vector2D point, Vector2D from, double rad, bool considerRelaxedIDs);
    Vector2D getEmptyPosOnPoints(Vector2D from, double &regionWidth, QList<Vector2D> points);
    int getNearestOppToPoint(Vector2D point);
    void clearRobotsRegionsWeights();
    int getNearestRegionToRobot(Vector2D agentPos);

    double calcReceiverDistanceFactor(Vector2D point, int passReceiverID, int region_id);
    double calcSenderDistanceFactor(Vector2D passSenderPos, Vector2D point);
    double caclClearPathFactor(Vector2D point, Vector2D passSenderPos, double robot_raduis_new);
    double calcOneTouchAngleFactor(Vector2D point, Vector2D passSenderPos);
    double calcWidenessFactor(Vector2D passSenderPos, Vector2D point);
    void hamidDebug();
    // END NEW PASS ZONE

    CDynamicAttack();
    ~CDynamicAttack() override;

    void execute_x() override;
    void init(const QList <Agent*>& _agents) override;

    void setDefenseClear(bool _isDefenseClearing);
    void setDirectShot(bool _directShot);
    void setPositions(QList<int> _positioningRegion);
    void setWeHaveBall(bool _ballPoss);
    void setNoPlanException(bool _noPlanException);
    void setFast(bool _fast);
    void setPlayMake(Agent* _playMake);
    void setCritical(bool _critical);
    void setBallInOppJaw(bool _ballInOppJaw);
    void choosePlan();
    void swapPlaymakeInPass();
    bool isInpass();

    SDynamicPlan currentPlan;
    SDynamicPlan* nextPlanA;
    SDynamicPlan* nextPlanB;

    Agent* getMahiPlayMaker();

private:
    // NEW PASS ZONE
    FieldRegion regions[3][3];
    QList<int> ourRelaxedIDs, oppRelaxedIDs;
    double robotRegionsWeights[11][9];
    Vector2D bestPointForRobotsInRegions[11][9];
    QList<int> matchingIDs;
    QList<int> matchingRegions;
    QList<QPair<int, Vector2D>> optimalPositionsForRecivers;
    int bestReceiver;
    // END NEW PASS ZONE


    double thrshDribble = 0;
    bool lastPMInitWasDribble;
    Vector2D oppRob;
    double lastYDrib = 0;
    Vector2D lastPassPosLoc;
    int lastGuards[_NUM_PLAYERS];
    int guardSize;
    QTime positioningIntention;
    QTime dribbleIntention;
    QTime playmakeIntention;
    double positioningIntentionInterval;
    bool shotInPass;

    void playMake();
    void positioning(QList <Vector2D> _points);
    void globalExecute(int agentSize);
    void dynamicPlanner(int agentSize);

    void makePlan(int agentSize);
    void assignId();
    void assignTasks();
    ///////////////////////30em 2015

    bool evalmovefwd();
    void swap(Segment2D *xp, Segment2D *yp);
    void sortobstacles(QList<Segment2D> &obstacles);
    double angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp);
    double findmax(const QList<double> &list);

    //[RegionCount][RegionIndex]
    Rect2D* guards[7];
    void showRegions(unsigned int agentSize, QColor color = QColor(Qt::gray));
    void assignRegions();
    void assignRegion_0();
    void assignRegion_1();
    void assignRegion_2();
    void assignRegion_3();
    void assignRegion_4();
    void assignRegion_5();
    void assignRegion_6();
    QList<int> guardIndexList;
    QList<Vector2D> semiDynamicPosition;
    QList<Vector2D> markPositions;

    //[PositionAgentsCount][GuardIndex][LocationIndex]
    Vector2D** guardLocations[7];
    void showLocations(unsigned int agentSize, QColor color = QColor(Qt::gray));
    void assignLocations();
    void assignLocations_0();
    void assignLocations_1();
    void assignLocations_2();
    void assignLocations_3();
    void assignLocations_4();
    void assignLocations_5();
    void assignLocations_6();
    bool isRightTimeToPass();
    int farGuardFromPoint(const int& _guardIndex, const Vector2D& _point);
    void chooseReceiverAndBestPosForPass();
    void chooseBestPositons();
    void chooseMarkPos();
    double getDynamicValue(const Vector2D& _dynamicPos) const;
    void checkPoints(QList<Vector2D>& _points);

    int minHorizontalDistID(const QList<Vector2D>& _points);
    int maxHorizontalDistID(const QList<Vector2D>& _points);

    ///////////////////
    bool isPathClear(Vector2D _pos1, Vector2D _pos2, double rad, double t);

    inline bool chipOrNot(Vector2D target,
                          double _radius = 1, double _treshold = .5);
    int appropriatePassSpeed();
    int appropriateChipSpeed();

    Vector2D neaerstGuardToPoint(const Vector2D& startVec) const;

    void managePasser();
    bool isPlayMakeChanged();

    QString getString(const DynamicMode& _mode) const;

    CRoleDynamic *roleAgents[8];
    CRoleDynamic *roleAgentPM;

    ////////Plan Making
    bool isDefenseClearing, isWeHaveBall, noPlanException;
    bool directShot, fast, critical, ballInOppJaw;
    ////////////////////

    double shotProb, shotAngle;
    bool passFlag, repeatFlag;
    int counter, passerID, lastPasserRoleIndex;
    long lastTime;
    QList<Agent*> mahiPositionAgents;
    QList<Vector2D> dynamicPosition;
    QList<int> regionsList;
    Agent* mahiPlayMaker;
    int mahiAgentsID[8];
    bool isBallInOurField;

    int playmakeID = -1;
    Agent* playmake;

    bool goToDynamic[5];
    int lastPlayMakerId;

    bool keepOrNot();
    bool isPlanDone();
    bool isPlanFailed();
    int lastPassPos;
    Segment2D left;
    Vector2D move_fwd_target;
    Vector2D last_move_fwd_target;
    Agent* receiver;

    QList<int> kianmatchedIDList;
    bool PMfromCoach;
    /////////Intentions
//    int intenHighProb;

protected:
    void reset() override;
};

#endif // DYNAMICATTACK_H
