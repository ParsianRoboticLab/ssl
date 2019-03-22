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

//        this->main_layout->addWidget(b1, 0, 0, 1, 4);
//        this->main_layout->addWidget(b2, 0, 4, 1, 8);
//        this->main_layout->addWidget(b3, 1, 0, 1, 4);
//        this->main_layout->addWidget(b4, 1, 4, 1, 8);
//        this->main_layout->addWidget(b5, 2, 0, 1, 4);
//        this->main_layout->addWidget(b6, 2, 4, 1, 4);
//        this->main_layout->addWidget(b7, 2, 8, 1, 4);
//        this->main_layout->addWidget(b8, 3, 0, 7, 4);
//        this->main_layout->addWidget(b9, 3, 4, 7, 8);
        this->setLayout(this->main_layout);


    }

    void PlayOffWidget::setServerUpdateService(ros::ServiceClient& _client) {
        server_update = &_client;
    }



    PlayOffWidget::~PlayOffWidget() = default;


}




