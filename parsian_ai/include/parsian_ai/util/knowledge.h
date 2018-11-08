#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include <parsian_util/mathtools.h>
#include <parsian_util/core/agent.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_util/core/knowledge.h>
#include <QVariant>
#include <QMap>
#include "agent.h"

#define Def_Var "Defense"
#define Pos_Var "positioning"
#define Mark_Var "Mark"

struct AngleRange {
    AngleDeg begin;
    AngleDeg end;
};

struct emptyAngleStruct {
    bool begin;
    double angle;
};

struct NewFastestToBall {
    double catch_time;
    bool isFastestOurs;
    QList< std::pair<double , int> > ourF;
    QList< std::pair<double , int> > oppF;
    NewFastestToBall() {
        catch_time = 10000;
        isFastestOurs = false;
        ourF.clear();
        oppF.clear();
    }
    int ourFastest() {
        if (ourF.size()) {
            return ourF.first().second;
        }
        return -1;
    }
    double ourFastestTime() {
        if (ourF.size()) {
            return ourF.first().first;
        }
        return -1;
    }
    int oppFastest() {
        if (oppF.size()) {
            return oppF.first().second;
        }
        return -1;
    }
    double oppFastestTime() {
        if (oppF.size()) {
            return oppF.first().first;
        }
        return -1;
    }
};

struct FastestToBall {
    double catch_time;
    int ourFastest;
    double ourFastestTime;
    int oppFastest;
    double oppFastestTime;
    FastestToBall() {
        catch_time = 1000;
        ourFastest = -1;
        ourFastestTime = -1;
        oppFastest = -1;
        oppFastestTime = -1;
    }
};

class Knowledge {
public:

    Knowledge();
    ~Knowledge();
    void initVariables();

    QVariantMap variables;
    NewFastestToBall newFastestToBall(double timeStep, QList<int> ourList, QList<int> oppList, const CWorldModel*& wm);
    FastestToBall findFastestToBall(QList<int> ourList, QList<int> oppList, const CWorldModel*& wm);


    NewFastestToBall newFastestToBall(double timeStep = 0.1, QList<int> ourList = wm->our.data->activeAgents, QList<int> oppList = wm->opp.data->activeAgents);
    int Matching(QList <Agent*> robots, QList <Vector2D> pointsToMatch, QList <int> &matchPoints);
    int MatchingMinTheMax(const QList <Agent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints);
    Vector2D getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion);
    double getEmptyAngle(Vector2D p, Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle, bool oppGoal, bool _draw = false);
    double getEmptyAngle(Vector2D p, Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle);
    Vector2D getEmptyPosOnGoal(Vector2D from, double &regionWidth, bool oppGoal, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, double wOpenness = 0.5, bool _draw = false);
    Vector2D getEmptyPosOnGoalForPenalty(double n, bool oppGoal, double th, Agent* ourAgent = NULL);
    bool isPointClear(Vector2D point, Vector2D from, double rad = 0.0795, bool considerRelaxedIDs = false, QList<int> ourRelaxedIDs = QList<int>(), QList<int> oppRelaxedIDs = QList<int>());
    bool isPointClear(Vector2D point, Vector2D from, double radBig, double radSmall, bool considerRelaxedIDs, QList<int>ourRelaxedIDs, QList<int>oppRelaxedIDs, QList<int>ourSmallIDs, QList<int>oppSmallIDs);
    double chipGoalPropability(bool isOurChip, Vector2D _goaliePos);
    int getNearestOppToPoint(Vector2D point);
    int nearestOppToBall();
    double chipGoalPropability(bool isOurChip);






private:

    QList<AngleRange> emptyAngles;
};

extern Knowledge * know;

#endif // KNOWLEDGE_H
