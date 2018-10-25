//
// Created by parsian-ai on 10/24/18.
//

#include "rqt_parsian_gui/analyzeWidget.h"
namespace rqt_parsian_gui {

    AnalyzeWidget::~AnalyzeWidget() {}

    AnalyzeWidget::AnalyzeWidget()
            :QWidget()
            {

        table = new QTableWidget(7, 3,this);
        QStringList tableheader;

        //setup table horizontal header
        tableheader << "Blue Team" << "Analysis" << "Yellow Team";
        table->setHorizontalHeaderLabels(tableheader);


        //make the table beautiful
        table->setStyleSheet(
                "QTableView {background-color: qlineargradient(x1: 0, y1: 0, x2: 0.5, y2: 0.8,stop: 0.3 #9999ff, stop: 1 #5555ff); color:#880000; font-size:16px;}"
        );
        
        
        //design Frame & Hide Vertical Header
        table->setFrameStyle(QFrame::NoFrame);
        QHeaderView *vh = new QHeaderView(Qt::Vertical);
        table->setColumnWidth(1, 200);
        vh->hide();
        table->setVerticalHeader(vh);

        //setup table elements prototype
        prototype = new QTableWidgetItem();
        prototype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        prototype->setTextAlignment(Qt::AlignCenter);
        
        
        //add analysis column elements
        QStringList analysisTitles;
        analysisTitles << "Goals" << "Yellow Cards" << "Penalties" << "Possession" << "Shots" << "Shot Succeed"<<"Pass Succeed";
        for (int i = 0; i < table->rowCount(); i++) {
            QTableWidgetItem *item = prototype->clone();
            // set the special features of each item
            item->setText(analysisTitles[i]);
            table->setItem(i, 1, item);
        }


        //init table variables
        QStringList bTitles;
        bTitles<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0";
        QStringList yTitles;
        yTitles<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0";

        updateTable(bTitles,yTitles);


        //set widget size
        table->setFixedSize(400,250);
    }
    
    
    
    void AnalyzeWidget::updateTable(QStringList bTitles,QStringList yTitles) {


        //add blue items
        for(int i=0; i<table->rowCount();i++) {
            QTableWidgetItem *item = prototype->clone();
            // set the special features of each item
            item->setText(bTitles[i]);
            table->setItem(i,0,item);
        }


        //add yellow items
        for(int i=0; i<table->rowCount();i++) {
            QTableWidgetItem *item = prototype->clone();
            // set the special features of each item
            item->setText(yTitles[i]);
            table->setItem(i,2,item);
        }
        table->update();
    }

}