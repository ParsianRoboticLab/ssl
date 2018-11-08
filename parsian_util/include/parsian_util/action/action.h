//
// Created by parsian-ai on 9/29/17.
//

#ifndef PARSIAN_UTIL_ACTION_H
#define PARSIAN_UTIL_ACTION_H

#include <QString>

class Action {
public:

    virtual void setMessage(const void*) = 0;
    virtual void* getMessage() = 0;
    virtual QString getActionName();

protected:
private:
};

#define SkillProperty(skill,type,name,local) \
        public: inline type get##name() {return local;} \
        public: inline skill* set##name(type val) {local = val;return this;} \
        protected: type local

#define SkillPropertyNoSet(skill,type,name,local) \
        public: inline type get##name() {return local;} \
        public: skill* set##name(type val); \
        protected: type local

#endif //PARSIAN_UTIL_ACTION_H
