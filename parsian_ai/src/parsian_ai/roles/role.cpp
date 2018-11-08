#include <parsian_ai/roles/role.h>
#include <parsian_ai/soccer.h>

#include <utility>

CRole::CRole() = default;

CRole::CRole(Agent* _agent) {
    assign(_agent);
}


void CRole::assign(Agent* _agent) {
    agent = _agent;
}

CRoleInfo::CRoleInfo(QString _roleName) {
    roleName = std::move(_roleName);
}

Agent* CRoleInfo::robot(int i) {
    return agents[i];

}

int CRoleInfo::count() {
    return agents.size();
}

int CRoleInfo::index(CRole *me) {
    return agents.indexOf(me->agent);
}

void CRoleInfo::addAgent(Agent* agent) {
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
