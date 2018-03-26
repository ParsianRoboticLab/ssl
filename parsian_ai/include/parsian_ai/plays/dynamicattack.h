#ifndef DYNAMICATTACK_H
#define DYNAMICATTACK_H

#include <parsian_ai/plays/masterplay.h>

//#define _MAX_REGION 7


struct SDynamicAgent {

    void init(DynamicSkill _skill,
              DynamicRegion _region) {
        skill  = _skill;
        region = _region;
    }

    DynamicSkill skill;
    DynamicRegion region;
};

struct SDynamicPlan {
    int agentSize;
    DynamicMode mode;
    SDynamicAgent positionAgents[6];
    SDynamicAgent playmake;
    Vector2D passPos;
    int passID;
};


class CDynamicAttack : public CMasterPlay {

    typedef CMasterPlay super;

public:


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

    SDynamicPlan currentPlan;

    Agent* getMahiPlayMaker();

private:
    double thrshDribble = 0;
    bool lastPMInitWasDribble;
    Vector2D oppRob;
    double lastYDrib = 0;
    Vector2D lastPassPosLoc;
    int lastGuards[_NUM_PLAYERS];
    int guardSize;
    QTime positioningIntention;
    QTime dribbleIntention;
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
    void chooseBestPosForPass(QList<Vector2D>);
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


    void ballLocation();

    QString getString(const DynamicMode& _mode) const;

    CRoleDynamic *roleAgents[5];
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
    int mahiAgentsID[5];
    bool isBallInOurField;

    int playmakeID = -1;
    Agent* playmake;

    Vector2D ballPos;
    Vector2D ballVel;
    Vector2D OppGoal;

    bool goToDynamic[5];
    int lastPlayMakerId;


    bool keepOrNot();
    int lastPassPos;
    /////////Intentions
//    int intenHighProb;


protected:
    void reset() override;


};

#endif // DYNAMICATTACK_H
