#include <rqt_parsian_gui/playoff/tabWidget.h>
namespace rqt_parsian_gui
{
    TabWidget::TabWidget(QScrollArea *parent) : QScrollArea(parent)
    {

        //getting style sheets
        std::string path = ros::package::getPath("rqt_parsian_gui");
        resourcePath = QString::fromStdString(path);
        resourcePath += "/resource/style_sheet/playoff_planLabel.qss";
        File.setFileName(resourcePath);
        File.open(QFile::ReadOnly);
        FormStyleSheet = QLatin1String(File.readAll());
        this->setStyleSheet(FormStyleSheet);
        File.close();
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

    void TabWidget::add_contact(QString name)
    {
        //check if the username already exists
        for(int i{}; i < contacts_list.size(); i++)
            if(contacts_list[i]->text() == name)
                return;
        QPushButton* new_contact = new QPushButton(name);
        new_contact->setFixedHeight(100);
        new_contact->setObjectName("tab");
        contacts_list_layout->addWidget(new_contact, 0, Qt::AlignTop);
        contacts_list.push_back(new_contact);
        contacts_list_layout->removeWidget(filler);
        if(this->height() - contacts_list.size()*100 > 0)
            filler->setFixedHeight(this->height() - contacts_list.size()*100);
        else
            filler->setFixedHeight(0);
        contacts_list_layout->addWidget(filler);
    }

    QPushButton* TabWidget::get_contact(QString name)
    {
        for(int i{}; i < contacts_list.size(); i++)
            if(contacts_list[i]->text() == name)
                return contacts_list[i];
        //nothing found(if this line reached means something in code is wrong)
        return nullptr;
    }


    void TabWidget::sort()
    {
        for(const auto& contact : contacts_list)
            contacts_list_layout->removeWidget(contact);
        contacts_list_layout->removeWidget(filler);
        //std::sort(contacts_list.begin(), contacts_list.end());
        //update GUI
        for(const auto& contact : contacts_list)
        {
            contacts_list_layout->addWidget(contact, 0, Qt::AlignTop);
        }
        if(this->height() - contacts_list.size()*100 > 0)
            filler->setFixedHeight(this->height() - contacts_list.size()*100);
        else
            filler->setFixedHeight(0);
        contacts_list_layout->addWidget(filler);
    }


    void TabWidget::resizeEvent(QResizeEvent *)
    {
        this->sort();
    }

}
