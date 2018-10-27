/*
  Copyright 2016 Lucas Walter
*/

#include <rqt_parsian_gui/monitor.h>
#include <QObject>





namespace rqt_parsian_gui {

    Monitor::Monitor() : rqt_gui_cpp::Plugin(), widget_(0)
//            ,               n(getNodeHandle()), n_private(getPrivateNodeHandle())
    {
        // Constructor is called first before initPlugin function, needless to say.
        // give QObjects reasonable names
        setObjectName("Nadia");
    }
    Monitor::~Monitor() {

    }

    void Monitor::initPlugin(qt_gui_cpp::PluginContext& context) {

        n = getNodeHandle();
        n_private = getPrivateNodeHandle();

        draw_sub = n.subscribe("/analyze_draws", 1000, &Monitor::drawCb, this);
        analysis_sub = n.subscribe("/analysis", 1000, &Monitor::analysisCb, this);
        color_sub = n.subscribe("/team_config", 1000, &Monitor::colorCb, this);
        timer = n.createTimer(ros::Duration(0.3), &Monitor::timerCb, this);
        parsian_msgs::parsian_team_configPtr team_config{new parsian_msgs::parsian_team_config};
        // access standalone command line arguments
        QStringList argv = context.argv();
        // create QWidget
        widget_ = new QWidget();
        drawer = new CguiDrawer();



        widget_->setWindowTitle("nadia");
        ourCol = QColor("blue");
        oppCol = QColor("yellow");


        saveaction = new QAction(this);
        saveaction->setShortcut(*new QKeySequence(tr("Ctrl+L")));
        loadaction = new QAction(this);
        loadaction->setShortcut(*new QKeySequence(tr("Ctrl+R")));

        fieldWidget = new MonitorWidget();
        fieldWidget->addAction(saveaction);
        fieldWidget->addAction(loadaction);
        connect(saveaction, SIGNAL(triggered(bool)), this, SLOT(saveAnalysis()));
        connect(loadaction, SIGNAL(triggered(bool)), this, SLOT(loadAnalysis()));



        analyzeW=new QWidget();

        table=new AnalyzeWidget();
//        xx->setMaximumHeight(30);
//        xx->setMaximumWidth(600);
        auto mainLayout = new QGridLayout();

        mainLayout->addWidget(table,0,0,8,3);






        clearButton=new QPushButton("Clear");
        clearButton->setFixedSize(400,30);
        mainLayout->addWidget(clearButton,6,0,1,3);
        connect(clearButton, SIGNAL(clicked(bool)) ,this, SLOT(clearField()));

        QPushButton *saveButton=new QPushButton("Save Analyze File");
        saveButton->setFixedSize(400,30);
        mainLayout->addWidget(saveButton,7,0,1,3);
        connect(saveButton, SIGNAL(clicked(bool)) ,this, SLOT(saveAnalysis()));

        QPushButton *browsebutton= new QPushButton("Browse");
        mainLayout->addWidget(browsebutton,8,0,1,3);
        connect(browsebutton, SIGNAL(clicked(bool)) ,this, SLOT(loadAnalysis()));

       QCheckBox *tt= new QCheckBox("nadia",analyzeW);
        tt->setEnabled(true);
        mainLayout->addWidget(tt,9,0);
        connect(tt, SIGNAL(clicked()), this, SLOT(changeDrawMode(3)));




        strDraws  << "Draw Shot"<< "Draw Pass" <<"Draw Receiver"<< "Draw Possession" ;
        for(int i=0 ; i<4 ; i++ )
        {
            btnDraws[i] = new QCheckBox(strDraws[i],analyzeW);
            btnDraws[i]->setEnabled(true);
            mainLayout->addWidget(btnDraws[i],10+i%2,i/2+i/2);
            connect(btnDraws[i], SIGNAL(clicked()), this, SLOT(changeDrawMode()));

        }

//        QFile file("FlightParam.csv");
//        if (!file.open(QIODevice::ReadOnly)) {
//        }
//
//        QStringList wordList;
//        while (!file.atEnd()) {
//            QByteArray line = file.readLine();
//            wordList.append(line.split(',').first());
//        }
//
//

        modeChooser=new ModeChooserWidget(n);
        mainLayout->addWidget(modeChooser,8,0,1,3);






        analyzeW->setMaximumHeight(700);
        analyzeW->setMaximumWidth(420);
        analyzeW->setLayout(mainLayout);

        context.addWidget(fieldWidget);
        context.addWidget(analyzeW);


        possessionnumber=1;
        possessionopp=0;
        shotNumber=0;
        shotsucceed=0;
        passNumber=1;
        passsucceed=0;
    }

