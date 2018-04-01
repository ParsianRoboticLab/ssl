#ifndef CHALFTIMELINEUP_H
#define CHALFTIMELINEUP_H

#include "parsian_ai/plays/masterplay.h"
#include <parsian_ai/config.h>


class CHalftimeLineup : public CMasterPlay{
public:
    CHalftimeLineup();
    ~CHalftimeLineup();
    void execute_x();
    void init(const QList <Agent*>& _agents);
    void reset();
    void fillPoints();
    void fillGPA();
private:
    bool halt;
    bool ready;

    QList<Vector2D> points;
    GotopointavoidAction* gpa[_MAX_NUM_PLAYERS];
};

#endif // CHALFTIMELINEUP_H
