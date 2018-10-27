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

class COurBallPlacement : public CMasterPlay{
public:
        COurBallPlacement();
        ~COurBallPlacement() override;
        void execute_x() override;
        void init(const QList <Agent*>& _agents) override;
        bool first;
        static int chooseFirst();


private:
        void reset() override;
        void otherRobotsFormation(const int&) const;
        int agentFinder(const Vector2D& pos , int) const;
        int firstStep(const Vector2D&);
        double xFinder(const Vector2D& desired , const Vector2D& ballPos , const double& dist) const ;
        double yFinder(const Vector2D& desired , const Vector2D& ballPos , const double& dist) const ;
        double tetaFinder(const Vector2D& desired, const Vector2D& ballPos) const ;
        BallPlacement state;
        bool flag;
        Vector2D passballpos;
        int minIndexPos;
        CAgent *ap;
        int nearAgentToBall;
        CAgent *a;
        double array[2][2];
        bool nearFlag , phFlag , shotFlag ,updateFlag ,reciveFlag;
        int nearID , nearTargetAgent;
        Vector2D currentBallPos;
        Vector2D lastBallPos;
        Vector2D ballPosBeforKick;
        GotopointavoidAction* gpa;
        GotopointavoidAction* gpa0;GotopointavoidAction* gpa1;GotopointavoidAction* gpa2;GotopointavoidAction* gpa3;
        GotopointavoidAction* gpa4;GotopointavoidAction* gpa5;GotopointavoidAction* gpa6;;GotopointavoidAction* gpa7;
        ReceivepassAction* recivePass;
        KickAction* pass;
        bool ballHaseMoved(const Vector2D& , const Vector2D&) const;
        bool ballDidntAriveToTarget(const Vector2D& ballPos , const Vector2D& desiredPos , const Vector2D& ballPosBeforKick) const;
        bool ballDidntKickedWell(const Vector2D& ballPos, const Vector2D& ballPosBeforKick) const;
        bool reciverAgentIsOnThePosition(const int &agent, const Vector2D &targetPos) const;
        bool ballIsNearToTarget(const Vector2D &ballPos, const Vector2D &targetPos) const ;
        bool ballSpeedIsLow();
        bool kickerAgentIsNearToBall(const Vector2D &ballPos, const int &agent) const ;
};

#endif // OURBALLPLACEMENT_H
