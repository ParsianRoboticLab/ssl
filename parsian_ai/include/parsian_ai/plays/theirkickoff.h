#ifndef THEIRKICKOFF_H
#define THEIRKICKOFF_H

#include "masterplay.h"

class CTheirKickOff : public CMasterPlay {
public:
    CTheirKickOff();
    ~CTheirKickOff();
    void execute_x();
    void init(const QList <Agent*>& _agents);
private:
    void reset();
};

#endif // THEIRKICKOFF_H
