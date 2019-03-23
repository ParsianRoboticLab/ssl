#ifndef RQT_PLANSVIEWWIDGET_H
#define RQT_PLANSVIEWWIDGET_H


#include <ros/ros.h>
#include <ros/package.h>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QString>
#include <QPushButton>
#include <vector>
#include <string>
#include <rqt_parsian_gui/playoff/planLabel.h>
#include <algorithm>



namespace rqt_parsian_gui
{

    class PlansView : public QScrollArea
    {
        Q_OBJECT
        public:
            explicit PlansView(QScrollArea *parent = 0);
            void add_contact(QString username, bool isActive, bool isMaster, bool no_option = true);
            void setServerUpdateService(ros::ServiceClient& _client);
            void sort();
            void clear_all();


        private:
            ros::ServiceClient* server_update;
            //create ZpContactList widget
            QList<PlanLabel*> contacts_list;
            QVBoxLayout* contacts_list_layout;
            QWidget* contact_list_widget;
            QWidget* filler;
            void resizeEvent(QResizeEvent*);

    };
}

#endif //RQT_PLANSVIEWWIDGET_H
