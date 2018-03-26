//
// Created by parsian-ai on 12/31/17.
//

#include "rqt_parsian_gui/playoffWidget.h"

using namespace rqt_parsian_gui;

PlayOffWidget::PlayOffWidget(ros::NodeHandle & n) : QWidget() {

    client = n.serviceClient<parsian_msgs::parsian_update_plans> ("/update_plans", true);
    theplans = new parsian_msgs::parsian_update_plans();
    lastPlan = new parsian_msgs::parsian_plan_GUI();
    chosen = new parsian_msgs::parsian_plan_GUI();

    mode = new QPushButton("Game Mode");
    update = new QPushButton("Update (Don't Worry! it will work fine :)");
    active = new QPushButton("Active");
    deactive = new QPushButton("Deactive");
    master = new QPushButton("Master");

    columns = new QColumnView();

    active->setEnabled(false);
    update->setEnabled(true);
    deactive->setEnabled(false);
    master->setEnabled(false);

    selection = columns->selectionModel();

    model = new QStandardItemModel();
    selection = new QItemSelectionModel(model);

    updateModel();

    columns->setModel(model);
    columns->setFont(QFont("Monospace"));
    columns->setSelectionModel(selection);

    QList<int> widthList;
    widthList.append(300);
    widthList.append(200);
    widthList.append(100);
    columns->setColumnWidths(widthList);

    //*Details*//


    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);


    auto buttons = new QHBoxLayout;
    auto *main = new QVBoxLayout;
    auto *detail = new QVBoxLayout;

    buttons->addWidget(deactive);
    buttons->addWidget(active);
    buttons->addWidget(master);

    for (auto &i : details) {
        i = new QLabel(this);
        i->setFont(QFont("Monospace"));
        detail->addWidget(i);
    }


    main->addWidget(mode);
    main->addWidget(update);
    main->addWidget(columns);
    main->addLayout(buttons);
    main->addWidget(line); //  <-- Just A Line
    main->addLayout(detail);

    connect(update, SIGNAL(clicked()), this, SLOT(slt_updatePlans()));
    connect(mode, SIGNAL(clicked()), this, SLOT(slt_changeMode()));
    connect(active, SIGNAL(clicked()), this, SLOT(slt_active()));
    connect(master, SIGNAL(clicked()), this, SLOT(slt_master()));
    connect(deactive, SIGNAL(clicked()), this, SLOT(slt_deactive()));
    connect(model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(slt_edit(QStandardItem *)));
    connect(selection, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(slt_selectionChanged(QItemSelection, QItemSelection)));
//    connect(m_loader, Q_SIGNAL(plansUpdated()), this, Q_SLOT(updateModel()));

    setLayout(main);
}

void PlayOffWidget::updateModel() {

    model->clear();

    QStandardItem *pkg;
    QStandardItem *file;
    QStandardItem *plan;

    int pkgCounter  = 0;
    int fileCounter = 0;
    int planCounter = 0;
    QStringList last_path_dir;
    QString last_pkg;
    QStringList response_path_dir;
    QString response_pkg ;
    int index[theplans->response.allPlans.size()][3] ; // to use in f

    for (size_t i = 0; i < theplans->response.allPlans.size(); i++) {



        index[i][0] = pkgCounter;
        index[i][1] = fileCounter;
        index[i][2] = planCounter;

        if (i != 0) {
            last_path_dir = response_path_dir;
            last_pkg = response_pkg;
        } else {
            last_pkg = "";
        }

        response_path_dir = QString::fromStdString(theplans->response.allPlans[i].planFile).split("plans/")[1].
                            split("/");

        response_pkg = "main";
        for (int path_ctr = 0; path_ctr < response_path_dir.size() - 1; path_ctr++) {
            response_pkg += "." + response_path_dir[path_ctr];
        }

        if (response_pkg.compare(last_pkg)) {
            pkgCounter++;
            fileCounter++;
            // debugger->debug("PKG",D_ALI);
            pkg = new QStandardItem(response_pkg);
            model->appendRow(pkg);
            //debugger->debug( "FILE",D_ALI);
            file = new QStandardItem(response_path_dir.last().left(response_path_dir.last().size() - 5));
            QString temp = response_pkg;
            file->setToolTip("<html><img src=" + temp.replace(".", "/") + "/" + response_pkg + ".png" + "/></html>");
            pkg->appendRow(file);
        } else if (response_path_dir.last().compare(last_path_dir.last())) {
            fileCounter++;
//            debugger->debug("PLAN",D_ALI);
            file = new QStandardItem(response_path_dir.last().left(response_path_dir.last().size() - 5));
            QString temp = response_pkg;
            file->setToolTip("<html><img src=" + temp.replace(".", "/") + "/" + response_pkg + ".png" + "/></html>");
            pkg->appendRow(file);
        }

        planCounter++;
        plan = new QStandardItem(QString("%1").arg(i));
        file->appendRow(plan);
        file->setEditable(false);
        plan->setEditable(false);
        pkg->setEditable(false);
        lastPlan = &theplans->response.allPlans.at(i);

    }
}

