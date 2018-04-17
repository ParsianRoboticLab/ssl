/*
  Copyright 2016 Lucas Walter
*/

#include "rqt_parsian_gui/robotstatus.h"

class parsian_robot_status;

namespace rqt_parsian_gui {

RobotStatus::RobotStatus()
    : rqt_gui_cpp::Plugin() {
    // Constructor is called first before initPlugin function, needless to say.
    // give QObjects reasonable names
    setObjectName("RobotStatus");
}

void RobotStatus::initPlugin(qt_gui_cpp::PluginContext &context) {
    n = getNodeHandle();
    n_private = getPrivateNodeHandle();

    rs_sub = n_private.subscribe("/robots_status", 1000, &RobotStatus::rsCallback, this);

    // create QWidget
    scroll_widget = new QWidget;
    scrollArea = new QScrollArea;
    scrollArea_l = new QVBoxLayout;

    scrollArea_l->setSizeConstraint(QLayout::SetMinimumSize);
    scroll_widget->setLayout(scrollArea_l);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWindowTitle("RobotStatus");
    scrollArea->setFixedHeight(800);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidget(scroll_widget);

    for (auto &i : statusWidget) {
        i = new RobotStatusWidget(parsian_msgs::parsian_team_config::YELLOW);
        scrollArea_l->addWidget(i);
    }



    // extend the widget with all attributes and children from UI file

    // add widget to the user interface

    context.addWidget(scrollArea);
    for (int j = 0; j < max_robot; ++j) {
        rc_sub[j] = n_private.subscribe(QString("/agent_%1/command").arg(j).toStdString(), 1000, &RobotStatus::rcCallback, this);
    }
}

void RobotStatus::rsCallback(const parsian_msgs::parsian_robots_statusConstPtr msg) {

    for (int i = 0; i < msg->status.size(); i++) {
        statusWidget[i]->setMessage(msg->status[i]);
    }
}

void RobotStatus::rcCallback(const parsian_msgs::parsian_robot_commandConstPtr msg) {
    if(msg->robot_id < max_robot)
        statusWidget[msg->robot_id]->setVel(*msg);
}

void RobotStatus::shutdownPlugin() {
    n.shutdown();
    n_private.shutdown();

}

}  // namespace rqt_example_cpp
PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::RobotStatus, rqt_gui_cpp::Plugin)
