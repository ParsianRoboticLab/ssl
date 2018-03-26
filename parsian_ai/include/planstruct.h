//
// Created by fateme on 11/10/17.
//


#ifndef PARSIAN_AI_PLANSTRUCT_H
#define PARSIAN_AI_PLANSTRUCT_H

#include <parsian_util/geom/geom.h>

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

enum POMODE {
    DIRECT   = 1,
    INDIRECT = 2,
    KICKOFF  = 3
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

////Play Off Plans
namespace NGameOff {

enum EMode {
    FirstPlay   = 0,
    FastPlay    = 1,
    StaticPlay  = 2,
    DynamicPlay = 3
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

#endif //PARSIAN_AI_PLANSTRUCT_H