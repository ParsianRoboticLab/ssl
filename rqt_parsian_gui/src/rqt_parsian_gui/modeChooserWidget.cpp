#include <rqt_parsian_gui/modeChooserWidget.h>
#include <QFile>
#include <QTextStream>
#include <rospack/rospack.h>
#include <ros/package.h>
namespace rqt_parsian_gui {
ModeChooserWidget::ModeChooserWidget(ros::NodeHandle & n)
    : QWidget() {
    color = Color::YELLOW;
    side  = TeamSide::LEFT;
    mode  = GameMode::SIMULATION;

    team_config_pub = n.advertise<parsian_msgs::parsian_team_config>("/team_config", 1000);
    wmsub = n.subscribe("/world_model", 1000, &ModeChooserWidget::wmCb, this);

    loadTeamConfig();

    colorStr[0] = "YELLOW";
    colorStr[1] = "BLUE";

    sideStr[0] = "LEFT";
    sideStr[1] = "RIGHT";

    modeStr[0] = "SIMULATION";
    modeStr[1] = "REAL";

    this->setFixedSize(90, 110);

    mainLayout = new QGridLayout(this);

    colorPB = new QPushButton(colorStr[static_cast<int>(color)], this);
    modePB  = new QPushButton(modeStr[static_cast<int>(mode)], this);
    sidePB  = new QPushButton(sideStr[static_cast<int>(side)], this);

    colorPB->setFixedSize(70, 30);
    modePB->setFixedSize(70, 30);
    sidePB->setFixedSize(70, 30);

    modeAct = new QAction(this);
    modeAct->setShortcut(*new QKeySequence(tr("Ctrl+S")));

    mainLayout->addWidget(colorPB);
    mainLayout->addWidget(sidePB);
    mainLayout->addWidget(modePB);

    this->setLayout(mainLayout);
    this->addAction(modeAct);

    connect(colorPB, SIGNAL(clicked(bool))  , this, SLOT(toggleColor()));
    connect(sidePB , SIGNAL(clicked(bool))  , this, SLOT(toggleSide()));
    connect(modePB , SIGNAL(clicked(bool))  , this, SLOT(toggleMode()));
    connect(modeAct, SIGNAL(triggered(bool)), this, SLOT(toggleMode()));


}

ModeChooserWidget::~ModeChooserWidget() {
}

void ModeChooserWidget::toggleMode() {
    mode = (mode == GameMode::SIMULATION) ? GameMode::REAL : GameMode::SIMULATION;
    modePB->setText(modeStr[static_cast<int>(mode)]);
    //sendTeamConfig();
}

void ModeChooserWidget::toggleColor() {
    color = (color == Color::YELLOW) ? Color::BLUE : Color::YELLOW;
    colorPB->setText(colorStr[static_cast<int>(color)]);
    //sendTeamConfig();
}

void ModeChooserWidget::toggleSide() {
    side = (side == TeamSide::LEFT) ? TeamSide::RIGHT : TeamSide::LEFT;
    sidePB->setText(sideStr[static_cast<int>(side)]);
    //sendTeamConfig();
}


void ModeChooserWidget::sendTeamConfig() {

    parsian_msgs::parsian_team_configPtr team_config{new parsian_msgs::parsian_team_config};
    team_config->color = static_cast<unsigned char>(color);
    team_config->mode  = static_cast<unsigned char>(mode);
    team_config->side  = static_cast<unsigned char>(side);
    team_config_pub.publish(team_config);
}

void ModeChooserWidget::saveTeamConfig() {
    std::string s;
    s = ros::package::getPath("rqt_parsian_gui");
    ROS_INFO_STREAM(s + "aa");
    QFile myFile(QString::fromStdString(s) + FILE_PATH);
    if (!myFile.open(QFile::WriteOnly | QFile::Text)) {
        ROS_INFO_STREAM("saving team config failed" << s + FILE_PATH);
        return;
    }
    QTextStream out(&myFile);
    out << static_cast<int>(color) << static_cast<int>(side) <<  static_cast<int>(mode);
    out.flush();
    myFile.close();
}

void ModeChooserWidget::loadTeamConfig() {

    ROS_INFO_STREAM(ros::package::getPath("rqt_parsian_gui") + FILE_PATH);
    QFile myFile(QString::fromStdString(ros::package::getPath("rqt_parsian_gui") + FILE_PATH));
    if (!myFile.open(QFile::ReadOnly | QFile::Text)) {
        ROS_INFO("loading team config failed");
        return;
    }
    QByteArray temp = myFile.readAll();
    color = static_cast<Color>(temp.at(0) - '0');
    side  = static_cast<TeamSide>(temp.at(1) - '0');
    mode  = static_cast<GameMode>(temp.at(2) - '0');

    ROS_INFO_STREAM((int)color << (int)side << (int)mode);
    myFile.close();
}

void ModeChooserWidget::wmCb(const parsian_msgs::parsian_world_modelConstPtr& msg) {
    sendTeamConfig();
}

}


