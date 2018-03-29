#ifndef PLAYOFF_H
#define PLAYOFF_H

#include "parsian_ai/plays/masterplay.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <QMessageBox>

#define POBALLPOS Vector2D(1234, 8456)

enum POffSkills {
    NoSkill = 0,
    PassSkill = 1,
    ReceivePassSkill = 2,
    ShotToGoalSkill = 3,
    ChipToGoalSkill = 4,
    OneTouchSkill = 5,
    MoveSkill = 6,
    ReceivePassIASkill = 7,
    //////////// Afterlife Roles
    Defense = 8,
    Support = 9,
    Position = 10,
    Goalie = 11,
    Mark = 12
};

enum ShotSpot {
    EveryWhere  = 0b11111111,
    KillSpot    = 0b00000001,
    CloseNear   = 0b00000010,
    CloseCenter = 0b00000100,
    CloseFar    = 0b00001000,
    FarNear     = 0b00010000,
    FarCenter   = 0b00100000,
    FarFar      = 0b01000000
};

struct playOffSkill {
    POffSkills name;
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

struct POInitPos {
    Vector2D ball;
    Vector2D Agent[_NUM_PLAYERS];
};

enum POMODE {
    DIRECT   = 1,
    INDIRECT = 2,
    KICKOFF  = 3
};

enum DynamicSelect {
    NOSELECT = 0,
    KHAFAN = 1,
    CHIP = 2,
    BLOCKER = 3
};

struct STuneParams {
    double lastDist = 0.5;
};

struct SPlanConfig {
    int         chance = 1;
    QString     name   = "mahi";
    STuneParams tuneParams;
};

struct SPlayOffPlan {

    SPlayOffPlan() = default;

    SPlayOffPlan(const SPlayOffPlan &_toCopy) {
        this->planMode  = _toCopy.planMode ;
        this->agentSize = _toCopy.agentSize;
        this->initPos   = _toCopy.initPos  ;
        this->config    = _toCopy.config   ;
        for (int i = 0; i < _NUM_PLAYERS; i++) {
            this->AgentPlan[i].clear();
            for (int j = 0; j < _toCopy.AgentPlan[i].size(); j++) {
                this->AgentPlan[i].append(_toCopy.AgentPlan[i].at(j));
            }
        }
    }

    SPlayOffPlan& operator =(const SPlayOffPlan &_insert) {
        this->planMode  = _insert.planMode ;
        this->agentSize = _insert.agentSize;
        this->initPos   = _insert.initPos  ;
        this->config    = _insert.config   ;
        for (int i = 0; i < _NUM_PLAYERS; i++) {
            this->AgentPlan[i].clear();
            for (int j = 0; j < _insert.AgentPlan[i].size(); j++) {
                this->AgentPlan[i].append(_insert.AgentPlan[i].at(j));
            }
        }
    }

    QList<playOffRobot> AgentPlan[_NUM_PLAYERS];
    POMODE planMode;
    int agentSize{};
    POInitPos initPos;
    SPlanConfig config;
    QString tags;
};

struct SPositioningArg {

    Vector2D staticPos;
    Vector2D staticAng;
    long rightData;
    long leftData;
    double staticEscapeRadius;
    POffSkills staticSkill;
    int PassToId = -1;
    int PassToState = -1;

    // TODO : Have Reciver Pointer
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
            qWarning() << QString("getArgs : wrong arg %1 < %2").arg(positionArg.size()).arg(_state + stateNumber);
            SPositioningArg null;
            return null;

        }
    }

    SPositioningArg getAbsArgs(const int& _state = 0) const {
        if (_state < positionArg.size()) {
            return positionArg.at(_state);

        } else {

            DBUG(QString("getArgs : wrong absarg %1 < %2").arg(positionArg.size()).arg(_state), D_ERROR);
            SPositioningArg null;
            return null;

        }
    }

};

struct POOwnerReceive {
    int ballOwnerAgent;
    int ballOwnerIndex;
    int receiveAgent;
    int receiveIndex;
    POffSkills skill;
};

struct SBallOwner {
    int id;
    int state;
};

