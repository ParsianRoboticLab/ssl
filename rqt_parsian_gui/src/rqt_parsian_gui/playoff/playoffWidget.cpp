#include <rqt_parsian_gui/playoff/playoffWidget.h>


namespace rqt_parsian_gui
{
    PlayOffWidget::PlayOffWidget() : QWidget()
    {
        this->create_main_widget();
    }

    void PlayOffWidget::create_main_widget()
    {
        this->main_layout = new QGridLayout(this);
        b1 = new PlanLabel(true);
        b1->create_plan("plan A", true, true);
        b2 = new PlanLabel(true);
        b2->create_plan("plan B", true, false);
        b2->setServerUpdateService(*server_update);
        b3 = new PlanLabel(true);
        b3->create_plan("plan C", false, true);
        b3->setServerUpdateService(*server_update);
        b4 = new PlanLabel(true);
        b4->create_plan("plan D", false, false);
        b4->setServerUpdateService(*server_update);
        b5 = new PlanLabel(false);
        b5->create_plan("plan E", true, true);
        b5->setServerUpdateService(*server_update);
        b6 = new PlanLabel(false);
        b6->create_plan("plan F", true, false);
        b6->setServerUpdateService(*server_update);
        b7 = new PlanLabel(false);
        b7->create_plan("plan G", false, true);
        b7->setServerUpdateService(*server_update);
        b8 = new PlanLabel(false);
        b8->create_plan("plan H", false, false);
        b8->setServerUpdateService(*server_update);
        b9 = new PlanLabel(true);
        b9->create_plan("plan I", true, true);
        b9->setServerUpdateService(*server_update);
        this->main_layout->addWidget(b1, 0, 0, 1, 4);
        this->main_layout->addWidget(b2, 0, 4, 1, 8);
        this->main_layout->addWidget(b3, 1, 0, 1, 4);
        this->main_layout->addWidget(b4, 1, 4, 1, 8);
        this->main_layout->addWidget(b5, 2, 0, 1, 4);
        this->main_layout->addWidget(b6, 2, 4, 1, 4);
        this->main_layout->addWidget(b7, 2, 8, 1, 4);
        this->main_layout->addWidget(b8, 3, 0, 7, 4);
        this->main_layout->addWidget(b9, 3, 4, 7, 8);
        this->setLayout(this->main_layout);


    }

    void PlayOffWidget::setServerUpdateService(ros::ServiceClient& _client) {
        server_update = &_client;
        b1->setServerUpdateService(*server_update);
        b2->setServerUpdateService(*server_update);
        b3->setServerUpdateService(*server_update);
        b4->setServerUpdateService(*server_update);
        b5->setServerUpdateService(*server_update);
        b6->setServerUpdateService(*server_update);
        b7->setServerUpdateService(*server_update);
        b8->setServerUpdateService(*server_update);
        b9->setServerUpdateService(*server_update);

    }


    PlayOffWidget::~PlayOffWidget() = default;


}




