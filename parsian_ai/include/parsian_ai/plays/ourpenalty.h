#ifndef OURPENALTY_H
#define OURPENALTY_H

#include <parsian_ai/plays/masterplay.h>
#include <parsian_ai/roles/playmake.h>
#include <parsian_ai/gamestate.h>
#include <parsian_util/geom/geom.h>
#include <parsian_ai/util/knowledge.h>

enum class PenaltyState{
    Positioning,
    Kicking
};

enum class KickState{
    Trick,
    Shoot
};

class COurPenalty : public CMasterPlay {
public:
    COurPenalty();
    ~COurPenalty();
    void execute_x();
    void init(const QList <Agent*>& _agents);
    void setPlaymake(Agent* _playmakeAgent);
    void setState(PenaltyState _state){penaltyState = _state;};
    void executeShootoutPositioning();
    void executeNormalPositioning();
    Vector2D getEmptyTarget(Vector2D _position, double _radius);
    void assignSkills();
    void playmakePositioning();
    void playmakeKick();
    double angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp);




private:
    void reset();
    bool isPenaltyShootOut;
    CRolePlayMake* playmakeRole;
    QList<GotopointavoidAction*> moveSkills;
    GotopointavoidAction* PMgotopoint;
    KickAction* PMkick;
    void generatePositions();
    QList<Vector2D> positions;
    PenaltyState penaltyState;
    KickState kickState;
    QTime changeDirPenaltyStrikerTime;
    bool timerStartFlag;
    Vector2D penaltyTarget;


};

#endif // OURPENALTY_H
