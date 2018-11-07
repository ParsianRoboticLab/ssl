//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_STATICPLAYOFF_H
#define PARSIAN_AI_STATICPLAYOFF_H

#include <parsian_msgs/plan_service.h>
#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <parsian_ai/config.h>
#include <QString>

#define BEHIND_BALL_POS Vector2D(1234, 5678)

enum class POFFSKILL {
    None = 0,
    Pass = 1,
    ReceivePass = 2,
    ShotToGoal = 3,
    ChipToGoal = 4,
    OneTouch = 5,
    Move = 6,
    ReceivePassIA = 7,
    //////////// Afterlife Roles
    Defense = 8,
    Support = 9,
    Position = 10,
    Goalie = 11,
    Mark = 12
};

struct playOffSkill {
    POFFSKILL name;
    int data[2];
    int targetIndex;
    int targetAgent;
};

struct playOffRobot {
    Vector2D pos;
    AngleDeg angle;
    double tolerance;
    QList<playOffSkill> skill;
};

struct SPositioningArg {

    Vector2D staticPos{Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE};
    Vector2D staticAng{Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE};
    long rightData{};
    long leftData{};
    double staticEscapeRadius{};
    POFFSKILL staticSkill = POFFSKILL::None;
    int PassToId = -1;
    int PassToState = -1;

    // TODO : Have Receiver Pointer
};

struct SPositioningAgent {

    // TODO : Make positionArg a List of Pointers

    QList<SPositioningArg> positionArg;
    int stateNumber = 0;
    bool zombie = false;

    //////////////Methods
    SPositioningArg getArgs(const int& _state = 0) const {
        if ((_state + stateNumber) < positionArg.size()) {
            return positionArg.at(stateNumber + _state);

        } else {

            DBUG(QString("getArgs : wrong arg %1 < %2").arg(positionArg.size()).arg(_state + stateNumber), D_ERROR);
            return SPositioningArg{};

        }
    }

    SPositioningArg getAbsArgs(const int& _state = 0) const {
        if (_state < positionArg.size()) {
            return positionArg.at(_state);

        } else {

            DBUG(QString("getArgs : wrong absarg %1 < %2").arg(positionArg.size()).arg(_state), D_ERROR);
            return SPositioningArg{};

        }
    }

};

struct SBallOwner {
    int id;
    int state;
};

////Play Off Plans

struct AgentPoint {

    AgentPoint() {
        id    = -1;
        state = -1;
    }

    AgentPoint(int id, int state) {
        this->id    = id;
        this->state = state;
    }

    int id;
    int state;
};

struct SExecution {

    QList< QList<playOffRobot> > AgentPlan;
    int symmetry     =  1;
    int theLastAgent = -1;
    int theLastState = -1;
    int passCount = -1;
    QList<AgentPoint> passer;
    QList<AgentPoint> receiver;
};


struct SPlan {
    struct SInitPos {
        Vector2D ball;
        QList<Vector2D> agents;
    };
    SInitPos initPos;
    int currentSize;
    double lastDist;
    QMap<int, int> matchedID;
    SExecution execution;

};

typedef QPair<AgentPoint, AgentPoint> AgentPair;

class CStaticPlayOff {
public:
    CStaticPlayOff();

    ~CStaticPlayOff();

    void init(const QList<Agent *> &_agents);

    void analyseShoot();

    void analysePass();

    void reset();

    void parsePlan(const parsian_msgs::parsian_plan &_plan);

    void execute();

private:

    QList<Agent *> agents;

    // Critical Play
    void criticalPlay();

    KickAction *criticalKick;
    bool criticalInit;
    bool firstPass;

    SPlan *masterPlan;

    QList<AgentPair> findThePasserandReciver(const SExecution &_plan);

    bool isPathClear(Vector2D _pos1, Vector2D _pos2, double _radius, double threshold);

    Polygon2D getPathPolygon(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold);

    SPositioningAgent positionAgent[_NUM_PLAYERS];

    Vector2D getEmptyTarget(const Vector2D &_position, const double &_radius);

    /////////////////////////////////////////////////////////////////////
    /////////////////////////MAHI PLANNER////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void staticExecute();
    ////////////////////////////Blocker//////////////////////////////////

    bool chipOrNot(const SPositioningArg &_posArg);

    Vector2D getGoalTarget(long _posArg);

    double getMaxVel(const CRolePlayOff *_roleAgent, const SPositioningArg &_posArg);

    Vector2D getMoveTarget(const SPositioningArg &_posArg);

    bool isTaskDone(CRolePlayOff *);

    void passManager();

    bool isFinalShotDone();

    Vector2D lastBallPos;
    unsigned int lastTime;

    QList<Agent *> activeAgents;
    CRolePlayOff *roleAgent[_NUM_PLAYERS];
    CRolePlayOff *tempAgent;


    bool isBallIn;

    bool doPass, doAfterlife;

    //////////////End  Plan
    bool isTimeOver();

    bool isBallDirChanged();

    bool isPlanDone();

    bool isPlanFailed();

    bool setTimer;
    unsigned int startTime;
    ////////////////////////////

    bool isKickDone(CRolePlayOff *);

    bool firstKickFailed();

    bool isOneTouchDone(CRolePlayOff *);

    bool isMoveDone(const CRolePlayOff *);

    bool isReceiveDone(const CRolePlayOff *);

    void assignTasks(const SPlan *_plan);

    void fillRoleProperties();

    void posExecute();

    void checkEndState();

    bool isPlanEnd();

    void assignTask(CRolePlayOff *, const SPositioningAgent &);

    void assignPass(CRolePlayOff *, const SPositioningAgent &);

    void assignMove(CRolePlayOff *, const SPositioningAgent &);

    void assignOneTouch(CRolePlayOff *, const SPositioningAgent &);

    void assignGoalie(CRolePlayOff *, const SPositioningAgent &);

    void assignDefense(CRolePlayOff *, const SPositioningAgent &);

    void assignMark(CRolePlayOff *, const SPositioningAgent &);

    void assignPosition(CRolePlayOff *, const SPositioningAgent &);

    void assignSupport(CRolePlayOff *, const SPositioningAgent &);

    int getIndex(int _planID);

    Agent *getAgent(int _planID);

    void assignKick(CRolePlayOff *, const SPositioningAgent &, bool _chip);

    void assignReceive(CRolePlayOff *, const SPositioningAgent &, bool _ignoreAngle);

    QPair<int, int> findTheLastShoot(const SExecution &_plan);

    int findReceiver(int _passer, int _state);

    QList<SBallOwner> ownerList;
    bool havePassInPlan;

    static void matchPlan(SPlan *_plan, const QList<Agent *> &_ourplayers);

    static POFFSKILL strToEnum(const std::string &_str);

    static void checkGUItoRefineMatch(SPlan *_plan, const QList<Agent *> &_ourplayers);

    static SPlan *planMsgToSPlan(const parsian_msgs::parsian_plan &_plan, int _currSize);

    bool playOnFlag;
};


#endif //PARSIAN_AI_STATICPLAYOFF_H
