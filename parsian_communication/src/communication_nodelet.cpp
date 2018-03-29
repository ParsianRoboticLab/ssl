//
// Created by parsian-ai on 10/6/17.
//

#include <parsian_communication/communication_nodelet.h>
PLUGINLIB_EXPORT_CLASS(parsian_communication::CommunicationNodelet, nodelet::Nodelet)

using namespace parsian_communication;


void CommunicationNodelet::onInit() {


    ros::NodeHandle& n = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();

    timer = n.createTimer(ros::Duration(.016), boost::bind(&CommunicationNodelet::timerCb, this, _1));
    recTimer = n.createTimer(ros::Duration(.02), boost::bind(&CommunicationNodelet::recTimerCb, this, _1));

    drawer = new Drawer();
    debugger = new Debugger();

    drawPub    = n.advertise<parsian_msgs::parsian_draw>("/draws", 1000);
    debugPub   = n.advertise<parsian_msgs::parsian_debugs>("/debugs", 1000);
    statusPub  = n.advertise<parsian_msgs::parsian_robots_status>("/robots_status", 1000);
    robotPacketSub   = n.subscribe("/packets" , 10000, &CommunicationNodelet::callBack, this);
    team_config_sub = n.subscribe("/team_config", 1000, & CommunicationNodelet::teamConfigCb, this);


    communicator.reset(new CCommunicator);
    /////connect serial

    server.reset(new dynamic_reconfigure::Server<communication::communicationConfig>(private_nh));
    dynamic_reconfigure::Server<communication::communicationConfig>::CallbackType f;
    f = boost::bind(&CommunicationNodelet::ConfigServerCallBack, this, _1, _2);
    server->setCallback(f);


    while (!communicator->isSerialConnected()) {
        communicator->connectSerial(conf.serial_connect.c_str());//conf.serial_connect.c_str());
    }


//    ros::Rate loop_rate(62);
//
//    while (ros::ok()) {
//
//        drawPub.publish(drawer->draws);
//        debugPub.publish(debugger->debugs);
//
//        ros::spinOnce();
//        loop_rate.sleep();
//    }

}

void CommunicationNodelet::callBack(const parsian_msgs::parsian_packetsConstPtr& _packet) {
    ROS_INFO("salam");

    if (!communicator->isSerialConnected()) {
        communicator->connectSerial(conf.serial_connect.c_str());//conf.serial_connect.c_str());
    }
    if (cbCount >= 90) {
        cbCount = 0;
        sim_handle_flag = false;
    }

    if (realGame) {
        communicator->packetCallBack(_packet);
        sim_handle_flag = true;
        cbCount = 0;
    } else if (cbCount < 60 && sim_handle_flag) {
        communicator->packetCallBack(modeChangePacket(_packet));
        cbCount++;
    } else if (cbCount < 90 && sim_handle_flag) {
        communicator->packetCallBack(modeChangePacketZero(_packet));
        cbCount++;
    }
}

parsian_msgs::parsian_packetsPtr CommunicationNodelet::modeChangePacket(const parsian_msgs::parsian_packetsConstPtr& _packet) {
    auto  sim_handle_packet = *_packet;
    for (auto & robot_packet : sim_handle_packet.value) {
        for (int i = 0; i < 14; i++) {
            if (i == 1) {
                robot_packet.packets[i] &= 0x0F;
            } else if (i == 8) {
                robot_packet.packets[i] = 0x01;
            } else if (i != 11 && i != 0) {
                robot_packet.packets[i] = 0;
            }

        }
    }
    parsian_msgs::parsian_packetsPtr packet_{new parsian_msgs::parsian_packets};
    *packet_ = sim_handle_packet;
    return packet_;
}

parsian_msgs::parsian_packetsPtr CommunicationNodelet::modeChangePacketZero(const parsian_msgs::parsian_packetsConstPtr& _packet) {
    auto  sim_handle_packet = *_packet;
    for (auto & robot_packet : sim_handle_packet.value) {
        for (int i = 0; i < 14; i++) {
            if (i == 1) {
                robot_packet.packets[i] &= 0x0F;
            } else if (i == 8) {
                robot_packet.packets[i] = 0x00;
            } else if (i != 11 && i != 0) {
                robot_packet.packets[i] = 0;
            }

        }
    }
    parsian_msgs::parsian_packetsPtr packet_{new parsian_msgs::parsian_packets};
    *packet_ = sim_handle_packet;
    return packet_;
}

void CommunicationNodelet::timerCb(const ros::TimerEvent &event) {
    if (drawer != nullptr) {
        drawPub.publish(drawer->draws);
        drawer->draws.vectors.clear();
    }
    if (debugger != nullptr) {
        debugPub.publish(debugger->debugs);
    }
}


void CommunicationNodelet::recTimerCb(const ros::TimerEvent &event) {
    communicator->readData();
    if (communicator->robotsStat != nullptr) {
        statusPub.publish(communicator->robotsStat);
    }
}

void CommunicationNodelet::ConfigServerCallBack(const communication::communicationConfig &config, uint32_t level) {
    conf = config;
}

void CommunicationNodelet::teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg) {
    realGame = msg->mode == parsian_msgs::parsian_team_config::REAL;
}

//PLUGINLIB_DECLARE_CLASS(parsian_communication,CommunicationNodelet,parsian_communication::CommunicationNodelet,nodelet::Nodelet);
