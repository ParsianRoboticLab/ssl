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
        b1 = new QPushButton("A");
        b2 = new QPushButton("B");
        b3 = new QPushButton("C");
        b4 = new QPushButton("D");
        b5 = new QPushButton("E");
        b6 = new QPushButton("F");
        b7 = new QPushButton("G");
        b8 = new QPushButton("H");
        b9 = new QPushButton("I");
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


    PlayOffWidget::~PlayOffWidget() = default;


}




