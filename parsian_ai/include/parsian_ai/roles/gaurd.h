#ifndef GAURD_H
#define GAURD_H

#include <role.h>

class CRoleGaurdInfo : public CRoleInfo {
public:
    CRoleGaurdInfo(QString _roleName);
    void reset() {}
};

class CRoleGaurd : public CRole {
protected:
    CSkillGotoPointAvoid* gotopoint;
public:
    DEF_ROLE(CRoleGaurd);
    virtual void generateFromConfig(Agent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    virtual void parse(QStringList params);
    SkillProperty(CRoleGaurd, bool, Opposite, opposite);
};

#endif // GAURD_H
