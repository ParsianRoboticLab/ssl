#ifndef RQT_PLAYOFFWIDGET_H
#define RQT_PLAYOFFWIDGET_H


#include <ros/ros.h>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <rqt_parsian_gui/playoff/planLabel.h>
#include <parsian_msgs/parsian_update_plans.h>



namespace rqt_parsian_gui
{

    class PlayOffWidget : public QWidget {
    Q_OBJECT
    public:
        explicit PlayOffWidget();
        virtual ~PlayOffWidget();
        void create_main_widget();
        void setServerUpdateService(ros::ServiceClient& _client);


    public slots:

    protected:

    private:
        ros::ServiceClient* server_update;

        QGridLayout* main_layout;
        PlanLabel* b1;
        PlanLabel* b2;
        PlanLabel* b3;
        PlanLabel* b4;
        PlanLabel* b5;
        PlanLabel* b6;
        PlanLabel* b7;
        PlanLabel* b8;
        PlanLabel* b9;

    };
}

#endif //RQT_PLAYOFFWIDGET_H
