#include <parsian_ai/roles/role.h>
#include <parsian_ai/soccer.h>

#include <utility>

CRole::CRole() {
    shotSkill = new KickAction;
    receiveSkill = new ReceivepassAction;
    moveSkill = new GotopointavoidAction;
    oneTouchSkill = new OnetouchAction;
}

CRole::CRole(Agent *_agent) {
    setAgent(_agent);
    shotSkill = new KickAction;
    receiveSkill = new ReceivepassAction;
    moveSkill = new GotopointavoidAction;
    oneTouchSkill = new OnetouchAction;
}

CRole::~CRole() {
    delete shotSkill;
    delete receiveSkill;
    delete moveSkill;
    delete oneTouchSkill;
}

CRoleInfo::CRoleInfo(QString _roleName) {
    roleName = std::move(_roleName);
}

Agent *CRoleInfo::robot(int i) {
    return agents[i];

}

int CRoleInfo::count() {
    return agents.size();
}

void CRoleInfo::addAgent(Agent *agent) {
    if (!agents.contains(agent)) {
        agents.append(agent);
    }
}

QString CRoleInfo::getRoleName() {
    return roleName;
}

void CRoleInfo::reset() {
    agents.clear();
}
