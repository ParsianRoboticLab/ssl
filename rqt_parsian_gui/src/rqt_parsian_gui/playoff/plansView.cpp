#include <rqt_parsian_gui/playoff/plansView.h>
namespace rqt_parsian_gui
{
    PlansView::PlansView(QScrollArea *parent) : QScrollArea(parent)
    {
        //layout
        contacts_list_layout = new QVBoxLayout(this);
        contacts_list_layout->setContentsMargins(0, 0, 0, 0);
        contacts_list_layout->setSpacing(0);
        contacts_list_layout->setMargin(0);
        filler = new QWidget(this);
        contacts_list_layout->addWidget(filler);

        //widget
        contact_list_widget = new QWidget(this);
        contact_list_widget->setObjectName("contactlist");
        contact_list_widget->setLayout(contacts_list_layout);
        contact_list_widget->setContentsMargins(0, 0, 0, 0);
        //contact_list_widget->setStyleSheet("QWidget#contactlist{background-color: white;}");

        //scroll bar
        this->setWidget(contact_list_widget);
        this->setAlignment(Qt::AlignCenter);
        this->setWidgetResizable(true);
        this->setMinimumWidth(300);
        this->setContentsMargins(0, 0, 0, 0);


    }

    void PlansView::add_contact(QString plan, bool isActive, bool isMaster, bool put_option)
    {
//        //check if the username already exists
//        for(int i{}; i < contacts_list.size(); i++)
//            if(contacts_list[i]->plan == plan)
//                return;
        PlanLabel* new_contact = new PlanLabel(put_option);
        new_contact->create_plan(plan, isActive, isMaster);
        new_contact->setServerUpdateService(*server_update);
        contacts_list_layout->addWidget(new_contact, 0, Qt::AlignTop);
        contacts_list.push_back(new_contact);
        contacts_list_layout->removeWidget(filler);
        if(this->height() - contacts_list.size()*50 > 0)
            filler->setFixedHeight(this->height() - contacts_list.size()*50);
        else
            filler->setFixedHeight(0);
        contacts_list_layout->addWidget(filler);
    }
    void PlansView::clear_all()
    {
        for(const auto& contact : contacts_list)
        {
            contacts_list_layout->removeWidget(contact);
            contact->hide();
        }
        contacts_list_layout->removeWidget(filler);
        contacts_list.clear();
        if(this->height() - contacts_list.size()*50 > 0)
            filler->setFixedHeight(this->height() - contacts_list.size()*50);
        else
            filler->setFixedHeight(0);
        contacts_list_layout->addWidget(filler);
    }


    void PlansView::sort()
    {
        for(const auto& contact : contacts_list)
            contacts_list_layout->removeWidget(contact);
        contacts_list_layout->removeWidget(filler);
        std::sort(contacts_list.begin(), contacts_list.end());
        //update GUI
        for(const auto& contact : contacts_list)
        {
            contacts_list_layout->addWidget(contact, 0, Qt::AlignTop);
            contact->setServerUpdateService(*server_update);
        }
        if(this->height() - contacts_list.size()*50 > 0)
            filler->setFixedHeight(this->height() - contacts_list.size()*50);
        else
            filler->setFixedHeight(0);
        contacts_list_layout->addWidget(filler);
    }


    void PlansView::resizeEvent(QResizeEvent *)
    {
        this->sort();
    }

    void PlansView::setServerUpdateService(ros::ServiceClient& _client) {
        server_update = &_client;
    }
}
