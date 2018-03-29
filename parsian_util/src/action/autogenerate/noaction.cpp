// auto-generated don't edit !

#include <parsian_util/action/autogenerate/noaction.h>

NoAction::NoAction() {
       waithere = false;
}

void NoAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_no msg = *((parsian_msgs::parsian_skill_no*)_msg);
        waithere = msg.waithere;

}

void* NoAction::getMessage() {
    parsian_msgs::parsian_skill_no* _msg = new parsian_msgs::parsian_skill_no;
    _msg->waithere = waithere;
    return _msg;

}


QString NoAction::getActionName(){
    return SActionName();
}

QString NoAction::SActionName(){
    return QString{"NoAction"};
}

