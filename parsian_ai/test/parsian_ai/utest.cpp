//
// Created by Mohammad Mahdi Rahimi on 9/27/18.
//
#include <gtest/gtest.h>
#include <parsian_ai/ai.h>
#include <ros/ros.h>
#include <parsian_msgs/ssl_force_referee.h>
#include <parsian_msgs/ssl_refree_command.h>
#include <parsian_msgs/ssl_refree_wrapper.h>

// Declare a test
TEST(GameState, ForceCommand) {
    AI ai;
    parsian_msgs::ssl_force_refereePtr r; r.reset(new parsian_msgs::ssl_force_referee);
    parsian_msgs::ssl_refree_command c;

    /// HALT
    c.command = parsian_msgs::ssl_refree_command::HALT;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::Halt);

    // STOP
    c.command = parsian_msgs::ssl_refree_command::STOP;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::Stop);

    /// Start after Stop
    c.command = parsian_msgs::ssl_refree_command::FORCE_START;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::Start);

    /// Everything Expect `HALT` and `STOP` during Start
    bool start = true;
    for (int i = parsian_msgs::ssl_refree_command::FORCE_START; i < parsian_msgs::ssl_refree_command::INDIRECT_FREE_THEM; i++) {
        c.command = static_cast<unsigned char>(i);
        r->command = c;
        ai.forceUpdateReferee(r);
        ROS_INFO_STREAM(static_cast<int>(gameState->getState()));
        start &= (gameState->getState() == States::Start);
    }
    EXPECT_TRUE(start);

    /// Stop During Start
    c.command = parsian_msgs::ssl_refree_command::STOP;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::Stop);

    /// Direct Free-Kick after Stop
    c.command = parsian_msgs::ssl_refree_command::DIRECT_FREE_US;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::OurDirectKick);

    /// Ball Placement Position and Command
    c.command = parsian_msgs::ssl_refree_command::STOP;
    r->command = c;
    ai.forceUpdateReferee(r);
    c.command = parsian_msgs::ssl_refree_command::BALL_PLACEMENT_US;
    parsian_msgs::vector2D bp_pos; bp_pos.x = 3000; bp_pos.y = 3000;
    r->ballPlacementPos = bp_pos;
    r->command = c;
    ai.forceUpdateReferee(r);
    EXPECT_EQ(gameState->getState(), States::OurBallPlacement);
    EXPECT_EQ(wm->ballplacementPoint().x, bp_pos.x);
    EXPECT_EQ(wm->ballplacementPoint().y, bp_pos.y);

}

// Declare another test
TEST(GameState, RefereeCommand) {
    AI ai;
    parsian_msgs::ssl_refree_wrapperPtr r; r.reset(new parsian_msgs::ssl_refree_wrapper);
//    ASSERT_NO_THROW(ai.updateReferee(r));
//    ASSERT_NO_FATAL_FAILURE(ai.updateReferee(r));

}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    ros::init(argc, argv, "tester");
    ros::NodeHandle nh;
    return RUN_ALL_TESTS();
}