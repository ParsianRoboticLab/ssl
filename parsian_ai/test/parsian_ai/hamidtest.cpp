//
// Created by Mohammad Mahdi Rahimi on 9/27/18.
//

#include <gtest/gtest.h>
#include <ros/ros.h>
#include <parsian_ai/plays/ourballplacement.h>
#include <parsian_msgs/parsian_world_model.h>

TEST(BALLPALCE, test1) {
    wm = new WorldModel();

    parsian_msgs::parsian_world_modelPtr wmmsg;
    wmmsg.reset(new parsian_msgs::parsian_world_model);

    wmmsg->ball.pos.x = 0;
    wmmsg->ball.pos.y = 0;
    wm->update(wmmsg);
    COurBallPlacement bpplay;

    EXPECT_GE(bpplay.chooseFirst(), 0);

    delete wm;
}


// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    ros::init(argc, argv, "tester");
    ros::NodeHandle nh;
    return RUN_ALL_TESTS();
}