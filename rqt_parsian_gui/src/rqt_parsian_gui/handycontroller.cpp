/*
  Copyright 2016 Lucas Walter
*/

#include <rqt_parsian_gui/handycontroller.h>


namespace rqt_parsian_gui
{

    HandyController::HandyController() : rqt_gui_cpp::Plugin(), widget_(0)
    {

        qApp->installEventFilter(this);
        //this->installEventFilter(this);
        // Constructor is called first before initPlugin function, needless to say.
        // give QObjects reasonable names
        setObjectName("handycontroller");
        cmd.reset(new parsian_msgs::parsian_robot_command);
    }

    void HandyController::initPlugin(qt_gui_cpp::PluginContext& context)
    {

        ROS_INFO("handy controller begin");
        // access standalone command line arguments
        QStringList argv = context.argv();

        // create QWidget
        widget_ = new QWidget();
        widget_->setWindowTitle("handycontroller");
        widget_->setFixedSize(450, 450);
        // extend the widget with all attributes and children from UI file
        mUI.setupUi(widget_);

        //initial value
        agentID = mUI.Agent_combobox->currentText().toInt();        cmd->robot_id = agentID;
        speed = mUI.Speed_slide->sliderPosition();                  //no msg for it should effect in wheelspwwds
        kickspeed = mUI.Kickspeed_slide->sliderPosition();          cmd->kickSpeed = 0;//kickspeed;
        chipkickspeed = mUI.ChipKickSpeed_slide->sliderPosition();  cmd->kickspeedz = 0;//chipkickspeed;
        rollerspeed = mUI.RollerSpeed_slide->sliderPosition();      cmd->roller_speed = 0;
        cmd->wheelsspeed = true;
        cmd->spinner = false;
        cmd->chip = false;

        wmTimer = new QTimer();
        wmtimedout = false;

        std::string path = ros::package::getPath("rqt_parsian_gui");
        pkgPath = QString::fromStdString(path);

        //connect signal and slots
        connect(mUI.Speed_slide, SIGNAL(valueChanged(int)), mUI.Speed_show, SLOT(setNum(int)));
        connect(mUI.Kickspeed_slide, SIGNAL(valueChanged(int)), mUI.Kickspeed_show, SLOT(setNum(int)));
        connect(mUI.RollerSpeed_slide, SIGNAL(valueChanged(int)), mUI.RollerSpeed_show, SLOT(setNum(int)));
        connect(mUI.ChipKickSpeed_slide, SIGNAL(valueChanged(int)), mUI.ChipKickSpeed_show, SLOT(setNum(int)));
        connect(mUI.Left, SIGNAL(pressed()), this, SLOT(leftpressed()));
        connect(mUI.Up, SIGNAL(pressed()), this, SLOT(uppressed()));
        connect(mUI.Right, SIGNAL(pressed()), this, SLOT(rightpressed()));
        connect(mUI.Down, SIGNAL(pressed()), this, SLOT(downpressed()));
        connect(mUI.Angle1, SIGNAL(pressed()), this, SLOT(angle1pressed()));
        connect(mUI.Angle2, SIGNAL(pressed()), this, SLOT(angle2pressed()));
        connect(mUI.Kick, SIGNAL(pressed()), this, SLOT(kickpressed()));
        connect(mUI.Chip, SIGNAL(pressed()), this, SLOT(chippressed()));
        connect(mUI.Roller, SIGNAL(pressed()), this, SLOT(rollerpressed()));
        connect(mUI.Halt, SIGNAL(pressed()), this, SLOT(haltpressed()));
        connect(mUI.Left, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Up, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Right, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Down, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Angle1, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Angle2, SIGNAL(released()), this, SLOT(releasednav()));
        connect(mUI.Kick, SIGNAL(released()), this, SLOT(releasedother()));
        connect(mUI.Chip, SIGNAL(released()), this, SLOT(releasedother()));
        connect(mUI.Roller, SIGNAL(released()), this, SLOT(releasedother()));
        connect(mUI.Agent_combobox, SIGNAL(activated(QString)),this, SLOT(agentidchanged(QString)));
        connect(mUI.Speed_slide, SIGNAL(valueChanged(int)), this, SLOT(speedchanged(int)));
        connect(mUI.Kickspeed_slide, SIGNAL(valueChanged(int)), this, SLOT(kickspeedchanged(int)));
        connect(mUI.ChipKickSpeed_slide, SIGNAL(valueChanged(int)), this, SLOT(chipkickspeedchanged(int)));
        connect(mUI.RollerSpeed_slide, SIGNAL(valueChanged(int)), this, SLOT(rollerspeedchanged(int)));

        mUI.Kick->setIcon(QIcon(pkgPath + "/resource/icons/kick_off.png"));
        mUI.Chip->setIcon(QIcon(pkgPath + "/resource/icons/chip_off.png"));
        mUI.Up->setIcon(QIcon(pkgPath + "/resource/icons/red_up.png"));
        mUI.Left->setIcon(QIcon(pkgPath + "/resource/icons/red_left.png"));
        mUI.Down->setIcon(QIcon(pkgPath + "/resource/icons/red_down.png"));
        mUI.Right->setIcon(QIcon(pkgPath + "/resource/icons/red_right.png"));
        mUI.Angle1->setIcon(QIcon(pkgPath + "/resource/icons/red_ccw.png"));
        mUI.Angle2->setIcon(QIcon(pkgPath + "/resource/icons/red_cw.png"));
        mUI.Roller->setIcon(QIcon(pkgPath + "/resource/icons/roller_off.png"));
        mUI.Halt->setIcon(QIcon(pkgPath + "/resource/icons/stop_on.png"));




        //initial publishing
        nh = getNodeHandle();
        nh_private = getPrivateNodeHandle();
        timer_ = nh.createTimer(ros::Duration(0.0166), boost::bind(& HandyController::timerCb, this, _1));
        m_wm_sub = nh.subscribe<parsian_msgs::parsian_world_model>("/world_model", 1000, boost::bind(& HandyController::m_wmCb, this, _1));
        publisher  = nh.advertise<parsian_msgs::parsian_robot_command>("/agent_0/command", 1000);
        wmpublisher  = nh.advertise<parsian_msgs::parsian_world_model>("/world_model", 1000);

        connect(wmTimer, SIGNAL(timeout()), this, SLOT(wmtimedOut()));
        wmTimer->start(2000);
        connect(this, SIGNAL(startwmtimer(int)), wmTimer, SLOT(start(int)));
        connect(this, SIGNAL(stopwmtimer()), wmTimer, SLOT(stop()));


        // add widget to the user interface
        context.addWidget(widget_);
        widget_->show();


    }

