//
// Created by parsian-ai on 12/31/17.
//

#ifndef RQT_PARSIAN_GUI_PLAYOFF_H
#define RQT_PARSIAN_GUI_PLAYOFF_H
#include <rqt_gui_cpp/plugin.h>
#include <rqt_parsian_gui/playoffWidget.h>
#include <pluginlib/class_list_macros.h>
#include <parsian_msgs/parsian_update_plans.h>
#include <ros/ros.h>

namespace rqt_parsian_gui {


class PlayOff : public rqt_gui_cpp::Plugin {
    Q_OBJECT
public:
    PlayOff();
    virtual void initPlugin(qt_gui_cpp::PluginContext& context);

private:
    ros::NodeHandle n;
    ros::NodeHandle n_private;
    boost::shared_ptr<PlayOffWidget> playOffWidget;

};
}  // namespace rqt_example_cpp

#endif //RQT_PARSIAN_GUI_PLAYOFF_H
