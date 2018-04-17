#include "parsian_util/core/team.h"
#include <parsian_util/tools/blackboard.h>
#include <ros/ros.h>



CTeam::CTeam(const bool& isYellow, const bool& isLeft) {
    data = new CTeamData;
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        data->teamMembers[i] = new CRobot(i);
    }
    setColor(isYellow);
    setSide(isLeft);

}

CTeam::CTeam(const  bool& isYellow, const bool& isLeft, const std::vector<parsian_msgs::parsian_robot> & _robots) :
    CTeam(isYellow, isLeft) {
    updateRobot(_robots);
}

CTeam::~CTeam() {
    for (auto &teamMember : data->teamMembers) {
        delete teamMember;
    }
    delete data;
}

void CTeam::updateRobot(const std::vector<parsian_msgs::parsian_robot> &_robots) {
    for (auto &teamMember : data->teamMembers) {
        teamMember->setActive(false);
    }

    for (auto& robot : _robots) {
        data->teamMembers[robot.id]->update(robot);
        data->teamMembers[robot.id]->setActive(true);
    }
    update();

}

int CTeam::activeAgentsCount() {
    return data->activeAgents.size();
}

void CTeam::update() {
    data->activeAgents.clear();
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        if (data->teamMembers[i]->getActive()) {
            data->activeAgents.push_back(i);
        }
    }
    ROS_INFO_STREAM("ACTs : " << data->activeAgents.size());
}

int CTeam::activeAgentID(int i) {
    if ((i < data->activeAgents.size()) && (i >= 0)) {
        return data->activeAgents[i];
    }
    DEBUG(QString("request for id %1 that does not exist in team").arg(i), D_ERROR);
    DEBUG("Active Agents:", D_DEBUG);
    for (int activeAgent : data->activeAgents) {
        DEBUG(activeAgent , D_DEBUG);
    }
    return -1;

}
CRobot* CTeam::operator [](const int& i) const {
    if (i >= 0 && i < _MAX_NUM_PLAYERS) {
        return data->teamMembers[i];
    }
    PDEBUG("id out of range", i, D_ERROR);
    return nullptr;
}

CRobot* CTeam::active(const int& i) const {
    if ((i < data->activeAgents.size()) && (i >= 0)) {
        return data->teamMembers[data->activeAgents[i]];
    }
    DEBUG(QString("request for id %1 that does not exist in team").arg(i), D_ERROR);
    DEBUG("Active Agents:", D_DEBUG);
    for (int activeAgent : data->activeAgents) {
        DEBUG(activeAgent , D_DEBUG);
    }
    return nullptr;

}


void CTeam::setColor(const bool& isYellow) {
    data->color.fromStdString((isYellow) ? "yellow" : "blue");
}

void CTeam::setSide(const bool& isLeft) {
    data->side.fromStdString((isLeft) ? "left" : "right");
}

void CTeam::updateGoaliID(int id) {
    data->goalieID = id;
}
