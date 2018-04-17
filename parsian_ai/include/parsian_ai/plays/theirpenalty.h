#ifndef THEIRPENALTYF_H
#define THEIRPENALTYF_H

#include "masterplay.h"

class CTheirPenalty : public CMasterPlay {
public:
    CTheirPenalty();
    ~CTheirPenalty();
    void execute_x();
    void init(const QList<Agent*>& _agents);
    void generatePositions();
    void executePositioning();
    void assignSkills();
    Vector2D getEmptyTarget(Vector2D _position, double _radius);


private:
    void reset();
    bool isPenaltyShootOut;
    QList<GotopointavoidAction*> moveSkills;
    QList<Vector2D> positions;
    Vector2D penaltyTarget;
};

#endif // THEIRPENALTYF_H