    void Monitor::saveAnalysis() {
        QChar cc = '0';
        QString suggestionName = QString("%1_%2_%3-%4:%5:%6")
                .arg(QString::number(QDate::currentDate().year()) , 4 , cc)
                .arg(QString::number(QDate::currentDate().month()) , 2 , cc)
                .arg(QString::number(QDate::currentDate().day()) , 2 , cc)
                .arg(QString::number(QTime::currentTime().hour()) , 2 , cc)
                .arg(QString::number(QTime::currentTime().minute()) , 2 , cc)
                .arg(QString::number(QTime::currentTime().second()) , 2 , cc);

            bool ok;
//
//
            QString baseFileName= QInputDialog::getText(fieldWidget, tr("Name") , tr("Enter the log name's: ") , QLineEdit::Normal , suggestionName , &ok);
            if( !ok ) {
                ROS_INFO_STREAM("log file not opened");
                baseFileName= QString("default");
            }
            else {
//                System("mkdir logs/"+suggestionName.toStdString());
                QDir().mkdir("logs/"+baseFileName);
                suggestionName="logs/"+baseFileName+"/"+suggestionName+".csv";
            }

            QFile* saving_file=new QFile();

        saving_file->setFileName(suggestionName);
        if(!saving_file->open(QIODevice::WriteOnly | QIODevice::Append))
            ROS_INFO_STREAM("Can't open the file to possessionFile");
        else
            ROS_INFO_STREAM("possessionFile file opened :) \n");
        std::string s;
        s = ros::package::getPath("rqt_parsian_gui");
        ROS_INFO_STREAM(s + "aa");

    }
    void Monitor::loadAnalysis() {
        ROS_INFO_STREAM("Loaaaad");



        QString fileName;

        fileName = QFileDialog::getOpenFileName(analyzeW, tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));

    }

    void Monitor::colorCb(const parsian_msgs::parsian_team_configConstPtr& _color) {

        mycolor=_color;
        if(mycolor->color){
            ourCol = QColor("blue");
            oppCol = QColor("yellow");
        }
        else{
            ourCol = QColor("yellow");
            oppCol = QColor("blue");
        }


    }


    void Monitor::clearField(){
        ROS_INFO_STREAM("clear");
        drawer->clear();
        fieldWidget->drawerBuffer = drawer;
        fieldWidget->update();


        //init table variables
        QStringList bTitles;
        bTitles<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0";
        QStringList yTitles;
        yTitles<<"0"<<"0"<<"0"<<"0"<<"0"<<"0"<<"0";

        table->updateTable(bTitles,yTitles);

        possessionnumber=1;
        possessionopp=0;
        shotNumber=0;
        shotsucceed=0;
        passNumber=1;
        passsucceed=0;
    }



    void Monitor::changeDrawMode(){
        ROS_INFO_STREAM("gppp");
        for(int i=0;i<4;i++) {

                fieldWidget->drawMode[i] = btnDraws[i]->isChecked();
        }
        fieldWidget->update();

    }

    void Monitor::analysisCb(const parsian_msgs::parsian_statistical_analyzeConstPtr &_analysis) {


        analysisMeassage=_analysis;


        oppCol.setAlpha(200);
        QColor faultcol=QColor("red");
        faultcol.setAlpha(200);

        switch (analysisMeassage->shootOrPassOrPossession){
            case 0://Shot
                shotNumber++;
                if(analysisMeassage->succeed) {
                    shotsucceed++;
                    drawer->drawRobot(0, analysisMeassage->shotter, analysisMeassage->shotDir,
                                      oppCol, analysisMeassage->shotterID, -1, "", true);
                } else{

                    drawer->drawRobot(0, analysisMeassage->shotter, analysisMeassage->shotDir,
                                      faultcol, analysisMeassage->shotterID, -1, "", true);
                }
                break;
            case 1://Pass
                passNumber++;

                if(analysisMeassage->succeed) {
                    passsucceed++;

                    drawer->drawRobot(1, analysisMeassage->shotter, analysisMeassage->shotDir,
                                      oppCol, analysisMeassage->shotterID, -1, "", true);
                    drawer->drawRobot(2, analysisMeassage->receiver, analysisMeassage->shotDir,
                                      oppCol, analysisMeassage->shotterID, -1, "", true);
                }
                else{
                    drawer->drawRobot(1, analysisMeassage->shotter, analysisMeassage->shotDir,
                                      faultcol, analysisMeassage->shotterID, -1, "", true);
                    drawer->drawRobot(2, analysisMeassage->receiver, analysisMeassage->shotDir,
                                      faultcol, analysisMeassage->shotterID, -1, "", true);
                }
                break;
            case 2://Possession
                CguiDrawer::GuiBall ball;
                possessionnumber++;
                if(analysisMeassage->BPSaved==0)
                    possessionopp++;

                ball.pos.x=analysisMeassage->ballPos.x;
                ball.pos.y=analysisMeassage->ballPos.y;
                ball.BP=analysisMeassage->BP;
                ball.Bpsaved=analysisMeassage->BPSaved;
                drawer->balls.enqueue(ball);
                break;
        }





        fieldWidget->drawerBuffer = drawer;


        fieldWidget->update();





    }









    void Monitor::drawCb(const parsian_msgs::parsian_drawConstPtr &_draw) {

        for (parsian_msgs::parsian_draw_circle cir : _draw->circles) {
            drawer->arcBuffer->append(cir);

        }
        for (parsian_msgs::parsian_draw_polygon polygon : _draw->polygons) {
            drawer->polygonBuffer->append(polygon);

        }
        for (parsian_msgs::parsian_draw_rect rect : _draw->rects) {
            drawer->rectBuffer->append(rect);

        }
        for (parsian_msgs::parsian_draw_segment seg : _draw->segments) {
            drawer->segBuffer->append(seg);
        }
        for (parsian_msgs::parsian_draw_text txt : _draw->texts) {
            drawer->textBuffer->append(txt);

        }
        for (parsian_msgs::parsian_draw_vector point : _draw->vectors) {
            drawer->pointBuffer->append(point);
        }

        fieldWidget->drawerBuffer = drawer;

        fieldWidget->update();

    }


    void Monitor::timerCb(const ros::TimerEvent &_timer) {


        bvals.clear();
        yvals.clear();
        bvals<<"0"<<"0"<<"0"<<QString::number(possessionnumber==0?0:(int)((possessionopp/(double)possessionnumber)*100))+"%"
        <<QString::number(shotNumber)
             <<QString::number((int)(shotNumber==0?0:(shotsucceed/(double)shotNumber)*100))+"%"
             <<QString::number((int)(passNumber==0?0:(passsucceed/(double)passNumber)*100))+"%";



        yvals<<"0"<<"0"<<"0"<<QString::number((int)(possessionnumber==0?0:(1-possessionopp/(double)possessionnumber)*100))+"%"
             <<QString::number(shotNumber)
             <<QString::number((int)(shotNumber==0?0:(0.7*shotsucceed/(double)shotNumber)*100))+"%"
             <<QString::number((int)(passNumber==0?0:(0.7*passsucceed/(double)passNumber)*100))+"%";

        table->updateTable(bvals,yvals);



//        fieldWidget->showLogMode(isLogMode,isReplayMode);

//        fieldWidget->drawerBuffer->clear();





//        fieldWidget->drawerBuffer->draw(Circle2D(ballpos, radius), 0, 360, QColor("orange"), true);



//        fieldWidget->drawerBuffer->robotBuffer.clear();
    }



    void Monitor::shutdownPlugin()
    {
        // unregister all publishers here
        ROS_INFO("Monitor closed");
        timer.stop();
        modeChooser->saveTeamConfig();
        n.shutdown();
        n_private.shutdown();


    }


}  // namespace rqt_example_cpp
PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::Monitor, rqt_gui_cpp::Plugin)
