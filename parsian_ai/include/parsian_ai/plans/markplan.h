#ifndef __MIDDLE_PLAN_H_
#define __MIDDLE_PLAN_H_

#include <parsian_ai/plans/plan.h>
#include <vector>
#include <QList>
#include <parsian_ai/roles/roles.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_ai/config.h>

class CMarkPlan : public Plan {
public:
    ////////////////////////////// AHZ ///////////////////
    Line2D getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    void manToManMarkInPlayOnBlockPass(QList<Vector2D> opponentAgentsToBeMarkePossition , int ourMarkAgentsSize , double proportionOfDistance);
    bool isInIndirectArea(Vector2D aPoint);
    QList <QString> markRoles;
    void findOppAgentsToMark();

    double ballCircleR = 0.5;
    ///////////////////////////////////////////////////
    ////Filtered Variables
    //bool weOwnBall; del
    //bool stopMode; del
    int ourBallOwner, oppBallOwner;    //Agent Id del
    Agent* supporter;
    int lastFrameInSupportMode;
    int lastFrameInClearingMode;
    int midFielder;
    double midFielderDist;
    /////
    int ourFastest;
    double ourFastestTime;
    int oppFastest;
    double oppFastestTime;
    double fastestTime;
    bool ballOurs;

    double markRadius;
    double markRadiusStrict;
    int fastest;
    double distance;
    int lastFastest, lastFastestFoundCycle;
    QList<int> oppOffenders;
    QList<int> ourOffenders;
    QList<int> lastAgents;
    QList<bool> ballOursQueue;
    QList<int> ourFastestQueue;
    QList<int> oppFastestQueue;
    double fastestTimeSum;

    int ballInOurField;
    ///--- HMD ----///
    CMarkPlan();
    ~CMarkPlan();
    double segmentpershoot;
    double segmentperpass;
    GotopointavoidAction *markGPA[6];
    void execute();
    Vector2D posvel(CRobot* opp);
    void markPosesRefinePlayon();
    QList<Vector2D> ShootBlockRatio(double, Vector2D);
    QList<Vector2D> PassBlockRatio(double ratio, Vector2D opp);
    bool sortBy(const Vector2D &robot1, const Vector2D &robot2);
    QList<CRobot*> sortdanger(const QList<CRobot*> oppagent);
    QList<QPair<Vector2D, double> > sortdangerpassplayoff(QList<Vector2D> oppposdanger);
    QList<QPair<Vector2D, double> > sortdangerpassplayon(QList<Vector2D> oppposdanger);
    QList<QPair<Vector2D, double> > sortdangershoot(double radius, double treshold);
    QList<Vector2D> ZoneMark(QList<QPair<Vector2D, double> > Posdanger , int marknum);
    Vector2D findcenterdefence();

    QList<CRobot*> oppAgentsToMark;
    QList<CRobot*> oppAgentsMarkedByDef;
    QList<CRobot*> ignoredOppAgents;
    QList<Agent*> offenseAgent;
    QList<Vector2D> markPoses;
    QList<Vector2D> oppAgentsToMarkPos;
    QList<Vector2D> markAngs;
    QList<Vector2D> obspos;
    QList<Vector2D> opppos;
    QList<Vector2D> oppmarkedpos;
    ///
    double ballCatch;
    int oppsInOurOneThirth;
    bool ballCatcherDanger;
    int numberOfMarkers;

protected:
//    CRoleMark *oldMark[_MAX_NUM_PLAYERS]; // TODO : FIX THIS DASTAN
};

#endif // __MIDDLE_PLAN_H_
