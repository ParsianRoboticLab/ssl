#include <rqt_parsian_gui/refereeWidget.h>

using namespace parsian_msgs;

namespace rqt_parsian_gui
{
    RefereeWidget::RefereeWidget(ros::NodeHandle & n) : QWidget() {
        refereePub = n.advertise<parsian_msgs::ssl_force_referee>("/force_referee", 100);
        mainLayout = new QHBoxLayout;

        posX = new QLineEdit("X",this);
        posY = new QLineEdit("Y",this);

        id = -1;
        auto *btnRefGroup = new QButtonGroup();
        auto *enableCHB = new QCheckBox("enable");
        strRefNames << "H" << "FS" << "S" << "NS" << "FK" << "IK" << "KO" << "PK" << "BP" << "FK" << "IK" << "KO" << "PK" << "BP";
        strRefCommands << "H" << "s" << "S" << " " << "F" << "I" << "K" << "P" << "B" << "f" << "i" << "k" << "p" << "b";
        for(int i=0 ; i<14 ; i++ )
        {
            btnRefs[i] = new QPushButton(strRefNames[i],this);
            QString strType = "n";
            if( i >=4 )
                strType = "b";
            if( i >=9 )
                strType = "y";
            btnRefs[i]->setProperty("refType" , QVariant::fromValue(strType));
            btnRefGroup->addButton(btnRefs[i] , i);
        }

        mainLayout->addWidget(enableCHB);
        for (auto &btnRef : btnRefs) {
            mainLayout->addWidget(btnRef);
        }
        mainLayout->addWidget(posX);
        mainLayout->addWidget(posY);
        double widgetWidth = 300;
        QString strWidth = QString("%1px").arg((int) (widgetWidth / 12));
        //StyleSheet
        QString styleSheet =
                QString("\
                    QPushButton[refType=\"n\"] {background-color:gray; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %1} \
            QPushButton[refType=\"b\"] {background-color:blue; color:white; padding: 0; border: 1px solid black; border-radius: 5px; width: %2} \
            QPushButton[refType=\"y\"] {background-color:yellow; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %3} \
            QPushButton[refType=\"n\"]:hover {border: 2px solid red} \
            QPushButton[refType=\"b\"]:hover {border: 2px solid red} \
            QPushButton[refType=\"y\"]:hover {border: 2px solid red} \
            ").arg(strWidth).arg(strWidth).arg(strWidth);

        this->setLayout(mainLayout);
        this->setStyleSheet(styleSheet);
        connect(btnRefGroup , SIGNAL(buttonClicked(int)) , this , SLOT(SetManualGS(int)));
        connect(enableCHB , SIGNAL(stateChanged(int)) , this , SLOT(SetEnable(int)));
        teamConfSub = n.subscribe("/team_config", 3, &RefereeWidget::teamConfCb, this);
        timer = n.createTimer(ros::Duration(0.1), &RefereeWidget::timerCb, this);

    }


    void RefereeWidget::SetManualGS( int id ){
        this->id = id;
    }

    void RefereeWidget::SetEnable(int enable){
        this->enable = enable;
    }

    void RefereeWidget::timerCb(const ros::TimerEvent& _timer){
        if(enable == 2){
            refree_command.reset(new parsian_msgs::ssl_force_referee());
            switch(id){
                case 0:
                    refree_command->command.command =ssl_refree_command::HALT ;
                    break;
                case 1:
                    refree_command->command.command = ssl_refree_command::FORCE_START;
                    break;
                case 2:
                    refree_command->command.command = ssl_refree_command::STOP;
                    break;
                case 3:
                    refree_command->command.command = ssl_refree_command::NORMAL_START;
                    break;
                case 4:
                    if (teamConfig.color == parsian_team_config::BLUE) {
                        refree_command->command.command = ssl_refree_command::DIRECT_FREE_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::DIRECT_FREE_THEM;
                    }
                    break;
                case 5:
                    if (teamConfig.color == parsian_team_config::BLUE) {
                        refree_command->command.command = ssl_refree_command::INDIRECT_FREE_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::INDIRECT_FREE_THEM;
                    }
                    break;
                case 6:
                    if (teamConfig.color == parsian_team_config::BLUE) {
                        refree_command->command.command = ssl_refree_command::PREPARE_KICKOFF_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::PREPARE_KICKOFF_THEM;
                    }
                    break;
                case 7:
                    if (teamConfig.color == parsian_team_config::BLUE) {
                        refree_command->command.command = ssl_refree_command::PREPARE_PENALTY_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::PREPARE_PENALTY_THEM;
                    }
                    break;
                case 8:
                    if (teamConfig.color == parsian_team_config::BLUE) {
                        refree_command->command.command = ssl_refree_command::BALL_PLACEMENT_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::BALL_PLACEMENT_THEM;
                    }
                    break;
                case 9:
                    if (teamConfig.color == parsian_team_config::YELLOW) {
                        refree_command->command.command = ssl_refree_command::DIRECT_FREE_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::DIRECT_FREE_THEM;
                    }
                    break;
                case 10:
                    if (teamConfig.color == parsian_team_config::YELLOW) {
                        refree_command->command.command = ssl_refree_command::INDIRECT_FREE_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::INDIRECT_FREE_THEM;
                    }
                    break;
                case 11:
                    if (teamConfig.color == parsian_team_config::YELLOW) {
                        refree_command->command.command = ssl_refree_command::PREPARE_KICKOFF_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::PREPARE_KICKOFF_THEM;
                    }
                    break;
                case 12:
                    if (teamConfig.color == parsian_team_config::YELLOW) {
                        refree_command->command.command = ssl_refree_command::PREPARE_PENALTY_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::PREPARE_PENALTY_THEM;
                    }
                    break;
                case 13:
                    if (teamConfig.color == parsian_team_config::YELLOW) {
                        refree_command->command.command = ssl_refree_command::BALL_PLACEMENT_US;
                    } else {
                        refree_command->command.command = ssl_refree_command::BALL_PLACEMENT_THEM;
                    }
                    break;
                default:
                    return;
            }
            refree_command->ballPlacementPos.x = posX->text().toDouble();
            refree_command->ballPlacementPos.y = posY->text().toDouble();
            refereePub.publish(refree_command);
        }
    }

    void RefereeWidget::teamConfCb(const parsian_msgs::parsian_team_configConstPtr& _conf) {
        teamConfig = *_conf;
    }

    RefereeWidget::~RefereeWidget() = default;


}




