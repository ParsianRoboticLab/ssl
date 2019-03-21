#ifndef RQT_PLAYOFFWIDGET_H
#define RQT_PLAYOFFWIDGET_H


#include <ros/ros.h>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>


namespace rqt_parsian_gui
{

    class PlayOffWidget : public QWidget {
    Q_OBJECT
    public:
        explicit PlayOffWidget();
        virtual ~PlayOffWidget();
        void create_main_widget();

    public slots:

    protected:

    private:
        QGridLayout* main_layout;
        QPushButton* b1;
        QPushButton* b2;
        QPushButton* b3;
        QPushButton* b4;
        QPushButton* b5;
        QPushButton* b6;
        QPushButton* b7;
        QPushButton* b8;
        QPushButton* b9;

    };
}

#endif //RQT_PLAYOFFWIDGET_H
