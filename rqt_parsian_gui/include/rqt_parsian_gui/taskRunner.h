/*
  Copyright 2016 Lucas Walter
*/


#ifndef RQT_TASKRUNNER_MONITOR_H
#define RQT_TASKRUNNER_MONITOR_H

#include <rqt_gui_cpp/plugin.h>
#include <QPushButton>
#include <rqt_parsian_gui/taskRunnerWidget.h>
#include <pluginlib/class_list_macros.h>



namespace rqt_parsian_gui
{

    class TaskRunner: public rqt_gui_cpp::Plugin
    {
    Q_OBJECT
    public:

        TaskRunner();
        virtual void initPlugin(qt_gui_cpp::PluginContext& context);
        virtual void shutdownPlugin();
    private:
        ros::NodeHandle n;
        ros::NodeHandle n_private;
        TaskRunnerWidget* taskRunnerWidget;
    };
}
#endif  // RQT_TASKRUNNER_MONITOR_H