    void HandyController::timerCb(const ros::TimerEvent& event){
        // Using timers is the preferred 'ROS way' to manual threading
        publisher.publish(cmd);
        //ROS_INFO_STREAM(wmtimedout);
        if(wmtimedout)
        {
            parsian_msgs::parsian_world_model wm;
            wm.ball.id = 100;
            wmpublisher.publish(wm);
        }
    }

    void HandyController::m_wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm){
        if(_wm->ball.id != 100)
        {
            //ROS_INFO("wm in");
            emit stopwmtimer();
            wmtimedout = false;
            emit startwmtimer(2000);
        }

    }

    void HandyController::wmtimedOut()
    {
       // ROS_INFO("wmtimedout");
        wmtimedout = true;
    }

    void HandyController::shutdownPlugin()
    {
        // unregister all publishers here
        publisher.shutdown();
        m_wm_sub.shutdown();
        wmpublisher.shutdown();
        timer_.stop();
        wmTimer->disconnect();
        nh.shutdown();
        nh_private.shutdown();



    }

    void HandyController::saveSettings(qt_gui_cpp::Settings& plugin_settings,
                               qt_gui_cpp::Settings& instance_settings) const
    {
        // instance_settings.setValue(k, v)
    }

    void HandyController::restoreSettings(const qt_gui_cpp::Settings& plugin_settings,
                                  const qt_gui_cpp::Settings& instance_settings)
    {
        // v = instance_settings.value(k)
    }

    void HandyController::takeAction(EActionType action)
    {
        switch(action)
        {
        case EActionType::_STOP_NAVIGATION:         //TODO better to be checked
        {
            //double r = 0;
            //r = cmd->roller_speed;
            waitHere();
            //cmd->roller_speed = r;

            mUI.Halt->setIcon(QIcon(pkgPath + "/resource/icons/stop_on.png"));
            mUI.Up->setIcon(QIcon(pkgPath + "/resource/icons/red_up.png"));
            mUI.Left->setIcon(QIcon(pkgPath + "/resource/icons/red_left.png"));
            mUI.Down->setIcon(QIcon(pkgPath + "/resource/icons/red_down.png"));
            mUI.Right->setIcon(QIcon(pkgPath + "/resource/icons/red_right.png"));
            mUI.Angle1->setIcon(QIcon(pkgPath + "/resource/icons/red_ccw.png"));
            mUI.Angle2->setIcon(QIcon(pkgPath + "/resource/icons/red_cw.png"));
            break;
        }
        case EActionType::_STOP_OTHER:              //TODO better to be checked
            waitHere();
//            cmd->kickSpeed = 0;
//            cmd->kickspeedz = 0;
//            cmd->chip = false;

            mUI.Kick->setIcon(QIcon(pkgPath + "/resource/icons/kick_off.png"));
            mUI.Chip->setIcon(QIcon(pkgPath + "/resource/icons/chip_off.png"));
            break;

        case EActionType::_FORWARD:
            mUI.Up->setIcon(QIcon(pkgPath + "/resource/icons/green_up.png"));
            setRobotVel(speed*4.2 / 127.0, 0.0, 0.0);
            break;

        case EActionType::_BACKWARD:
            mUI.Down->setIcon(QIcon(pkgPath + "/resource/icons/green_down.png"));
            setRobotVel(-speed*4.2 / 127.0, 0.0, 0.0);
            break;

        case EActionType::_LEFT:
            mUI.Left->setIcon(QIcon(pkgPath + "/resource/icons/green_left.png"));
            setRobotVel(0.0, speed*4.2 / 127.0, 0.0);

            break;

        case EActionType::_RIGHT:
            mUI.Right->setIcon(QIcon(pkgPath + "/resource/icons/green_right.png"));
            setRobotVel(0.0, -speed*4.2 / 127.0, 0.0);
            break;

        case EActionType::_TURN_CCW:
           mUI.Angle1->setIcon(QIcon(pkgPath + "/resource/icons/green_ccw.png"));
            setRobotVel(0.0, 0.0, speed/20.0 );
            break;

        case EActionType::_TURN_CW:
           mUI.Angle2->setIcon(QIcon(pkgPath + "/resource/icons/green_cw.png"));
            setRobotVel(0.0, 0.0,-speed/20.0);
            break;

        case EActionType::_KICK:

           mUI.Kick->setIcon(QIcon(pkgPath + "/resource/icons/kick_on.png"));
            cmd->kickSpeed = kickspeed;
            break;

        case EActionType::_CHIP:
            mUI.Chip->setIcon(QIcon(pkgPath + "/resource/icons/chip_on.png"));
            cmd->chip = true;
            cmd->kickspeedz = chipkickspeed;
            break;

        case EActionType::_ROLLER:  //TODO some issue when robot id changes
            if ( cmd->spinner )
            {
                mUI.Roller->setIcon(QIcon(pkgPath + "/resource/icons/roller_off.png"));
                cmd->spinner = false;
                cmd->roller_speed = 0;
                //ROS_INFO("spin off");
            }
            else
            {
                mUI.Roller->setIcon(QIcon(pkgPath + "/resource/icons/roller_on.png"));
                cmd->spinner = true;
                cmd->roller_speed = rollerspeed;
                //ROS_INFO("spin on");
            }
            break;

        case EActionType::_STOP_ALL:
            waitHere();
            cmd->roller_speed = 0;
            cmd->spinner = false;
            break;

        }

    }

    void HandyController::setRobotVel(double _vtan , double _vnorm , double _w )
    {
        cmd->vel_F = _vtan;     cmd->vel_N = _vnorm;        cmd->vel_w = _w *_RAD2DEG;
        double wheel1, wheel2, wheel3, wheel4;
        jacobian(_vtan, _vnorm, _w *_RAD2DEG, wheel1, wheel2, wheel3, wheel4);
        //ROS_INFO_STREAM(_w<<", "<<_w *_RAD2DEG<<", "<<wheel1<<", "<<wheel2<<", "<< wheel3<<", "<<wheel4);
        cmd->wheel1 = static_cast<float>(wheel1);   cmd->wheel2 = static_cast<float>(wheel2);   cmd->wheel3 = static_cast<float>(wheel3);   cmd->wheel4 = static_cast<float>(wheel4);
    }

    void HandyController::waitHere()
    {
        setRobotVel(0.0, 0.0, 0.0);
        cmd->kickSpeed = 0;
        cmd->kickspeedz = 0;
        cmd->chip = false;
        //cmd->roller_speed = 0;
    }

    bool HandyController::eventFilter(QObject* target, QEvent* event)
    {
        //if(target == this)
        {
            if (event->type() == QEvent::KeyPress)
            {
                QKeyEvent* e = static_cast<QKeyEvent*>(event);
                if(e->key() == Qt::Key_A)
                {
                    emit mUI.Left->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_W)
                {
                    emit mUI.Up->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_D)
                {
                    emit mUI.Right->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_S)
                {
                    emit mUI.Down->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_Q)
                {
                    emit mUI.Angle1->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_E)
                {
                    emit mUI.Angle2->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_K)
                {
                    emit mUI.Kick->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_C)
                {
                    emit mUI.Chip->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_R)
                {
                    emit mUI.Roller->pressed();
                    return true;
                }
                if(e->key() == Qt::Key_H)
                {
                    emit mUI.Halt->pressed();
                    return true;
                }
            }
            if (event->type() == QEvent::KeyRelease)
            {
                QKeyEvent* e = static_cast<QKeyEvent*>(event);

                if( e->isAutoRepeat() )
                    return false;


                if(e->key() == Qt::Key_K || e->key() == Qt::Key_C || e->key() == Qt::Key_R)
                {
                    takeAction(_STOP_OTHER);
                    //publisher.publish(cmd);
                    return true;
                }
                if(e->key() == Qt::Key_W || e->key() == Qt::Key_A || e->key() == Qt::Key_S
                        || e->key() == Qt::Key_D || e->key() == Qt::Key_Q || e->key() == Qt::Key_E)
                {
                    takeAction(_STOP_NAVIGATION);
                    //publisher.publish(cmd);
                    return true;
                }

            }
        }
        return false;
    }

    void HandyController::leftpressed()
    {
        //ROS_INFO("leftclicked");
        takeAction(EActionType::_LEFT);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::uppressed()
    {
        //ROS_INFO("upclicked");
        takeAction(EActionType::_FORWARD);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::rightpressed()
    {
        //ROS_INFO("rightclicked");
        takeAction(EActionType::_RIGHT);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::downpressed()
    {
        //ROS_INFO("downclicked");
        takeAction(EActionType::_BACKWARD);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::angle1pressed()
    {
        //ROS_INFO("angle1clicked");
        takeAction(EActionType::_TURN_CCW);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::angle2pressed()
    {
        //ROS_INFO("angle2clicked");
        takeAction(EActionType::_TURN_CW);
        //publisher.publish(cmd);
        //cleanRobotVel();
    }
    void HandyController::kickpressed()
    {
        //ROS_INFO("kickclicked");
        takeAction(EActionType::_KICK);
        //publisher.publish(cmd);
       // cleanRobotKick();
    }
    void HandyController::chippressed()
    {
        //ROS_INFO("chipclicked");
        takeAction(EActionType::_CHIP);
        //publisher.publish(cmd);
        //cleanRobotChip();
    }

    void HandyController::haltpressed()
    {
        //ROS_INFO("haltclicked");
        takeAction(EActionType::_STOP_ALL);
        //publisher.publish(cmd);
    }

    void HandyController::rollerpressed()
    {
        //ROS_INFO("Rollerclicked");
        takeAction(EActionType::_ROLLER);
    }

    void HandyController::releasedother()
    {
        takeAction(_STOP_OTHER);
        //publisher.publish(cmd);
    }

    void HandyController::releasednav()
    {
        takeAction(_STOP_NAVIGATION);
        //publisher.publish(cmd);
    }

    void HandyController::agentidchanged(QString id)
    {
        agentID = id.toInt();
        publisher  = nh.advertise<parsian_msgs::parsian_robot_command>("/agent_"+ id.toStdString() +"/command", 1000);
        cmd.reset(new parsian_msgs::parsian_robot_command);
        cmd->robot_id = agentID;
        cmd->kickSpeed = 0;//kickspeed;
        cmd->kickspeedz = 0;//chipkickspeed;
        cmd->roller_speed = 0;
        cmd->wheelsspeed = true;
        cmd->spinner = false;
        cmd->chip = false;
    }
    void HandyController::speedchanged(int speed_)
    {
        speed = speed_;
    }
    void HandyController::kickspeedchanged(int speed_)
    {
        kickspeed = speed_;     cmd->kickSpeed =kickspeed;
    }
    void HandyController::chipkickspeedchanged(int speed_)
    {
        chipkickspeed = speed_; cmd->kickspeedz = chipkickspeed;
    }
    void HandyController::rollerspeedchanged(int speed_)
    {
        rollerspeed = speed_;   cmd->roller_speed = rollerspeed;
    }


/*bool hasConfiguration() const
{
  return true;
}

void triggerConfiguration()
{
  // Usually used to open a dialog to offer the user a set of configuration
}*/

    void HandyController::jacobian(double vx, double vy, double w, double &v1, double &v2, double &v3, double &v4)
    {
        float robotRadius = 0.0795;
        float wheelRadius = 0.0275;

        // Calculate Motor Speeds
        double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};

        double dw1 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[0])) + (vy * cos(motorAlpha[0]))) );
        double dw2 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[1])) + (vy * cos(motorAlpha[1]))) );
        double dw3 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[2])) + (vy * cos(motorAlpha[2]))) );
        double dw4 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[3])) + (vy * cos(motorAlpha[3]))) );

        float motorMaxRadPerSec = 1000*2*M_PI/60.0f;

        dw1 = (dw1 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
        dw2 = (dw2 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
        dw3 = (dw3 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
        dw4 = (dw4 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;

        v1 = (char) (round(dw1));
        v2 = (char) (round(dw2));
        v3 = (char) (round(dw3));
        v4 = (char) (round(dw4));
    }
}  // namespace rqt_example_cpp
PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::HandyController, rqt_gui_cpp::Plugin)
