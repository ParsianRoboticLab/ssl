#include <rqt_parsian_gui/taskRunnerWidget.h>
#include <parsian_msgs/parsian_world_model.h>


namespace rqt_parsian_gui
{
    TaskRunnerWidget::TaskRunnerWidget(ros::NodeHandle & n) : QWidget() {

        gridLayout = new QGridLayout();


        //toolButton = new QToolButton();
        //toolButton->setText("GoToPointAvoid");

        //agentId = new QToolButton;
        //agentId->setText("0");

        comboBoxTask = new QComboBox;
        comboBoxTask->addItem("GotoPointAvoid");
        comboBoxTask->addItem("Kick");
        comboBoxTask->addItem("Receive");

        comboBoxPN = new QComboBox;
        comboBoxPN->addItem("0");
        comboBoxPN->addItem("1");
        comboBoxPN->addItem("2");
        comboBoxPN->addItem("3");
        comboBoxPN->addItem("4");
        comboBoxPN->addItem("5");
        comboBoxPN->addItem("6");
        comboBoxPN->addItem("7");
        comboBoxPN->addItem("8");
        comboBoxPN->addItem("9");
        comboBoxPN->addItem("10");
        comboBoxPN->addItem("11");







        tasks = new QAction* [_TASK_NUM];
        ids   = new QAction* [_PLAYER_NUMBER];

        //connect(toolButton,SIGNAL(triggered(QAction*)),this,SLOT(setTask(QAction*)));

        for (int i = 0; i < _TASK_NUM; ++i) {
            tasks[i] = new QAction(taskNames[i], this);
            connect(comboBoxTask, SIGNAL(triggered(QAction * )), this, SLOT(setTask(QAction * )));
            comboBoxTask->addAction(tasks[i]);
        }

        for(int i{};i <_PLAYER_NUMBER;i++){
            ids[i] = new QAction(QString::number(i),this);
            connect(comboBoxPN, SIGNAL(triggered(QAction * )), this, SLOT(setID(QAction * )));
            comboBoxPN->addAction(ids[i]);
        }




        gridLayout->addWidget(comboBoxTask);
        gridLayout->addWidget(comboBoxPN);
        this->setLayout(gridLayout);

        //robTaskPub = n.advertise<parsian_msgs::parsian_robot_task>("/agent_0/task",100);
        for (int i = 0; i < _PLAYER_NUMBER; ++i) {
            std::string topic(QString("/agent_%1/task").arg(i).toStdString());
            robTaskPub[i] = n.advertise<parsian_msgs::parsian_robot_task>(topic, 100);
        }
        worldModelSub = n.subscribe<parsian_msgs::parsian_world_model>("/world_model", 1000, boost::bind(& TaskRunnerWidget::m_wmCb, this, _1));
        mousePosSub = n.subscribe("/mousePos",10, &TaskRunnerWidget::mousePosCallBack,this);
        ROS_INFO("Satr");



}
    void TaskRunnerWidget::setTask(QAction* action) {
      /////////
      //////////
    }

    void TaskRunnerWidget::setID(QAction * action ){
        //agentId->setText(action->text());
       // agent_id = action->text().toInt();


//        task->select = 255;
    }


    void TaskRunnerWidget::m_wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm) {

        ROS_INFO_STREAM(comboBoxTask->currentText().toStdString());
        //ROS_INFO("wm_in");
        if(task != 0){
            robTaskPub[(comboBoxPN->currentText()).toInt()].publish(task);
        }

    }




    TaskRunnerWidget::~TaskRunnerWidget() {
    }

    void TaskRunnerWidget::mousePosCallBack(parsian_msgs::vector2DConstPtr pos) {
        ROS_INFO("MOSPOS");
        task.reset(new parsian_msgs::parsian_robot_task());
        if(QString::fromStdString(taskNames[0]) == comboBoxTask->currentText()) {
            ROS_INFO("GOTOPOINTAVOID");
            task->gotoPointAvoidTask.base.targetPos.x = pos->x;
            task->gotoPointAvoidTask.base.targetPos.y = pos->y;
            task->select = parsian_msgs::parsian_robot_task::GOTOPOINTAVOID;
            //robTaskPub.publish(task);
        }
        //else if (QString::fromStdString(taskNames[1]) == comboBoxTask->currentText()){
        else if(QString::fromStdString(taskNames[1]) == comboBoxTask->currentText()){
            ROS_INFO("KICK");
            task->kickTask.iskickchargetime = true;
            task->kickTask.kickchargetime = 500;
            task->kickTask.target.x = pos->x;
            task->kickTask.target.y = pos->y;
            task->kickTask.avoidPenaltyArea = true;
            task->select = parsian_msgs::parsian_robot_task::KICK;

        }
        else if(QString::fromStdString(taskNames[2]) == comboBoxTask->currentText()) {
            ROS_INFO("RECEIVE");
            task->receivePassTask.target.x = pos->x;
            task->receivePassTask.target.y = pos->y;
            task->receivePassTask.receiveRadius = 0.5;
            task->select = parsian_msgs::parsian_robot_task::RECIVEPASS;

        }

    }
}




