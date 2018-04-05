#ifndef THEIRPENALTYSHOOTOUT_H
#define THEIRPENALTYSHOOTOUT_H

#include <parsian_ai/plays/masterplay.h>
#include <parsian_ai/roles/playmake.h>
#include <parsian_ai/gamestate.h>
#include <parsian_util/geom/geom.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/roles/playmake.h>


enum class PenaltyShootoutState{
    Positioning,
    Goaling
};

enum class ShootOutState{
    Trick,
    Shoot
};

class CTheirPenaltyShootout : public CMasterPlay {

public:
    CTheirPenaltyShootout();
    ~CTheirPenaltyShootout();
    void execute_x();
    void init(const QList <Agent*>& _agents);
    void setState(PenaltyShootoutState _state){penaltyState = _state;};
    void setPlaymake(Agent* _playmakeAgent);
    void generatePositions();
    Vector2D getEmptyTarget(Vector2D _position, double _radius);
    void assignSkills();
    void playmakeInitialPositioning();
    void runShootOut();

private:
    void reset();
    QList<GotopointavoidAction*> moveSkills;
    GotopointavoidAction* PMgotopoint;
    KickAction* PMkick;
    QTime shootoutTimer;
    bool timerFlag;
    QList<Vector2D> positions;
    PenaltyShootoutState penaltyState;
    ShootOutState shootoutState;
};

#endif // THEIRPENALTYSHOOTOUT_H
