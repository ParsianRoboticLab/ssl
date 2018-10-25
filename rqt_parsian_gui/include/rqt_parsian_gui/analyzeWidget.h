//
// Created by parsian-ai on 10/24/18.
//

#ifndef RQT_PARSIAN_GUI_ANALYZEWIDGET_H
#define RQT_PARSIAN_GUI_ANALYZEWIDGET_H


#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>


namespace rqt_parsian_gui
{
    class AnalyzeWidget :public QWidget {
        Q_OBJECT
    public:
        AnalyzeWidget();
        ~AnalyzeWidget();
        void updateTable(QStringList bTitles,QStringList yTitles);







    public slots:


    protected:
        QTableWidget *table;
        QTableWidgetItem *prototype;


    private:

    };




}  // namespace rqt_example_cpp


#endif //RQT_PARSIAN_GUI_ANALYZEWIDGET_H
