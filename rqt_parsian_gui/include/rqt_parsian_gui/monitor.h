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
#include <rqt_parsian_gui/modeChooserWidget.h>
#include <pluginlib/class_list_macros.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_team_config.h>
#include <parsian_msgs/parsian_draw_monitor.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_msgs/parsian_statistical_analyze.h>
#include <ros/package.h>
#include <rqt_parsian_gui/analyzeWidget.h>
#include <rqt_parsian_gui/guiDrawer.h>
#include <QApplication>
#include <QThread>
#include <QTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QDir>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QRadioButton>
#include <QFileDialog>
#include <QAbstractTableModel>
#include <QString>



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

    ros::Subscriber draw_sub;
    ros::Subscriber color_sub;
    ros::Subscriber analysis_sub;

    ros::Timer timer;



    double radius = 0.0215;

    QAction* saveaction;
    QAction* loadaction;

    QColor ourCol;
    QColor oppCol;

    int shotNumber,shotsucceed,passNumber,passsucceed,possessionnumber,possessionopp;
    AnalyzeWidget *table;
    QStringList bvals,yvals;
    QPushButton *clearButton;
    QCheckBox *btnDraws[4];
    QStringList strDraws;

    void analysisCb(const parsian_msgs::parsian_statistical_analyzeConstPtr& _analysis);
    void drawCb(const parsian_msgs::parsian_draw_monitorConstPtr& _draw);
    void colorCb(const parsian_msgs::parsian_team_configConstPtr& _color);
    void timerCb(const ros::TimerEvent& _timer);

    // Comment in to signal that the plugin has a way to configure it
    // bool hasConfiguration() const;
    // void triggerConfiguration();
private:
    QWidget* widget_;
    ModeChooserWidget *modeChooser;

    CguiDrawer* drawer;
    parsian_msgs::parsian_statistical_analyzeConstPtr analysisMeassage;
    parsian_msgs::parsian_team_configConstPtr mycolor;
    MonitorWidget* fieldWidget;
    QWidget *analyzeW;


public slots:
    void loadAnalysis();
    void saveAnalysis();
    void clearField();
    void changeDrawMode();


};
}  // namespace rqt_example_cpp
#endif  // RQT_EXAMPLE_CPP_MY_PLUGIN_H
