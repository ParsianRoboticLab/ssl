#ifndef BEHAVIORMOVE_FORWARD_H
#define BEHAVIORMOVE_FORWARD_H

#include<parsian_ai/util/behavior.h>
#include <cmath>
#include <QList>
#include <QPair>

#define OPP_GOAL_Y 0
#define OPP_GOAL_X 6

class BehaviorMove_Forward: public Behavior {

public:
    BehaviorMove_Forward();
    virtual ~BehaviorMove_Forward();
    virtual int execute();
    virtual double eval();
    void sortobstacles(QList<Segment2D> &obstacles);
    void swap(Segment2D *xp, Segment2D *yp);
    double angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp);
    double findmax(const QList<double> &list);
private:
    Segment2D left;

};

#endif // BEHAVIORMOVE_FORWARD_H
