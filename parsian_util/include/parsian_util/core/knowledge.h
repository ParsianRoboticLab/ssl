#ifndef PARSIAN_UTIL_KNOWLEDGE_H
#define PARSIAN_UTIL_KNOWLEDGE_H


#include <parsian_util/geom/geom.h>
#include <parsian_util/core/field.h>
#include <parsian_util/core/team.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_util/core/agent.h>
#include <QList>


namespace CKnowledge {

struct range {
    double a, b;
};

inline double getangle(double x1, double y1, double x2, double y2);

inline double len(double x1, double y1, double x2, double y2);

inline double len2(double x1, double y1, double x2, double y2);


inline double normalang(double dir);

inline double normalangabs(double dir);

double kickTimeEstimation(CAgent *_agent, Vector2D _target, const CBall& _ball, const double& _VMax, double AccMaxForward, double DecMax, double AccMaxNormal);
double timeNeeded(CAgent *_agentT, Vector2D posT, double vMax, double AccMaxForward, double DecMax, double AccMaxNormal);
double oneTouchAngle(Vector2D pos, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double lambda, double gamma);

double getEmptyAngle(const CField&, Vector2D p, Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle, bool oppGoal = true, bool _draw = false);
Vector2D getReflectPos(const CField& field, Vector2D goal, double dist, Vector2D _ballpos);
int getNearestRobotToPoint(CTeam _team, Vector2D _point);
};

#endif //PARSIAN_UTIL_KNOWLEDGE_H
