//
// Created by mahi on 10/12/18.
//

#include <rqt_parsian_gui/graphical/logger.h>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QVariant>
#include <QSlider>

Logger::Logger() {
    auto* lay = new QVBoxLayout();
    setLayout(lay);
    auto* btnLay = new QHBoxLayout();
    auto *btnRefGroup = new QButtonGroup();
    QString strs[] = {"Record", "Back", "Play", "Forward"};
    for (const auto& s : strs) {
        auto* btn = new QPushButton(s, this);
        btnRefGroup->addButton(btn);
        btnLay->addWidget(btn);
    }

    auto* pb = new QSlider(Qt::Horizontal, this);
    lay->addLayout(btnLay);
    lay->addWidget(pb);
//    auto *enableCHB = new QCheckBox("enable");
////    strRefNames << "H" << "FS" << "S" << "NS" << "FK" << "IK" << "KO" << "PK" << "BP" << "FK" << "IK" << "KO" << "PK" << "BP";
////    strRefCommands << "H" << "s" << "S" << " " << "F" << "I" << "K" << "P" << "B" << "f" << "i" << "k" << "p" << "b";
//    for(int i=0 ; i<14 ; i++ )
//    {
////        auto* btnRefs = new QPushButton(strRefNames[i],this);
//        QString strType = "n";
//        if( i >=4 )
//            strType = "b";
//        if( i >=9 )
//            strType = "y";
////        btnRefs->setProperty("refType" , QVariant::fromValue(strType));
////        btnRefGroup->addButton(btnRefs , i);
//    }
//
////    for (auto &btnRef : btnRefs) {
////        mainLayout->addWidget(btnRefGroup);
////    }
//    double widgetWidth = 300;
//    QString strWidth = QString("%1px").arg((int) (widgetWidth / 12));
//    //StyleSheet
//    QString styleSheet =
//            QString("\
//                    QPushButton[refType=\"n\"] {background-color:gray; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %1} \
//            QPushButton[refType=\"b\"] {background-color:blue; color:white; padding: 0; border: 1px solid black; border-radius: 5px; width: %2} \
//            QPushButton[refType=\"y\"] {background-color:yellow; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %3} \
//            QPushButton[refType=\"n\"]:hover {border: 2px solid red} \
//            QPushButton[refType=\"b\"]:hover {border: 2px solid red} \
//            QPushButton[refType=\"y\"]:hover {border: 2px solid red} \
//            ").arg(strWidth).arg(strWidth).arg(strWidth);
//
//    this->setStyleSheet(styleSheet);
//    connect(btnRefGroup , SIGNAL(buttonClicked(int)) , this , SLOT(SetManualGS(int)));
//    connect(enableCHB , SIGNAL(stateChanged(int)) , this , SLOT(SetEnable(int)));

}
