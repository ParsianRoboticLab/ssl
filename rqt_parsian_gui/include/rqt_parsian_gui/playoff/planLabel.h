#ifndef RQT_PLANLABELWIDGET_H
#define RQT_PLANLABELWIDGET_H


#include <ros/ros.h>
#include <ros/package.h>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QStyle>
#include <QString>
#include <QLabel>
#include <QVariant>


namespace rqt_parsian_gui
{

    class PlanLabel : public QWidget {
    Q_OBJECT
    public:
        explicit PlanLabel(bool _put_options);
        virtual ~PlanLabel();
        void create_plan(QString _plan, bool _isActive, bool _isMaster);
        QString plan;
        bool put_options;
        bool isActive;
        bool isMaster;

    public slots:

    protected:

    private:
        //stylesheet
        QFile File;
        QString FormStyleSheet;
        QString resourcePath;
        //widgets
        QHBoxLayout* main;
        QWidget* main_widget;
        QGridLayout* main_layout;
        QPushButton* activate;
        QPushButton* deactivate;
        QPushButton* master;
        QLabel* planName;


    };
}

#endif //RQT_PLANLABELWIDGET_H
