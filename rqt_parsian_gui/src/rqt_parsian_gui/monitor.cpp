/*
  Copyright 2016 Lucas Walter
*/

#include <rqt_parsian_gui/monitor.h>
#include <rqt_parsian_gui/guiDrawer.h>
#include <QApplication>
#include <QThread>
#include <QTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QDir>



namespace rqt_parsian_gui
{

    Monitor::Monitor() : rqt_gui_cpp::Plugin(), widget_(0)
//            ,               n(getNodeHandle()), n_private(getPrivateNodeHandle())
    {
        // Constructor is called first before initPlugin function, needless to say.
        // give QObjects reasonable names
        setObjectName("Nadia");
    }
    Monitor::~Monitor(){

    }

    void Monitor::initPlugin(qt_gui_cpp::PluginContext& context)
    {

        n = getNodeHandle();
        n_private = getPrivateNodeHandle();

        wm_sub = n.subscribe("/world_model", 1000, &Monitor::wmCb, this);
        log_wm_sub = n.subscribe("/log/world_model", 1000, &Monitor::logwmCb, this);
        draw_sub = n.subscribe("/draws", 1000, &Monitor::drawCb, this);
        log_draw_sub = n.subscribe("/log/draws", 1000, &Monitor::logdrawCb, this);
        color_sub = n.subscribe("/team_config", 1000, &Monitor::colorCb, this);
        timer = n.createTimer(ros::Duration(0.080), &Monitor::timerCb, this);
        parsian_msgs::parsian_team_configPtr team_config{new parsian_msgs::parsian_team_config};
        // access standalone command line arguments
        QStringList argv = context.argv();
        // create QWidget
        widget_ = new QWidget();
        drawer=new CguiDrawer();
        lastdrawer=new CguiDrawer();


        widget_->setWindowTitle("nadia");
        ourCol = QColor("blue");
        oppCol = QColor("yellow");


        LogMode = new QAction(this);
        LogMode->setShortcut(*new QKeySequence(tr("Ctrl+L")));
        ReplayMode = new QAction(this);
        ReplayMode->setShortcut(*new QKeySequence(tr("Ctrl+R")));
        isLogMode= false;
        isReplayMode=false;

        fieldWidget=new MonitorWidget();
        fieldWidget->addAction(LogMode);
        fieldWidget->addAction(ReplayMode);
        connect(LogMode, SIGNAL(triggered(bool)), this, SLOT(startLog()));
        connect(ReplayMode, SIGNAL(triggered(bool)), this, SLOT(playLog()));



        context.addWidget(fieldWidget);
    }

    void Monitor::startLog(){
        ROS_INFO_STREAM("log mode");
        if(!isLogMode) {
            isLogMode = true;
            isReplayMode=false;
            QChar cc = '0';
            QString suggestionName = QString("%1_%2_%3-%4:%5:%6")
                    .arg(QString::number(QDate::currentDate().year()) , 4 , cc)
                    .arg(QString::number(QDate::currentDate().month()) , 2 , cc)
                    .arg(QString::number(QDate::currentDate().day()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().hour()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().minute()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().second()) , 2 , cc);

//            bool ok;
//
//
//            QString baseFileName= QInputDialog::getText(fieldWidget, tr("Name") , tr("Enter the log name's: ") , QLineEdit::Normal , suggestionName , &ok);
//            if( !ok ) {
//                ROS_INFO_STREAM("log file not opened");
//                baseFileName= QString("default");
//            }
//            else {
////                System("mkdir logs/"+suggestionName.toStdString());
//                QDir().mkdir("logs/"+baseFileName);
//                suggestionName="logs/"+baseFileName+"/"+suggestionName+".bag";
//            }
            bag.open("logs/"+suggestionName.toStdString()+".bag", rosbag::bagmode::Write);

        }

    }
    void Monitor::playLog(){
        ROS_INFO_STREAM("replay mode");
        if(!isReplayMode){
            isReplayMode = true;
        }
        else{
            isReplayMode = false;
        }

        if(isLogMode) {
            isLogMode = false;
            bag.close();
        }
    }


    void Monitor::colorCb(const parsian_msgs::parsian_team_configConstPtr& _color){
        
        mycolor=_color;
        if(mycolor->color==0){
            ourCol = QColor("blue");
            oppCol = QColor("yellow");
        }
        else{
            ourCol = QColor("yellow");
            oppCol = QColor("blue");
        }

    }

    void Monitor::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {




        if(isLogMode) {
            mywm = _wm;


            bag.write("log/world_model", ros::Time::now(), mywm);
        }
        if(!isReplayMode){

            mywm = _wm;

            drawer = new CguiDrawer();
//        drawer->polygonBuffer=lastdrawer->polygonBuffer;
//        drawer->rectBuffer=lastdrawer->rectBuffer;
//        drawer->pointBuffer=lastdrawer->pointBuffer;
//        drawer->segBuffer=lastdrawer->segBuffer;





            drawer->guiBall.inSight = mywm->ball.inSight;
            drawer->guiBall.pos.x = mywm->ball.pos.x;
            drawer->guiBall.pos.y = mywm->ball.pos.y;
            drawer->guiBall.radius = mywm->ball.obstacleRadius;
            for (int i = 0; i < mywm->our.size(); i++) {
                if (fabs(mywm->our[i].inSight - 0.5) < 0.01) {
                    ourCol.setAlpha(150);
                }
                drawer->drawRobot(mywm->our[i].pos, mywm->our[i].dir,
                                  ourCol, mywm->our[i].id, i, "", false);

                //        if (soccer->agents[wm->our.active(i)->id]->goalVisibility>0)
                //            draw(QString::number(soccer->agents[wm->our.active(i)->id]->goalVisibility,'f',2), wm->our.active(i)->pos + Vector2D(-0.3, -0.1), QColor("black"), 14);


            }


            for (const auto &i : mywm->opp) {


                if (fabs(i.inSight - 0.5) < 0.01) {
                    oppCol.setAlpha(150);
                }


                drawer->drawRobot(i.pos, i.dir,
                                  oppCol, i.id, -1);

            }
//        fieldWidget->update();

        }


    }



