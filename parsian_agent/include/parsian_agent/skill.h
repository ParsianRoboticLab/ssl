#ifndef BASICSKILL_H
#define BASICSKILL_H

#include <parsian_agent/newbangbang.h>
#include <parsian_util/base.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_msgs/parsian_agent.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_util/geom/geom.h>
#include <QtCore/QStringList>
#include <parsian_util/action/action.h>
#include <parsian_agent/agent.h>
#include <parsian_util/tools/blackboard.h>
#include <parsian_util/tools/drawer.h>

using namespace rcsc;

class CSkill {
public:
    QString localAgentName;
    CSkill() = default;

    explicit CSkill(Agent* _agent);
    ~CSkill();
    virtual CSkill* allocate(Agent* _agent) = 0;
    virtual QString getName() = 0;

    //these functions should be defined in child classes
    virtual double progress() = 0; //between 0-1 ; less than zero on failure
    virtual void execute() = 0;

    Property(Agent*, Agent, agent){};
    friend class CSkills;
};

class CSkills {
public:
    CSkills();
    ~CSkills();
    static bool registerSkill(const char *name, CSkill* Skill);
    static int skillsCount();
    static CSkill* skill(int i);
private:
    struct RegisteredSkill {
        const char *name;
        CSkill *Skill;
        void* Info;
    };
    static QList<RegisteredSkill>* Skills;
    static bool inited;
};

#define DEF_SKILL(skill) \
    skill(Agent* _agent); \
    skill() {} \
    ~skill(); \
    static const char *Name;\
    virtual CSkill* allocate(Agent* _agent); \
    virtual void execute(); \
    virtual double progress(); \
    virtual QString getName(); \
    static skill* set(Agent* a); \
    static skill* get(Agent* a)


#define INIT_SKILL(_Skill,name) \
    CSkill* _Skill::allocate(Agent* _agent) \
    {return new _Skill(_agent);} \
    QString _Skill::getName() {return QString(Name);} \
    _Skill* _Skill::set(Agent* a) \
    { \
        if (QString(_Skill::Name)!=a->skillName) \
        { \
            if (a->skill!=NULL && a->skillName!="") delete ((_Skill*) a->skill); \
            else a->skill = (CSkill*) (new _Skill(a)); \
            a->skillName = _Skill::Name; \
        } \
        return (_Skill*) a->skill; \
    } \
    _Skill* _Skill::get(Agent* a) \
    { \
        return (_Skill*) a->skill; \
    } \
        bool _Skill##_registered \
      = CSkills::registerSkill(_Skill::Name,new _Skill());\
    const char* _Skill::Name = name
#endif // BASICSKILL_H
