// auto-generated don't edit !

#ifndef NoAction_HEADER_
#define NoAction_HEADER_



#include <parsian_util/action/action.h>
#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_skill_no.h>
#include <list>

class NoAction : public Action {

public:
    NoAction();
    void setMessage(const void* _msg);

    void* getMessage();

    QString getActionName() override;
    static QString SActionName();


    SkillProperty(NoAction, bool, Waithere, waithere);


};

#endif // NoAction_HEADER_