void PlayOffWidget::slt_changeMode() {
    debugMode = !debugMode;
    mode->setText((debugMode) ? "Game Mode" : "Debug Mode");
    updateBtn(debugMode);
//    qDebug() << "Mode Chaged to " << ((debugMode) ? "Debug" : "Game") << " Mode";
}

void PlayOffWidget::updateBtn(bool _debug) {
    if (_debug) {
        update->setEnabled(true);
        columns->setEnabled(true);
        active->setEnabled(true);
        master->setEnabled(true);
        deactive->setEnabled(true);
    } else {
        active->setEnabled(false);
        update->setEnabled(false);
        deactive->setEnabled(false);
        master->setEnabled(false);
    }
}

void PlayOffWidget::slt_updatePlans() {

    if (client.call(*theplans)) {
        ROS_INFO("req to plan server......");
    } else {
        ROS_INFO("ERROR req to plan server");
        return;
    }
    theplans->request.newPlans.clear();
    theplans->request.index.clear();
    updateModel();
}

void PlayOffWidget::slt_active() {

    QModelIndexList modelList = itemSelected.indexes();
    Q_FOREACH (QModelIndex model, modelList) {
        if (model.parent().row() == -1) {
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int j = -1;
                while (model.child(i, 0).child(++j, 0).data().toString() != "") {
                    int planID = model.child(i, 0).child(j, 0).data().toUInt();
                    theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                    theplans->request.index.push_back(j);
                }
            }
        } else if (model.parent().parent().row() == -1) {
            details[0]->setText(QString("Type : File"));
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int planID = model.child(i, 0).data().toUInt();
                theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                theplans->request.index.push_back(i);
            }
        } else if (model.parent().parent().parent().row() == -1) {
            unsigned int planID = model.data().toUInt();
            theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
            theplans->request.index.push_back(static_cast < unsigned int && >(model.row()));
        }
        theplans->request.isActive = static_cast<unsigned char>(true);
        theplans->request.isMaster = static_cast<unsigned char>(false);
        active->setEnabled(false);
        deactive->setEnabled(true);
        master->setEnabled(true);
    }

    slt_updatePlans();
}

void PlayOffWidget::slt_deactive() {
    QModelIndexList modelList = itemSelected.indexes();
    Q_FOREACH (QModelIndex model, modelList) {
        if (model.parent().row() == -1) {
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int j = -1;
                while (model.child(i, 0).child(++j, 0).data().toString() != "") {
                    int planID = model.child(i, 0).child(j, 0).data().toUInt();
                    theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                    theplans->request.index.push_back(j);
                }
            }
        } else if (model.parent().parent().row() == -1) {
            details[0]->setText(QString("Type : File"));
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int planID = model.child(i, 0).data().toUInt();
                theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                theplans->request.index.push_back(i);
            }
        } else if (model.parent().parent().parent().row() == -1) {
            unsigned int planID = model.data().toUInt();
            theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
            theplans->request.index.push_back(static_cast < unsigned int && >(model.row()));
        }
        theplans->request.isActive = static_cast<unsigned char>(false);
        theplans->request.isMaster = static_cast<unsigned char>(false);

        active->setEnabled(true);
        deactive->setEnabled(false);
        master->setEnabled(true);

    }

    slt_updatePlans();
}

