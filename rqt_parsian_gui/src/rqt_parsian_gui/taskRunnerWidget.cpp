#include <rqt_parsian_gui/taskRunnerWidget.h>


namespace rqt_parsian_gui
{
    TaskRunnerWidget::TaskRunnerWidget(ros::NodeHandle & n) : QWidget() {
        task.reset(new parsian_msgs::parsian_robot_task());
        task->select = 255;
        client = new parsian_msgs::grsim_ball_replacement();
        this->setFixedSize(300,200);
        gridLayout = new QGridLayout();

        toolButton = new QToolButton();
        toolButton->setText("Choose Task");

        agentId = new QToolButton;
        agentId->setText("0");
        agent_id =0;

        tasks = new QAction* [TASK_NUM];
        ids   = new QAction* [_MAX_NUM_PLAYERS];

        for (int i = 0; i < TASK_NUM; ++i) {
            tasks[i] = new QAction(taskNames[i], this);
            connect(toolButton, SIGNAL(triggered(QAction * )), this, SLOT(setTask(QAction * )));
            toolButton->addAction(tasks[i]);
        }
            //##################################################
        for (int i = 0; i < _MAX_NUM_PLAYERS; ++i) {
            ids[i] = new QAction(QString::number(i),this);
            connect(agentId, SIGNAL(triggered(QAction * )), this, SLOT(setID(QAction * )));
            agentId->addAction(ids[i]);

        }

        gridLayout->addWidget(toolButton);
        gridLayout->addWidget(agentId);
        this->setLayout(gridLayout);

        mousePosSub = n.subscribe("/mousePos",10, &TaskRunnerWidget::mousePosCallBack, this);
        ballReplacementClient = n.serviceClient<parsian_msgs::grsim_ball_replacement>("/GrsimBallReplacesrv",true);
        robTaskPub = new ros::Publisher[_MAX_NUM_PLAYERS];
        for (int i = 0; i < _MAX_NUM_PLAYERS; ++i) {
            std::string topic(QString("/agent_%1/task").arg(i).toStdString());
            robTaskPub[i] = n.advertise<parsian_msgs::parsian_robot_task>(topic, 100);
        }
        timer = n.createTimer(ros::Duration(0.016), &TaskRunnerWidget::timerCb, this);
    }

    void TaskRunnerWidget::setTask(QAction* action) {
        task->select = 255;
        toolButton->setText(action->text());
    }

    void TaskRunnerWidget::setID(QAction * action ){
        task->select = 255;
        agentId->setText(action->text());
        agent_id = action->text().toInt();
    }

    TaskRunnerWidget::~TaskRunnerWidget() {
    }

    void TaskRunnerWidget::mousePosCallBack(parsian_msgs::vector2DConstPtr pos) {
        if (QString::fromStdString(taskNames[0]) == toolButton->text()){  // BALL REPLACEMENT REQUEST
            client->request.vx = 0;
            client->request.vy = 0;
            client->request.x = pos->x;
            client->request.y = pos->y;
            if(ballReplacementClient.call(*client)){
                ROS_INFO_STREAM("YES");
            } else ROS_INFO_STREAM("CALL FAILED");


        } else if(QString::fromStdString(taskNames[1]) == toolButton->text()){
            task->gotoPointAvoidTask = *new parsian_msgs::parsian_skill_gotoPointAvoid();
            task->gotoPointAvoidTask.base.targetPos.x=pos->x;
            task->gotoPointAvoidTask.base.targetPos.y=pos->y;
            task->select = parsian_msgs::parsian_robot_task::GOTOPOINTAVOID;
        }
        else if(QString::fromStdString(taskNames[2]) == toolButton->text()){
            task->gotoPointAvoidTask = *new parsian_msgs::parsian_skill_gotoPointAvoid();
            task->gotoPointAvoidTask.base.targetPos.x=pos->x;
            task->gotoPointAvoidTask.base.targetPos.y=pos->y;
            task->gotoPointAvoidTask.noAvoid = static_cast<unsigned char>(true);
            task->select = parsian_msgs::parsian_robot_task::GOTOPOINTAVOID;
        }else{
        }
    }

    void TaskRunnerWidget::timerCb(const ros::TimerEvent& _timer){
        if(task->select != 255)
            robTaskPub[agent_id].publish(task);
    }
}




