#ifndef RQT_TABWIDGETWIDGET_H
#define RQT_TABWIDGETWIDGET_H


#include <ros/ros.h>
#include <ros/package.h>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QString>
#include <QPushButton>
#include <vector>
#include <string>
#include <algorithm>
#include <QFile>



namespace rqt_parsian_gui
{

    class TabWidget : public QScrollArea
    {
        Q_OBJECT
        public:
            explicit TabWidget(QScrollArea *parent = 0);
            void add_contact(QString name);
            void sort();
            QPushButton* get_contact(QString name);


        private:
            //stylesheet
            QFile File;
            QString FormStyleSheet;
            QString resourcePath;
            //create ZpContactList widget
            QList<QPushButton*> contacts_list;
            QVBoxLayout* contacts_list_layout;
            QWidget* contact_list_widget;
            QWidget* filler;
            void resizeEvent(QResizeEvent*);

    };
}

#endif //RQT_TABWIDGETWIDGET_H