void PlayOffWidget::slt_master() {
    QModelIndexList modelList = itemSelected.indexes();
    Q_FOREACH (QModelIndex model, modelList) {
        if (model.parent().row() == -1) {
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int j = -1;
                while (model.child(i, 0).child(++j, 0).data().toString() != "") {
                    int planID = model.child(i, 0).child(j, 0).data().toUInt();
                    theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                    theplans->request.index.push_back(j);
                }
            }
        } else if (model.parent().parent().row() == -1) {
            details[0]->setText(QString("Type : File"));
            int i = -1;
            while (model.child(++i, 0).data().toString() != "") {
                int planID = model.child(i, 0).data().toUInt();
                theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
                theplans->request.index.push_back(i);
            }
        } else if (model.parent().parent().parent().row() == -1) {
            unsigned int planID = model.data().toUInt();
            theplans->request.newPlans.push_back(theplans->response.allPlans[planID].planFile);
            theplans->request.index.push_back(static_cast < unsigned int && >(model.row()));
        }

        theplans->request.isActive = static_cast<unsigned char>(true);
        theplans->request.isMaster = static_cast<unsigned char>(true);

        active->setEnabled(false);
        deactive->setEnabled(true);
        master->setEnabled(false);

    }
    slt_updatePlans();
}

void PlayOffWidget::slt_edit(QStandardItem *_item) {
    // TODO : Make it possible to edit plans via gui.
}

void PlayOffWidget::slt_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

    for (int i = 0; i < 8; i++) {
        details[i]->setText("");
    }

//    chosen = NULL;
    itemSelected = selected;
    QModelIndexList modelList = selected.indexes();
    Q_FOREACH (QModelIndex model, modelList) {
        if (model.parent().row() == -1) {
            details[0]->setText(QString("Type  : Package"));
            active->setEnabled(true);
            deactive->setEnabled(true);
            master->setEnabled(true);
        } else if (model.parent().parent().row() == -1) {
            details[0]->setText(QString("Type : File"));
            active->setEnabled(true);
            deactive->setEnabled(true);
            master->setEnabled(true);

        } else if (model.parent().parent().parent().row() == -1) {

            unsigned int planID = model.data().toUInt();
            chosen = &theplans->response.allPlans.at(planID);

            details[0]->setText(QString("Type       : Plan"));
            details[1]->setText(QString("Agent Size : %1").arg(chosen->agentSize));
            details[2]->setText(QString("Plan Mode  : %1").arg(chosen->planMode.c_str()));
            details[3]->setText(QString("Chance     : %1").arg(chosen->chance));
            details[4]->setText(QString("Last Dist  : %1").arg(chosen->lastDist));
            auto final_tag = new QString();
            for (std::string str : chosen->tags) {
                *final_tag += QString::fromStdString(str) + "  ";
            }
            details[5]->setText(QString("Tags       : %1").arg(*final_tag));
//                details[6]->setText(QString("ShotPos    : (%1, %2)").arg(chosen->matching.shotPos.x).arg(
//                        chosen->matching.shotPos.y));
            //details[7]->setText(QString("ShotZone   : %1").arg(CCoach::getShotSpot(chosen->matching.initPos.ball, chosen->matching.shotPos)));

            active->setEnabled(!chosen->isActive);
            deactive->setEnabled(chosen->isActive);
            master->setEnabled(!chosen->isMaster);
//
        } else {
            details[0]->setText(QString("Type : SubPlan !!"));
        }
    }

}
