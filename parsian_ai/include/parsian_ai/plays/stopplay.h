#ifndef CSTOPPLAY_H
#define CSTOPPLAY_H

#include "parsian_ai/plays/masterplay.h"

class CStopPlay : public CMasterPlay {
public:
    CStopPlay();
    ~CStopPlay() override;

    void execute_x() override;
    void init(const QList <Agent*>& _agents) override;
private:
    void stopPosition();
    void reset() override;

    GotopointavoidAction *gpa[6];
    Vector2D rolePosition[6];

};

#endif // CSTOPPLAY_H
