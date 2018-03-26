#ifndef Block_H
#define Block_H

#include <parsian_ai/roles/role.h>


class CRoleBlockInfo : public CRoleInfo {
public:
    explicit CRoleBlockInfo(QString _roleName);
    void findPos(bool blockGoal);
    Vector2D blockPosition;

};

class CRoleBlock : public CRole {
protected:
    GotopointavoidAction* gotopoint;
    static CRoleBlockInfo* roleInfo;
public:
    explicit CRoleBlock(Agent *_agent);
    ~CRoleBlock();
    void execute();
    double progress();
    virtual void parse(QStringList params = QStringList());
    CRoleBlockInfo* info();
    SkillProperty(CRoleBlock, bool, BlockGoal, blockGoal);
    SkillProperty(CRoleBlock, bool, Stop, stop);
};

#endif // Block_H

