/*
  Copyright 2016 Lucas Walter
*/
#ifndef RQT_PARSIAN_GUI_MONITOR_H
#define RQT_PARSIAN_GUI_MONITOR_H

#include <rqt_gui_cpp/plugin.h>
#include <QWidget>
#include <QPushButton>
#include <QAction>
#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <rqt_parsian_gui/monitorWidget.h>
#include <pluginlib/class_list_macros.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_team_config.h>
#include <parsian_util/core/worldmodel.h>
#include <ros/package.h>



namespace rqt_parsian_gui {

class Monitor
    : public rqt_gui_cpp::Plugin {
    Q_OBJECT
public:

    Monitor();
    ~Monitor();
    virtual void initPlugin(qt_gui_cpp::PluginContext& context);
    virtual void shutdownPlugin();

    ros::NodeHandle n;
    ros::NodeHandle n_private;
    ros::NodeHandle n_color;

    ros::Subscriber wm_sub;
    ros::Subscriber log_wm_sub;
    ros::Subscriber draw_sub;
    ros::Subscriber log_draw_sub;
    ros::Subscriber color_sub;

    ros::Timer timer;

        rosbag::Bag *bag;


    double radius = 0.0215;
    bool isLogMode;
    bool isReplayMode;
    QAction* LogMode;
    QAction* ReplayMode;

    QColor ourCol;
    QColor oppCol;


    void wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
    void logwmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
    void drawCb(const parsian_msgs::parsian_drawConstPtr& _draw);
    void logdrawCb(const parsian_msgs::parsian_drawConstPtr& _draw);
    void colorCb(const parsian_msgs::parsian_team_configConstPtr& _color);
    void timerCb(const ros::TimerEvent& _timer);

    // Comment in to signal that the plugin has a way to configure it
    // bool hasConfiguration() const;
    // void triggerConfiguration();
private:
    QWidget* widget_;

    CguiDrawer* drawer;
    CguiDrawer logDrawer;
    CguiDrawer* lastdrawer;
    parsian_msgs::parsian_world_modelConstPtr mywm;
    parsian_msgs::parsian_team_configConstPtr mycolor;
    MonitorWidget* fieldWidget;


public slots:
    void playLog();
    void startLog();

};
}  // namespace rqt_example_cpp
#endif  // RQT_EXAMPLE_CPP_MY_PLUGIN_H