    void Monitor::logwmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {




        if(isReplayMode) {
            mywm = _wm;

            drawer = new CguiDrawer();



            drawer->guiBall.inSight = mywm->ball.inSight;
            drawer->guiBall.pos.x = mywm->ball.pos.x;
            drawer->guiBall.pos.y = mywm->ball.pos.y;
            drawer->guiBall.radius = mywm->ball.obstacleRadius;
            for (int i = 0; i < mywm->our.size(); i++) {
                if (fabs(mywm->our[i].inSight - 0.5) < 0.01) {
                    ourCol.setAlpha(150);
                }
                drawer->drawRobot(mywm->our[i].pos, mywm->our[i].dir,
                                  ourCol, mywm->our[i].id, i, "", false);

                //        if (soccer->agents[wm->our.active(i)->id]->goalVisibility>0)
                //            draw(QString::number(soccer->agents[wm->our.active(i)->id]->goalVisibility,'f',2), wm->our.active(i)->pos + Vector2D(-0.3, -0.1), QColor("black"), 14);


            }


            for (const auto &i : mywm->opp) {


                if (fabs(i.inSight - 0.5) < 0.01) {
                    oppCol.setAlpha(150);
                }


                drawer->drawRobot(i.pos, i.dir,
                                  oppCol, i.id, -1);

            }
//        fieldWidget->update();

        }


    }




    void Monitor::drawCb(const parsian_msgs::parsian_drawConstPtr &_draw) {
        if(isLogMode) {
            bag.write("log/draws", ros::Time::now(), _draw);
        }

        if(!isReplayMode) {

            for (parsian_msgs::parsian_draw_circle cir: _draw->circles) {
                lastdrawer->arcBuffer->append(cir);

            }
            for (parsian_msgs::parsian_draw_polygon polygon: _draw->polygons) {
                lastdrawer->polygonBuffer->append(polygon);

            }
            for (parsian_msgs::parsian_draw_rect rect: _draw->rects) {
                lastdrawer->rectBuffer->append(rect);

            }
            for (parsian_msgs::parsian_draw_segment seg: _draw->segments) {
                lastdrawer->segBuffer->append(seg);
            }
            for (parsian_msgs::parsian_draw_text txt: _draw->texts) {
                lastdrawer->textBuffer->append(txt);

            }
            for (parsian_msgs::parsian_draw_vector point: _draw->vectors) {
                lastdrawer->pointBuffer->append(point);

            }
        }
//        fieldWidget->update();

    }


    void Monitor::logdrawCb(const parsian_msgs::parsian_drawConstPtr &_draw) {
        if(isReplayMode) {

            for (parsian_msgs::parsian_draw_circle cir: _draw->circles) {
                lastdrawer->arcBuffer->append(cir);

            }
            for (parsian_msgs::parsian_draw_polygon polygon: _draw->polygons) {
                lastdrawer->polygonBuffer->append(polygon);

            }
            for (parsian_msgs::parsian_draw_rect rect: _draw->rects) {
                lastdrawer->rectBuffer->append(rect);

            }
            for (parsian_msgs::parsian_draw_segment seg: _draw->segments) {
                lastdrawer->segBuffer->append(seg);
            }
            for (parsian_msgs::parsian_draw_text txt: _draw->texts) {
                lastdrawer->textBuffer->append(txt);

            }
            for (parsian_msgs::parsian_draw_vector point: _draw->vectors) {
                lastdrawer->pointBuffer->append(point);

            }
        }
//        fieldWidget->update();

    }

    void Monitor::timerCb(const ros::TimerEvent &_timer) {

//        fieldWidget->drawerBuffer->clear();
            drawer->arcBuffer = lastdrawer->arcBuffer;
            drawer->segBuffer = lastdrawer->segBuffer;
            drawer->pointBuffer = lastdrawer->pointBuffer;
            drawer->textBuffer = lastdrawer->textBuffer;
            drawer->rectBuffer = lastdrawer->rectBuffer;
            drawer->polygonBuffer = lastdrawer->polygonBuffer;
            lastdrawer = drawer;
            fieldWidget->drawerBuffer = lastdrawer;

//        fieldWidget->drawerBuffer->draw(Circle2D(ballpos, radius), 0, 360, QColor("orange"), true);

            fieldWidget->update();
        if(isReplayMode && mywm!= nullptr) {
            logwmCb(mywm);
        }


//        fieldWidget->drawerBuffer->robotBuffer.clear();
    }


    void Monitor::shutdownPlugin()
    {
        // unregister all publishers here
        ROS_INFO("Monitor closed");
        timer.stop();
        n.shutdown();
        n_private.shutdown();

    }


}  // namespace rqt_example_cpp
PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::Monitor, rqt_gui_cpp::Plugin)
