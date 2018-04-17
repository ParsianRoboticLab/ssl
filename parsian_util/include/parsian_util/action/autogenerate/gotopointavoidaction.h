// auto-generated don't edit !

#ifndef GotopointavoidAction_HEADER_
#define GotopointavoidAction_HEADER_


#include <parsian_util/action/autogenerate/gotopointaction.h> 

#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_skill_gotoPointAvoid.h>
#include <list>

class GotopointavoidAction : public GotopointAction  {

public:
    GotopointavoidAction();
    void setMessage(const void* _msg);

    void* getMessage();

    QString getActionName() override;
    static QString SActionName();


    SkillProperty(GotopointavoidAction, bool, Chip, chip);
    SkillProperty(GotopointavoidAction, double, Kickspeed, kickSpeed);
    SkillProperty(GotopointavoidAction, double, Chipdist, chipDist);
    SkillProperty(GotopointavoidAction, bool, Onetouchflag, oneTouchFlag);
    SkillProperty(GotopointavoidAction, bool, Avoidpenaltyarea, avoidPenaltyArea);
    SkillProperty(GotopointavoidAction, bool, Noavoid, noAvoid);
    SkillProperty(GotopointavoidAction, bool, Avoidcentercircle, avoidCenterCircle);
    SkillProperty(GotopointavoidAction, float, Ballobstacleradius, ballObstacleRadius);
    SkillProperty(GotopointavoidAction, bool, Drawpath, drawPath);
    SkillProperty(GotopointavoidAction, bool, Divemode, diveMode);
    SkillProperty(GotopointavoidAction, Vector2D, Addvel, addVel);
protected:
    std::list<unsigned char> ourrelax;
public:
    void clearOurrelax();
    void addOurrelax(unsigned char);
    void removeOurrelax(unsigned char);
    std::list<unsigned char> getOurrelax();
protected:
    std::list<unsigned char> theirrelax;
public:
    void clearTheirrelax();
    void addTheirrelax(unsigned char);
    void removeTheirrelax(unsigned char);
    std::list<unsigned char> getTheirrelax();


};

#endif // GotopointavoidAction_HEADER_
