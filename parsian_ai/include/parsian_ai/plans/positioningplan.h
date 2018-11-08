#ifndef POSITIONINGPLAN_H
#define POSITIONINGPLAN_H

#include <parsian_ai/plans/plan.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/gamestate.h>
#include <parsian_ai/util/knowledge.h>
#include <vector>
#include <queue>
#include <qbytearray.h>

enum class positioningType {
    ONETOUCH,
    TOBALL,
    TOOPPGOAL,
    TOOURGOAL
};

enum class edgeMode {
    TOP,
    BOT
};

class holdingPoints {
public:
    Agent *player;
    QList< Vector2D > points;
    QList< int > cyclesToWait;
    QList< double > escapeRadius;
    QList<positioningType> dir;
    holdingPoints() {}
    holdingPoints(Agent *_player, QList< Vector2D > _points, QList< int > _cyclesToWait, QList< double > _escapeRadius , QList<positioningType> _dir) {
        player = _player;
        points.clear();
        points.append(_points);
        cyclesToWait.clear();
        cyclesToWait.append(_cyclesToWait);
        escapeRadius.clear();
        escapeRadius.append(_escapeRadius);
        dir.clear();
        dir.append(_dir);
    }
};

class Comparar {
public:
    bool operator()(QPair< edgeMode , double >& s1, QPair< edgeMode , double >& s2) {
        if (s1.second < s2.second) {
            return true;
        } else {
            return false;
        }
    }
};

class PositioningPlan : public Plan {
private:
    QList< holdingPoints > positionStaticPoints;

    GotopointAction* gps[_MAX_NUM_PLAYERS];
    GotopointavoidAction *gpa[_MAX_NUM_PLAYERS];
    Vector2D positioningTargets[_MAX_NUM_PLAYERS];
    Vector2D staticPositioningTargets[_MAX_NUM_PLAYERS];
    Vector2D staticPositioningTargetsInput[_MAX_NUM_PLAYERS];
    Vector2D staticPositioningFacePoints[_MAX_NUM_PLAYERS];
    Vector2D lastStaticPositioningTargets[_MAX_NUM_PLAYERS];
    int posCount;
    double staticEscapeRadius[_MAX_NUM_PLAYERS];
    int executedCycles[_MAX_NUM_PLAYERS];
    Vector2D rolesPositions[_MAX_NUM_PLAYERS];
    QList<int> dynamicPositioners;
    QList<int> staticPositioners;
    int staticStateNo[_MAX_NUM_PLAYERS];

    int match[_MAX_NUM_PLAYERS];
    int last_match[_MAX_NUM_PLAYERS];

    bool dfs(int v , int n , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int mark[] , int s[] , int t[]);
    bool matching(int n , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int s[] , int t[] , int sa[]);
    void getBipartiteGraph(int n , int m , int weight[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS]);

    bool isValidPoint(Vector2D target , bool callFromStaticPositioner = false);
    bool isDifferentSequence(QList<Vector2D> first , QList<Vector2D> second);

    Vector2D findFacePoint(positioningType type);

public:
    PositioningPlan();
    void staticInit(QList< holdingPoints > &_staticPoints);
    void init(const QList<Agent*> & _agents , QString playMode);
    void staticExec();
    void execute();
    void reset();

    class PositioningObject {
    private:

        int positioner;
        Vector2D homePos, target, last_target;

        double score;

        static constexpr double target_to_goal_openness_coeff = 100.0;
        static constexpr double target_to_ball_openness_coeff = 100.0;
        static constexpr double target_opp_mean_dist_coeff    = 250.0;
//        static constexpr double target_to_home_pos_dist_coeff =-1.0;
        static constexpr double target_to_last_target_coeff    = -10.0; //-100.0;

    public:

        PositioningObject(const int _positioner ,
                          const Vector2D _homePos ,
                          const Vector2D _target ,
                          const Vector2D _last_target);

        double Score() {
            return score;
        }

        double target_to_goal_openness();
        double target_to_ball_openness();
        double target_opp_mean_dist();
        double target_to_home_pos_dist();
        double target_to_last_target();
        double getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
        double coveredArea(std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar >& obstacles);
    };
};

#endif // POSITIONINGPLAN_H
