/*
  Copyright 2016 Lucas Walter
*/


#ifndef RQT_MODE_CHOOSER_H
#define RQT_MODE_CHOOSER_H

#include <rqt_gui_cpp/plugin.h>
#include <QPushButton>
#include <rqt_parsian_gui/modeChooserWidget.h>
#include <pluginlib/class_list_macros.h>



namespace rqt_parsian_gui {

class ModeChooser: public rqt_gui_cpp::Plugin {
    Q_OBJECT
public:

    ModeChooser();
    virtual void initPlugin(qt_gui_cpp::PluginContext& context);
    virtual void shutdownPlugin();
private:
    ros::NodeHandle n;
    ros::NodeHandle n_private;
    ModeChooserWidget* modeWidget;
};
}
#endif  // RQT_MODE_CHOOSER_H
