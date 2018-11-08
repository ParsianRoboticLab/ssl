#include <ros/ros.h>
#include <gtest/gtest.h>
#include "parsian_protobuf_wrapper/ssl-vision/convert/convert_units.h"

TEST(VisionTest, basicTest) {
    EXPECT_TRUE(true);
}

TEST(VisionTest, secondTest) {
    EXPECT_EQ(pr::mm_to_m(1000), 1.0);
}

int main(int argc, char** argv) {
    //// Initialize ROS node ////
    ros::init(argc, argv, "VisionTest");

    ros::NodeHandle nh("~");

    testing::InitGoogleTest(&argc, argv);
    const int res = RUN_ALL_TESTS();

    ros::shutdown();

    return res;
}

