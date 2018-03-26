#ifndef KICK_H
#define KICK_H

#include <parsian_agent/gotopointavoid.h>
#include <QtCore/QQueue>
#include <parsian_util/core/ball.h>
#include <parsian_util/core/movingobject.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/core/knowledge.h>

#define vRobotTemp 1


enum kckMode {
    KDIRECT = 0,
    KINDIRECCT = 1,
    KJTURN = 2,
    KTURN = 3,
    KINTERCEPT = 4,
    KKICK = 5,
    KPENALTY = 6,
    KAVOIDOPPENALTY = 7,
    KWAITANDKICK = 8,
    KDONTKICK = 9,
    KWAITFORTURN = 10,
    KGOALIE = 11

};

class CSkillKick : public CSkill, public KickAction {
private:
    kckMode decideMode();
    kckMode kickMode;
    _PID *angPid;
    _PID *speedPid;
    _PID *posPid;


    Vector2D agentPos;
    Vector2D agentDir;
    Circle2D kickerArea;
    double jturnThr;
    double onetouchThr;
    double distThr;
    QQueue <Vector2D> dirQueue;
    double kkDist;
    double kkBallDist;
    double kkRadThresh;
    double kkAngleThresh;
    double kkTAngleThresh;
    double kkKickThresh;
    double kkKickAngThresh;
    double jTurnThr;
    Vector2D oneTouchDir;

    double kkJTurnThresh;
    double kkMovementTheta;
    Vector2D ballPos;
    double ballRealVel;
    AngleDeg kickFinalDir;
    Vector2D finalDirVec;
    Vector2D finalPos;
    void direct();
    void indirect();
    void jTurn();
    void turnForKick();
    void kkKick();
    void kkIntercept();
    void kkPenalty();
    void avoidOppPenalty();
    void avoidOurPenalty();
    void waitAndKick();
    void kDontKick();
    void kWaitForTurn();
    void kgoalie();
    void findPosToGo();
    void findPosToGoAlt();
    double oneTouchAngle(Vector2D pos, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double landa, double gamma);
    Vector2D jTurnStartPos;
    bool isJturn;
    bool jTurnFromBack;
    CSkillGotoPointAvoid *gpa;
    Vector2D getTurnFunc(double ang, double _r, double _angOff = 0);



protected:
    CSkillGotoPointAvoid* gotopoint;
    bool lastThroughed;
    int chipWait;
    int chipSpinSpeed;
    bool readyToChip;
    CAgent* agent0;
    bool ballflag;
    bool ballWasFar;
    bool kickerOn;
public:
    DEF_SKILL(CSkillKick);
    void resetI();
    bool kickable();
    Vector2D findMostPossible();
    static double kickTimeEstimation(Agent * _agent, Vector2D target, const CBall& _ball);
};


#endif // KICK_H