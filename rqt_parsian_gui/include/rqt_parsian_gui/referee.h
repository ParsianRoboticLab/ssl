/*
  Copyright 2016 Lucas Walter
*/


#ifndef RQT_REFEREE_H
#define RQT_REFEREE_H

#include <rqt_gui_cpp/plugin.h>
#include <QPushButton>
#include <rqt_parsian_gui/refereeWidget.h>
#include <pluginlib/class_list_macros.h>



namespace rqt_parsian_gui
{

    class Referee: public rqt_gui_cpp::Plugin
    {
    Q_OBJECT
    public:

        Referee();
        virtual void initPlugin(qt_gui_cpp::PluginContext& context);
        virtual void shutdownPlugin();
    private:
        ros::NodeHandle n;
        ros::NodeHandle n_private;
        RefereeWidget* refereeWidget;
    };
}
#endif  // RQT_REFEREE_H
