/*
  Copyright 2016 Lucas Walter
*/


#ifndef RQT_PLAYOFF_H
#define RQT_PLAYOFF_H

#include <ros/ros.h>
#include <rqt_gui_cpp/plugin.h>
#include <pluginlib/class_list_macros.h>
#include <rqt_parsian_gui/playoff/playoffWidget.h>

namespace rqt_parsian_gui
{

    class PlayOff: public rqt_gui_cpp::Plugin
    {
        Q_OBJECT
        public:

            PlayOff();
            virtual void initPlugin(qt_gui_cpp::PluginContext& context);
            virtual void shutdownPlugin();
        private:
            ros::NodeHandle n;
            ros::NodeHandle n_private;
            PlayOffWidget* playoffWidget;
        };
}
#endif  // RQT_PLAYOFF_H
