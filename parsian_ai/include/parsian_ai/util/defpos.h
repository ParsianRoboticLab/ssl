#ifndef DEFPOS_H
#define DEFPOS_H

#include "parsian_util/geom/geom.h"
#include "parsian_ai/util/worldmodel.h"
#include <parsian_util/tools/drawer.h>

struct Intersection {
    double angle[2];
    double radius[2];
};

struct DefPos {
    int size = -1;
    Vector2D pos[5];
};


class CDefPos {
public:
    CDefPos();
    DefPos getDefPositions(const Vector2D& _ballPos, int _size, double _limit1, double _limit2);
    static DefPos getStaticDefPositions(const Vector2D& _ballPos, int _size, double _limit1, double _limit2);
    static Vector2D getXYByAngle(double _angle, double _radius);
    static double getRobotAngle(double _radius);
    static Intersection getIntersections(Vector2D _ballPos, double _radius, bool isNearPenaltyArea);
    static double findBestRadius(int _numOfDefs);
private:
    static double getAngleByXY(Vector2D _point);
    double oneDefThr;
    static double penaltyAreaOffset;
    double penaltyAreaRadius;
    Circle2D penaltyAreaCircle;
};


#endif // DEFPOS_H
