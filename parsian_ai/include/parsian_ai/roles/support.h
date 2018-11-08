#ifndef Support_H
#define Support_H

#include <role.h>

class CRoleSupportInfo : public CRoleInfo {
public:
    CRoleSupportInfo(QString _roleName);
    void findPos();
    Vector2D supportPosition;
    void reset() {}
};

class CRoleSupport : public CRole {
protected:
    CSkillGotoPointAvoid* gotopoint;
    CSkillKick* kick;
public:
    DEF_ROLE(CRoleSupport);
    virtual void generateFromConfig(Agent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    virtual void parse(QStringList params);
};

#endif // Support_H
