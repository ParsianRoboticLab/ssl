#include "ros/ros.h"
#include "nodelet/loader.h"

int main(int argc, char **argv) {
    ros::init(argc, argv, "refbox");
    nodelet::Loader nodelet;
    const nodelet::M_string &remap(ros::names::getRemappings());
    nodelet::V_string nargv;
    const std::string &nodelet_name = ros::this_node::getName();
    nodelet.load(nodelet_name, "parsian_protobuf_wrapper/RefreeNodelet", remap, nargv);
    ros::spin();
    return 0;
}