////Play Off Plans
namespace NGameOff {

enum EType {
    None   = 0,
    File   = 1,
    Link   = 2,
    SubDir = 3
};

enum EMode {
    FirstPlay   = 0,
    FastPlay    = 1,
    StaticPlay  = 2,
    DynamicPlay = 3
};

struct SFail {
    bool fail;
    int agentID, roleID, planID, taskID;
    EMode mode;
    RoleSkill skill;
    int succesRate;
};

struct SCommon {
    int agentSize;
    int currentSize;
    double chance;
    double lastDist;
    POMODE planMode;
    QStringList tags;
    int succesRate = 0; // {},{},{},{},{},{},{}
    QMap<int, int> matchedID;
    int planRepeat;
    void addHistory(const int _story) {
        int tempSucces = _story - succesRate;
        history.append(_story);
        succesRate += tempSucces / history.size();
    }

private:
    QList<int> history;

};

struct SMatching {
    struct SInitPos {
        Vector2D ball;
        QList<Vector2D> agents;
    };
    SInitPos initPos;
    SCommon *common;
    Vector2D shotPos;
};

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
    SCommon *common;
    int symmetry     =  1;
    int theLastAgent = -1;
    int theLastState = -1;
    int passCount;
    AgentPoint passer;
    AgentPoint reciver;
};

struct SGUI {

    QString name;
    QString planFile;
    QString package;
    bool active = true;
    bool master = false;
    unsigned int index[3]; // {package_index, file_index, plan_index}
    SCommon *common;
};


struct SPlan {

    SPlan() {
        gui.common       = &common;
        matching.common  = &common;
        execution.common = &common;
    }

    SGUI       gui;
    SCommon    common;
    SMatching  matching;
    SExecution execution;

    //    friend QDebug operator<< (QDebug d, const SPlan plan);
};

}

typedef QPair<NGameOff::AgentPoint, NGameOff::AgentPoint> AgentPair;

using namespace NGameOff;

enum FirstStep {Stay, Move, Done};
enum BlockerSteps {S0, S1, S2, S3};

class CPlayOff : public CMasterPlay {

public:
    CPlayOff();
    virtual ~CPlayOff();

    void execute_x();
    void init(const QList <Agent*>& _agents);
    virtual QString whoami() {
        return "PlayOff";
    }
    bool deleted;

    void setMasterPlan(SPlan* _thePlan);
    void analyseShoot();
    void analysePass();


    void setMasterMode(EMode _mode);
    EMode getMasterMode();
    void reset();
    void setInitial(bool _init);
private:


    // Critical Play
    bool criticalPlay();
    KickAction* criticalKick;
    bool criticalInit;

    bool initial;

    SPlan* masterPlan;
    EMode masterMode;

    void globalExecute();
    bool isBlockDisturbing();

    int BlockerStopperID;

    bool isPathClear(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold);

    SPositioningAgent positionAgent[_NUM_PLAYERS];

    int agentSize;
    Vector2D getEmptyTarget(Vector2D _position, double _radius);
    QList<POOwnerReceive> ownerReceiveList;

    /////////////////////////////////////////////////////////////////////
    /////////////////////////MAHI PLANNER////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void mainExecute();
    void staticExecute();
    void dynamicExecute();
    void fastExecute();
    void firstExecute();
    void kickOffStopModePlay(int tagentSize);
    void firstPlayForOppCorner(int _agentSize);


    ////////////////////////////Blocker//////////////////////////////////
    bool BlockerExecute(int agentID);
    Agent* BlockerAgent;
    GotopointavoidAction* blockergpa;
    enum BlockerStop {
        Diversion,
        BlockStop,
        TurnAndKick
    };
    BlockerStop blockerStopStates;


    POMODE getPlayOffMode();
    void getCostRec(double costArr[][_NUM_PLAYERS], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId = 0);
    int kkGetIndex(kkValue &value, int cIndex);
    bool chipOrNot(int passerID, int ReceiverID, int ReceiverState);
    bool chipOrNot(const SPositioningArg& _posArg);
    Vector2D getGoalTarget(int shoterID, int shoterState);
    Vector2D getGoalTarget(const long& _posArg);
    double getMaxVel(int agentID, int agentState);
    double getMaxVel(const CRolePlayOff* _roleAgent, const SPositioningArg& _posArg);
    Vector2D getMoveTarget(int agentID, int agentState);
    Vector2D getMoveTarget(const SPositioningArg& _posArg);
    /// After life points
    Vector2D getMarkTarget(const SPositioningArg&);
    Vector2D getDefenseTarget(const SPositioningArg&);
    Vector2D getSupportTarget(const SPositioningArg&);
    ///

