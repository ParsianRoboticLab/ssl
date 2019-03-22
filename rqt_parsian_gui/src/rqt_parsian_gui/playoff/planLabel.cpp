#include <rqt_parsian_gui/playoff/planLabel.h>


namespace rqt_parsian_gui
{
    PlanLabel::PlanLabel(bool _put_options) : QWidget()
    {
        //getting style sheets
        std::string path = ros::package::getPath("rqt_parsian_gui");
        resourcePath = QString::fromStdString(path);
        resourcePath += "/resource/style_sheet/playoff_planLabel.qss";
        File.setFileName(resourcePath);
        ROS_INFO_STREAM("is qt PlanLabel_stylesheet opend:" <<File.open(QFile::ReadOnly));
        FormStyleSheet = QLatin1String(File.readAll());
        this->setStyleSheet(FormStyleSheet);
        File.close();

        //widget
        this->plan = "";
        this->put_options = _put_options;
        this->isActive = false;
        this->isMaster = false;
        this->main = new QHBoxLayout(this);
        this->main_widget = new QWidget(this);
        this->main_widget->setObjectName("main_widget");
        this->main_layout = new QGridLayout(this->main_widget);
        if(this->put_options)
        {
            this->activate = new QPushButton("A", this);
            this->activate->setObjectName("activate_button");
            this->activate->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            this->deactivate = new QPushButton("D", this);
            this->deactivate->setObjectName("deactivate_button");
            this->deactivate->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            this->master = new QPushButton("M", this);
            this->master->setObjectName("master_button");
            this->master->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        }
        this->planName = new QLabel("No Plan", this);
        this->planName->setObjectName("planName");

        //margin and padding
        this->setContentsMargins(0, 0, 0, 0);
        this->setMaximumHeight(50);
        this->main->setContentsMargins(0, 0, 0, 0);
        this->main->setMargin(0);
        this->main->setSpacing(0);
        this->main_widget->setContentsMargins(0, 0, 0, 0);
        this->main_layout->setContentsMargins(0, 0, 0, 0);
        this->main_layout->setMargin(0);
        this->main_layout->setSpacing(0);

    }

    void PlanLabel::create_plan(QString _plan, bool _isActive, bool _isMaster)
    {
        this->plan = "   " + _plan;
        this->isActive = _isActive;
        this->isMaster = _isMaster;
        this->planName->setText(this->plan);
        if(this->put_options)
        {
            this->main_layout->addWidget(this->planName, 0, 0, 1, 5);
            this->main_layout->addWidget(this->activate, 0, 5, 1, 1);
            this->main_layout->addWidget(this->deactivate, 0, 6, 1, 1);
            this->main_layout->addWidget(this->master, 0, 7, 1, 1);

            if(this->isActive)
            {
                this->activate->setEnabled(false);
                this->deactivate->setEnabled(true);
            }
            else
            {
                this->activate->setEnabled(true);
                this->deactivate->setEnabled(false);
            }

            if(this->isMaster)
            {
                this->master->setEnabled(false);
            }
            else
            {
                this->master->setEnabled(true);
            }
            this->activate->style()->unpolish(this->activate);
            this->activate->style()->polish(this->activate);
            this->deactivate->style()->unpolish(this->deactivate);
            this->deactivate->style()->polish(this->deactivate);
            this->master->style()->unpolish(this->master);
            this->master->style()->polish(this->master);
        }
        else
        {
            this->main_layout->addWidget(this->planName, 0, 0, 1, 8);
        }
        this->main_widget->setLayout(this->main_layout);
        this->main->addWidget(this->main_widget);
        this->setLayout(this->main);

    }


    PlanLabel::~PlanLabel()
    {
        delete this->planName;
        delete this->master;
        delete this->deactivate;
        delete this->activate;
        delete this->main_layout;
    }


}




