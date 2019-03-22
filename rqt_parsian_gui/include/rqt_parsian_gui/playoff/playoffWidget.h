#ifndef RQT_PLAYOFFWIDGET_H
#define RQT_PLAYOFFWIDGET_H


#include <ros/ros.h>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <rqt_parsian_gui/playoff/planLabel.h>
#include <rqt_parsian_gui/playoff/plansView.h>
#include <parsian_msgs/parsian_update_plans.h>
#include <vector>
#include <string>



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

    };
}

#endif //RQT_PLAYOFFWIDGET_H
