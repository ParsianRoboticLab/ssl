#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include <string>
#include <QList>
#include <cstring>
#include <QMetaType>
#include <parsian_msgs/parsian_path.h>
#include <parsian_util/core/field.h>
#include "parsian_util/tools/drawer.h"                      //TODO must be new in node
#include "parsian_util/tools/blackboard.h"
#include "parsian_util/core/agent.h"
#include "parsian_util/geom/geom.h"
#include "parsian_util/core/worldmodel.h"
#include "planner/obstacle.h"
#include "ros/ros.h"
#include "parsian_agent/config.h"
#include "planner/state.h"
#include <planner/c2dtree.h>
#define _PLANNER_EXTEND_POINT_LIMIT 150
#define _PLANNER_EXTEND_MAX_ATTEMPT 200

#define _MAX_NUM_PLAYERS 12         //TODO must be get from somewhere else



class CPlanner {
public:
    ///////////////////////////////////////////////////////////////////
    explicit CPlanner(int _ID);
    ~CPlanner();
    void runPlanner();
    void run();
    void resetPlanner(Vector2D);
    void initPathPlanner(Vector2D _goal, const QList<int>& _ourRelaxList, const QList<int>& _oppRelaxList , const bool& _avoidPenaltyArea , const bool& _avoidCenterArea , const double& _ballObstacleRadius);
    int getID();
    vector<Vector2D> getResultModified();
    Vector2D getAverageDir();
private:
    vector<Vector2D> result , Rresult , resultModified , RresultModified;
    Vector2D goal , lastGoal , Rgoal , unchangedGoal;
    double goalProb , wayPointProb;
    double threshold , stepSize , pointStep;
    CObstacles obst;
    bool avoidPenaltyArea , avoidCenterArea;
    double ballObstacleRadius;
    QList<int> ourRelaxList , oppRelaxList;
    C2DTree nodes;
    C2DTree Rnodes;
    int ID , counter;
    Vector2D robotVel;
    Vector2D averageDir;
    bool flag;
    CField field;
    QList <Vector2D> dirs;
    Vector2D chooseTarget(Vector2D &);
    state *extendTree(state *& , Vector2D & , Vector2D & , vector<Vector2D> &);
    bool validState(state *&);
    void checkAgent();
    void Draw();
    ///////////////////////////////////////////////////////////////
    double drawTimer;
    bool isRandomState;
    Segment2D agentPath;
    void generateObstacleSpace(CObstacles &obs, QList<int> &ourRelaxList, QList<int> &oppRelaxList, bool avoidPenaltyArea, bool avoidCenterCircle , double ballObstacleRadius, Vector2D agentGoal);
    double timeEstimator(Vector2D _pos, Vector2D _vel, Vector2D _ang, Vector2D _goal);
    void createObstacleProb(CObstacles &obs, Vector2D _pos, Vector2D _vel, Vector2D _ang, Vector2D &_center, double &_rad, Vector2D agentPos, Vector2D agentVel, Vector2D agentGoal, Vector2D agentDir);
    void emitPlan(const vector<Vector2D>& _resultModified, const Vector2D& averageDir);
    bool readyToPlan;
public:
    ros::Publisher path_pub;
};

#endif // PLANNER_H
