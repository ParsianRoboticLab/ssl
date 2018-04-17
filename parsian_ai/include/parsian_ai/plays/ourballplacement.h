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
        ~COurBallPlacement();
        void execute_x();
        void init(const QList <Agent*>& _agents);
        bool first;
private:
        void reset();
        BallPlacement state;
        bool flag;
        Vector2D passballpos;
        int minIndexPos;
        CAgent *ap;
        int minIndex;
        CAgent *a;

};

#endif // OURBALLPLACEMENT_H
