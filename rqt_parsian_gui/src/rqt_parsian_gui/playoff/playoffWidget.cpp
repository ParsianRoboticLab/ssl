#include <rqt_parsian_gui/playoff/playoffWidget.h>


namespace rqt_parsian_gui
{
    PlayOffWidget::PlayOffWidget() : QWidget()
    {
        this->main_layout = new QGridLayout(this);
        prev_view = new QWidget();
        all_plansView = new PlansView();
        active_plansView = new PlansView();
        ignored_plansView = new PlansView();
        plansView_widg = new QWidget();
        plansView_lay = new QVBoxLayout();
        tabWidget = new TabWidget();
        last_plan_title = new QLabel();
        last_plan = new PlanLabel(false);
        master_plan_title = new QLabel();
        master_plan = new PlanLabel(false);
        activate_all = new QPushButton("Activate All");
        deactivate_all = new QPushButton("DeActivate All");
        demaster = new QPushButton("DeMaster");
    }

    void PlayOffWidget::create_main_widget()
    {
        plansView_widg->setContentsMargins(0, 0, 0, 0);
        plansView_lay->setContentsMargins(0, 0, 0, 0);
        plansView_lay->setMargin(0);
        plansView_lay->setSpacing(0);
        plansView_widg->setLayout(plansView_lay);

        tabWidget->add_contact("All");
        tabWidget->add_contact("Active");
        tabWidget->add_contact("Ignored");
        connect(tabWidget->get_contact("All"), SIGNAL(pressed()), this, SLOT(all_pressed()));
        connect(tabWidget->get_contact("Active"), SIGNAL(pressed()), this, SLOT(active_pressed()));
        connect(tabWidget->get_contact("Ignored"), SIGNAL(pressed()), this, SLOT(ignored_pressed()));

        last_plan_title->setText("last ai plan");
        last_plan_title->setObjectName("plan_title");
        last_plan->create_plan("None", false, false);

        master_plan_title->setText("master plan");
        master_plan_title->setObjectName("plan_title");
        master_plan->create_plan("None", false, false);

        activate_all->setObjectName("mainbutton");
        deactivate_all->setObjectName("mainbutton");
        demaster->setObjectName("mainbutton");
        connect(activate_all, SIGNAL(pressed()), this, SLOT(activate_all_pressed()));
        connect(deactivate_all, SIGNAL(pressed()), this, SLOT(deactivate_all_pressed()));
        connect(demaster, SIGNAL(pressed()), this, SLOT(demaster_pressed()));


        this->main_layout->addWidget(last_plan_title, 0, 0, 1, 4);
        this->main_layout->addWidget(last_plan, 0, 4, 1, 8);
        this->main_layout->addWidget(master_plan_title, 1, 0, 1, 4);
        this->main_layout->addWidget(master_plan, 1, 4, 1, 8);
        this->main_layout->addWidget(activate_all, 2, 0, 1, 4);
        this->main_layout->addWidget(deactivate_all, 2, 4, 1, 4);
        this->main_layout->addWidget(demaster, 2, 8, 1, 4);
        this->main_layout->addWidget(tabWidget, 3, 0, 7, 4);
        this->main_layout->addWidget(plansView_widg, 3, 4, 7, 8);
        this->setLayout(this->main_layout);
        for(int i{}; i< 27 ; i++)
            all_plansView->add_contact("plan ", true , true, true);

        prev_view->hide();
        this->main_layout->addWidget(plansView_widg, 3, 4, 7, 8);
        prev_view = plansView_widg;
        prev_view->show();


    }


    void PlayOffWidget::setServerUpdateService(ros::ServiceClient& _client) {
        server_update = &_client;
        all_plansView->setServerUpdateService(*server_update);
        active_plansView->setServerUpdateService(*server_update);
        ignored_plansView->setServerUpdateService(*server_update);

    }

    void PlayOffWidget::all_pressed()
    {
        prev_view->hide();
        this->main_layout->addWidget(all_plansView, 3, 4, 7, 8);
        prev_view = all_plansView;
        prev_view->show();
    }

    void PlayOffWidget::active_pressed()
    {
        prev_view->hide();
        this->main_layout->addWidget(active_plansView, 3, 4, 7, 8);
        prev_view = active_plansView;
        prev_view->show();
    }

    void PlayOffWidget::ignored_pressed()
    {
        prev_view->hide();
        this->main_layout->addWidget(ignored_plansView, 3, 4, 7, 8);
        prev_view = ignored_plansView;
        prev_view->show();
    }

    void PlayOffWidget::activate_all_pressed()
    {
        if(this->server_update == nullptr)
            return;
        parsian_msgs::parsian_update_plansRequest req;
        parsian_msgs::parsian_update_plansResponse rep;
        req.Mode = 5;//ACTIVATE_ALL
        server_update->call(req, rep);
    }

    void PlayOffWidget::deactivate_all_pressed()
    {
        if(this->server_update == nullptr)
            return;
        parsian_msgs::parsian_update_plansRequest req;
        parsian_msgs::parsian_update_plansResponse rep;
        req.Mode = 6;//DEACTIVATE_ALL
        server_update->call(req, rep);
    }

    void PlayOffWidget::demaster_pressed()
    {
        if(this->server_update == nullptr)
            return;
        parsian_msgs::parsian_update_plansRequest req;
        parsian_msgs::parsian_update_plansResponse rep;
        req.Mode = 4;//DEMASTER
        server_update->call(req, rep);
    }

    void PlayOffWidget::subscribe(const parsian_msgs::parsian_playoff_clientConstPtr& _msg)
    {
        return;
        all_plansView = new PlansView();
        active_plansView = new PlansView();
        ignored_plansView = new PlansView();

        last_plan->create_plan(QString::fromStdString(_msg->last_ai_response), false, false);
        master_plan->create_plan(QString::fromStdString(_msg->master_plan), false, false);

        for(const auto& plan : _msg->active_plans)
        {
            if(plan == _msg->master_plan)
                active_plansView->add_contact(QString::fromStdString(plan), true, true, true);
            else
                active_plansView->add_contact(QString::fromStdString(plan), true, false, true);
        }

        for(const auto& plan : _msg->desired_plans)
        {
            if(std::find(_msg->active_plans.begin(), _msg->active_plans.end(), plan) != _msg->active_plans.end() && plan == _msg->master_plan) {
                all_plansView->add_contact(QString::fromStdString(plan), true, true, true);
            } else if(std::find(_msg->active_plans.begin(), _msg->active_plans.end(), plan) != _msg->active_plans.end()) {
                all_plansView->add_contact(QString::fromStdString(plan), true, false, true);
            }else if(plan == _msg->master_plan){
                all_plansView->add_contact(QString::fromStdString(plan), true, true, true);
            }else{
                all_plansView->add_contact(QString::fromStdString(plan), true, false, true);
            }
        }

        for(const auto& plan : _msg->ignored_plans)
        {
            ignored_plansView->add_contact(QString::fromStdString(plan), false, false, false);
        }

    }




    PlayOffWidget::~PlayOffWidget() = default;


}




