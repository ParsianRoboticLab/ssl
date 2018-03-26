#ifndef THEIRDIRECT_H
#define THEIRDIRECT_H

#include "masterplay.h"

class CTheirDirect : public CMasterPlay {
public:
    CTheirDirect();
    ~CTheirDirect();
    void execute_x();
    void init(const QList <Agent*>& _agents);
private:
    void reset();
};

#endif // THEIRDIRECT_H
