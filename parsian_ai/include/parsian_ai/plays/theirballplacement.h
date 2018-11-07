#ifndef THEIRBALLPLACEMENT_H
#define THEIRBALLPLACEMENT_H

#include "masterplay.h"

class CTheirBallPlacement : public CMasterPlay {
public:
    CTheirBallPlacement();
    ~CTheirBallPlacement();
    void execute_x();
    void init();
private:
    void reset();
};

#endif // THEIRBALLPLACEMENT_H
