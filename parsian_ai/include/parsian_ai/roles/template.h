#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <role.h>

class CRoleTemplateInfo : public CRoleInfo {
public:
    CRoleTemplateInfo(QString _roleName);
};


class CRoleTemplate : public CRole {
public:
    DEF_ROLE(CRoleTemplate);
    virtual void generateFromConfig(Agent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
};

#endif // Template_H
