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
    int kickerAgent;
    int receiverAgent;
    bool nearFlag, shotFlag, updateFlag , loop;
    int nearID ;
    Vector2D lastBallPos;
    Vector2D ballPosBeforKick;
    GotopointavoidAction *gpaP;
    GotopointavoidAction *gpaH;
    GotopointavoidAction *gpa[_NUM_PLAYERS];
    ReceivepassAction *recivePass;
    KickAction *pass;


    static bool isBallHaseMoved(const Vector2D &, const Vector2D &);
    static bool isBallDidntKickedWell(const Vector2D &ballPos, const Vector2D &ballPosBeforKick);
    bool isReciverAgentOnThePosition(const int &agent, const Vector2D &targetPos);
    static bool isBallNearToTarget(const Vector2D &ballPos, const Vector2D &targetPos, const double &dist);
    bool isBallSpeedLow(const double &speed, const Vector2D &velocity);
    bool isKickerOnThePosition(const Vector2D &ballPos, const int &agent) const;
    bool isPassReceived(const Vector2D &ballPos, const Vector2D &desiredPos, const int &nearAgent, const int &nearTargetAgent);
    void reset() override;
    void otherRobotsFormation(const int & ,  const int&) const;
    int agentFinder(const Vector2D &, const int &);
    int firstStep(const Vector2D &);
};

#endif // OURBALLPLACEMENT_H
