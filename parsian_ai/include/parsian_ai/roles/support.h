#ifndef Support_H
#define Support_H

#include <parsian_ai/roles/role.h>

//class CRoleSupportInfo : public CRoleInfo {
//public:
//    CRoleSupportInfo(QString _roleName);
//
//    void reset() {}
//};

class CRoleSupport : public CRole {
protected:
    Vector2D supportPosition;
public:
    //DEF_ROLE(CRoleSupport);
    void findPos();
    void execute() override;
    void update() override;
    explicit CRoleSupport(Agent* agent);
    ~CRoleSupport();
    virtual void parse(QStringList params);


SkillProperty(CRole, SupporterSkill, SelectedSupporterSkill, supporterSkill);
};

#endif // Support_H