    bool isTaskDone(CRolePlayOff*);
    void passManager();
    /////////////////////////////////////
    void oneBehindBall();
    void oneLeftOneCentre();
    void oneRightOneCentre();
    void twoSidesOneCentre();
    void twoSideOneCentreOneDef();
    void twoSideOneCentreTwoDef();
    void twoSideOneCentreTwoDefAndGoalie();
    ////////////////////////////////////
    bool isFinalShotDone();

    Vector2D lastBallPos;
    long lastTime;
    bool decidePlan;
    int kkAgentsID[_NUM_PLAYERS];

    SPlayOffPlan* currentPlan;
    QList<Agent*> activeAgents;
    CRolePlayOff *roleAgent[_NUM_PLAYERS];
    CRolePlayOff *tempAgent;
    CRolePlayOff *newRoleAgent[_NUM_PLAYERS];
    enum BlockerDetector {
        penaltyAreaBlock   = 0b001,
        centralRegionBlock = 0b010,
        RoundRegionBlock   = 0b100
    };

    int blockerState;
    int blockerID;
    QList<int> blockersPenaltyArea;
    QList<int> blockersCentralRegion;
    QList<int> blockersRoundRegion;
    Vector2D kickOffPos[_NUM_PLAYERS];

    bool isBallIn;
    double debugs[10];
    Vector2D draws[10];
    Circle2D circles[10];

    void mahiDebug(int limit);
    void mahiCircle(int limit);
    void mahiVector(int limit);

    bool doPass, doAfterlife;

    //////////////End  Plan
    bool isBallPushed();
    bool isTimeOver();
    bool isBallDirChanged();
    SFail isAnyTaskFaild();
    bool isAllTasksDone();
    bool isPlanDone();
    bool isPlanFaild();
    bool setTimer;
    unsigned int tempStart;
    ////////////////////////////

    bool isKickDone(CRolePlayOff*);
    bool isOneTouchDone(CRolePlayOff*);
    bool isMoveDone(const CRolePlayOff*);
    bool isReceiveDone(const CRolePlayOff*);
    void assignTasks();
    void fillRoleProperties();
    void posExecute();
    void checkEndState();
    bool isPlanEnd();
    void assignTask(CRolePlayOff*, const SPositioningAgent&);
    void assignPass(CRolePlayOff*, const SPositioningAgent&);
    void assignMove(CRolePlayOff*, const SPositioningAgent&);
    void assignOneTouch(CRolePlayOff*, const SPositioningAgent&);
    void assignGoalie(CRolePlayOff*, const SPositioningAgent&);
    void assignDefense(CRolePlayOff*, const SPositioningAgent&);
    void assignMark(CRolePlayOff*, const SPositioningAgent&);
    void assignPosition(CRolePlayOff*, const SPositioningAgent&);
    void assignSupport(CRolePlayOff*, const SPositioningAgent&);

    void assignKick(CRolePlayOff*, const SPositioningAgent&, bool _chip);
    void assignReceive(CRolePlayOff*, const SPositioningAgent&, bool _ignoreAngle);
    QPair<int, int> findTheLastShoot(const SExecution& _plan);
    void findThePasserandReciver(const SExecution&, QList<AgentPair> &_pairList);
    int findReceiver(int _passer, int _state);
    QList<SBallOwner> ownerList;
    bool havePassInPlan;
    ////Dynamic
public:
    int dynamicMatch[_NUM_PLAYERS];
    DynamicSelect dynamicSelect;
private:
    void dynamicAssignID();
    void dynamicPlayKhafan();
    void dynamicPlayBlocker();
    void dynamicPlayChipToGoal();

    void checkEndKhafan();
    void checkEndBlocker();
    void checkEndChipToGoal();
    Vector2D getDynamicTarget(int i);

    int dynamicAgentSize;
    bool ready, pass, shot;
    int dynamicState;
    long dynamicStartTime;

////////////First
public:
    bool isFirstFinished();
    void resetFirstPlayFinishedFlag();
    int getShotSpot();
protected:
private:
    FirstStep firstStepEnums;
    BlockerSteps blockerStep;
    int shotSpot;
    double mx;
    void stayPoistioning();
    void movePositioning();
    void donePositioning();

    void firstDegree();
    void secondDegree();
    void thirdDegree();
    void doneDegree();



};
///////////OverLoading Operators
QDebug operator<< (QDebug d, const NGameOff::SPlan _plan);

#endif // CPLAYOFF_H
