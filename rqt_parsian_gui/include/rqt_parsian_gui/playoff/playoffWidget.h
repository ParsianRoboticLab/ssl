#ifndef RQT_PLAYOFFWIDGET_H
#define RQT_PLAYOFFWIDGET_H


#include <ros/ros.h>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <rqt_parsian_gui/playoff/planLabel.h>
#include <rqt_parsian_gui/playoff/plansView.h>
#include <parsian_msgs/parsian_update_plans.h>
#include <rqt_parsian_gui/playoff/tabWidget.h>
#include <parsian_msgs/parsian_playoff_client.h>
#include <vector>
#include <string>
#include <algorithm>



namespace rqt_parsian_gui
{

    class PlayOffWidget : public QWidget {
    Q_OBJECT
    public:
        explicit PlayOffWidget();
        virtual ~PlayOffWidget();
        void create_main_widget();
        void setServerUpdateService(ros::ServiceClient& _client);
        void subscribe(const parsian_msgs::parsian_playoff_clientConstPtr &msg);



    public slots:
        void all_pressed();
        void active_pressed();
        void ignored_pressed();

        void activate_all_pressed();
        void deactivate_all_pressed();
        void demaster_pressed();

        void subscribe_slot();


    signals:
        void subscribe_sig();

    protected:

    private:
        ros::ServiceClient* server_update;
        QGridLayout* main_layout;
        PlansView* all_plansView;
        PlansView* active_plansView;
        PlansView* ignored_plansView;
        QWidget* prev_view;
        QWidget* plansView_widg;
        QVBoxLayout* plansView_lay;

        TabWidget* tabWidget;
        QLabel* last_plan_title;
        PlanLabel* last_plan;
        QLabel* master_plan_title;
        PlanLabel* master_plan;
        QPushButton* activate_all;
        QPushButton* deactivate_all;
        QPushButton* demaster;

        std::vector<std::string> allPlan;
        std::vector<std::string> activePlan;
        std::vector<std::string> ignoredPlan;
        std::string masterPlan;
        std::string lastPlan;




    };
}

#endif //RQT_PLAYOFFWIDGET_H
