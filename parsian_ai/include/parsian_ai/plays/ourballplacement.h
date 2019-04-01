#ifndef OURBALLPLACEMENT_H
#define OURBALLPLACEMENT_H

#include "masterplay.h"

enum class BallPlacement {
    NoState = 0,
    GO_FOR_BALL = 1,
    PASS  = 2,
    GO_FOR_VALID_PASS = 3,
    VALID_PASS = 4,
    RECIVE_AND_POS = 5,
    FINAL_POS = 6,
    DONE = 7
};

class COurBallPlacement : public CMasterPlay {
public:
        COurBallPlacement();
        ~COurBallPlacement();
        void execute_x();
        void init(QList<Agent*>& _agents);
        bool first;
        static int chooseFirst();

private:
    Agent* kickerAgent;
    int i;
    int  loopCounter;
    int fuckOff;
    Agent* receiverAgent;
    bool nearFlag, shotFlag, updateFlag , find;
    Agent* nearID ;
    Vector2D lastBallPos;
    Vector2D ballPosBeforKick;
    Vector2D kickTarget;
    GotopointavoidAction *gpaP, *gpaK;
    GotopointavoidAction *gpaH, *gpaR;
    GotopointavoidAction *sag;
    GotopointavoidAction *gpa[_NUM_PLAYERS];
    ReceivepassAction *recivePass;
    KickAction *pass;

    bool divari(const Vector2D &ballPos);
    bool isAgentsOnThePosition(Agent* kickerAgent, Agent* reciverAgent);
    static bool isBallHaseMoved(const Vector2D &, const Vector2D &, const double &);
    static bool isBallNearToTarget(const Vector2D &ballPos, const Vector2D &targetPos, const double &dist);
    bool isBallSpeedLow(const double &speed, const Vector2D &velocity);
    bool isPassReceived(const Vector2D &ballPos, const Vector2D &desiredPos);
    double kickSpeedCalculator(const Vector2D &ballPos, const Vector2D &targetPos);
    void reset() override;
    void otherRobotsFormation(Agent* ,Agent*);
    Agent* reciverFinder(const Vector2D &, Agent*);
    Agent* kickerfinder(const Vector2D & );
    Agent* firstStep(const Vector2D &);
};

#endif // OURBALLPLACEMENT_H
