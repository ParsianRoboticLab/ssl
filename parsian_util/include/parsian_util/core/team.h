//
// Created by parsian-ai on 9/21/17.
//

#ifndef PARSIAN_UTIL_TEAM_H
#define PARSIAN_UTIL_TEAM_H

#include "parsian_util/core/agent.h"
#include "parsian_util/base.h"
#include <QMutex>

struct CTeamData {
    CRobot* teamMembers[_MAX_NUM_PLAYERS];
    QList<int> activeAgents;
    QString color;
    QString side;
    int goalieID;
};

class CTeam {
public:
    CTeamData *data;
    CTeam(const bool& isYellow, const bool& isLeft);
    CTeam(const bool& isYellow, const bool& isLeft, const std::vector<parsian_msgs::parsian_robot>&);
    ~CTeam();
    CRobot* operator [](const int& i) const;
    CRobot* active(const int& i) const;
    int activeAgentsCount();
    int activeAgentID(int i);
    void update();
    void updateRobot(const std::vector<parsian_msgs::parsian_robot>& _robots);

    void setColor(const bool& isYellow);
    void setSide(const bool& isLeft);
    void updateGoaliID(int id);

private:
    QMutex wmMutex;

};

#endif //PARSIAN_UTIL_TEAM_H
