
#include <parsian_protobuf_wrapper/referee.pb.h>
#include "parsian_protobuf_wrapper/ssl-refbox/convert/convert_referee.h"

#include "parsian_protobuf_wrapper/ssl-vision/convert/convert_units.h"


namespace pr {

/**
 * Converts protoBuf Referee messages to the ROS version.
 */
parsian_msgs::ssl_refree_wrapper convert_referee_data(SSL_Referee protoRef, bool us_is_yellow) {
    parsian_msgs::ssl_refree_wrapper rosRef;
    rosRef.command_counter = protoRef.command_counter();
    rosRef.packet_timestamp = protoRef.packet_timestamp();

    rosRef.stage = convert_referee_stage(protoRef.stage());
    rosRef.stage_time_left = protoRef.stage_time_left();

    rosRef.command = convert_referee_command(protoRef.command(), us_is_yellow);
    rosRef.command_timestamp = protoRef.command_timestamp();

    if (us_is_yellow) {
        rosRef.us = convert_referee_team_info(protoRef.yellow());
        rosRef.them = convert_referee_team_info(protoRef.blue());
    } else {
        rosRef.us = convert_referee_team_info(protoRef.blue());
        rosRef.them = convert_referee_team_info(protoRef.yellow());
    }

    rosRef.designated_position.x = mm_to_m(protoRef.designated_position().x());
    rosRef.designated_position.y = mm_to_m(protoRef.designated_position().y());

    return rosRef;
}


/**
 * Converts protoBuf Referee states to the ROS version.
 */
parsian_msgs::ssl_refree_stage convert_referee_stage(SSL_Referee_Stage protoStage) {
    int stage;

    switch (protoStage) {
    case SSL_Referee_Stage_NORMAL_FIRST_HALF_PRE:
        stage = parsian_msgs::ssl_refree_stage::NORMAL_FIRST_HALF_PRE;
        break;
    case SSL_Referee_Stage_NORMAL_FIRST_HALF:
        stage = parsian_msgs::ssl_refree_stage::NORMAL_FIRST_HALF;
        break;
    case SSL_Referee_Stage_NORMAL_HALF_TIME:
        stage = parsian_msgs::ssl_refree_stage::NORMAL_HALF_TIME;
        break;
    case SSL_Referee_Stage_NORMAL_SECOND_HALF_PRE:
        stage = parsian_msgs::ssl_refree_stage::NORMAL_SECOND_HALF_PRE;
        break;
    case SSL_Referee_Stage_NORMAL_SECOND_HALF:
        stage = parsian_msgs::ssl_refree_stage::NORMAL_SECOND_HALF;
        break;
    case SSL_Referee_Stage_EXTRA_TIME_BREAK:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_TIME_BREAK;
        break;
    case SSL_Referee_Stage_EXTRA_FIRST_HALF_PRE:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_FIRST_HALF_PRE;
        break;
    case SSL_Referee_Stage_EXTRA_FIRST_HALF:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_FIRST_HALF;
        break;
    case SSL_Referee_Stage_EXTRA_HALF_TIME:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_HALF_TIME;
        break;
    case SSL_Referee_Stage_EXTRA_SECOND_HALF_PRE:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_SECOND_HALF_PRE;
        break;
    case SSL_Referee_Stage_EXTRA_SECOND_HALF:
        stage = parsian_msgs::ssl_refree_stage::EXTRA_SECOND_HALF;
        break;
    case SSL_Referee_Stage_PENALTY_SHOOTOUT_BREAK:
        stage = parsian_msgs::ssl_refree_stage::PENALTY_SHOOTOUT_BREAK;
        break;
    case SSL_Referee_Stage_PENALTY_SHOOTOUT:
        stage = parsian_msgs::ssl_refree_stage::PENALTY_SHOOTOUT;
        break;
    case SSL_Referee_Stage_POST_GAME:
        stage = parsian_msgs::ssl_refree_stage::POST_GAME;
        break;
    }

    parsian_msgs::ssl_refree_stage rosStage;
    rosStage.stage = stage;

    return rosStage;
}


/**
 * Converts protoBuf Referee commands to the ROS version.
 */
parsian_msgs::ssl_refree_command convert_referee_command(SSL_Referee_Command protoCommand, bool us_is_yellow) {
    int command;

    switch (protoCommand) {
    case SSL_Referee_Command_HALT:
        command = parsian_msgs::ssl_refree_command::HALT;
        break;
    case SSL_Referee_Command_STOP:
        command = parsian_msgs::ssl_refree_command::STOP;
        break;
    case SSL_Referee_Command_NORMAL_START:
        command = parsian_msgs::ssl_refree_command::NORMAL_START;
        break;
    case SSL_Referee_Command_FORCE_START:
        command = parsian_msgs::ssl_refree_command::FORCE_START;
        break;
    }

    if (us_is_yellow) {
        switch (protoCommand) {
        case SSL_Referee_Command_PREPARE_KICKOFF_YELLOW:
            command = parsian_msgs::ssl_refree_command::PREPARE_KICKOFF_US;
            break;
        case SSL_Referee_Command_PREPARE_KICKOFF_BLUE:
            command = parsian_msgs::ssl_refree_command::PREPARE_KICKOFF_THEM;
            break;
        case SSL_Referee_Command_PREPARE_PENALTY_YELLOW:
            command = parsian_msgs::ssl_refree_command::PREPARE_PENALTY_US;
            break;
        case SSL_Referee_Command_PREPARE_PENALTY_BLUE:
            command = parsian_msgs::ssl_refree_command::PREPARE_PENALTY_THEM;
            break;
        case SSL_Referee_Command_DIRECT_FREE_YELLOW:
            command = parsian_msgs::ssl_refree_command::DIRECT_FREE_US;
            break;
        case SSL_Referee_Command_DIRECT_FREE_BLUE:
            command = parsian_msgs::ssl_refree_command::DIRECT_FREE_THEM;
            break;
        case SSL_Referee_Command_INDIRECT_FREE_YELLOW:
            command = parsian_msgs::ssl_refree_command::INDIRECT_FREE_US;
            break;
        case SSL_Referee_Command_INDIRECT_FREE_BLUE:
            command = parsian_msgs::ssl_refree_command::INDIRECT_FREE_THEM;
            break;
        case SSL_Referee_Command_TIMEOUT_YELLOW:
            command = parsian_msgs::ssl_refree_command::TIMEOUT_US;
            break;
        case SSL_Referee_Command_TIMEOUT_BLUE:
            command = parsian_msgs::ssl_refree_command::TIMEOUT_THEM;
            break;
        case SSL_Referee_Command_GOAL_YELLOW:
            command = parsian_msgs::ssl_refree_command::GOAL_US;
            break;
        case SSL_Referee_Command_GOAL_BLUE:
            command = parsian_msgs::ssl_refree_command::GOAL_THEM;
            break;
        case SSL_Referee_Command_BALL_PLACEMENT_YELLOW:
            command = parsian_msgs::ssl_refree_command::BALL_PLACEMENT_US;
            break;
        case SSL_Referee_Command_BALL_PLACEMENT_BLUE:
            command = parsian_msgs::ssl_refree_command::BALL_PLACEMENT_THEM;
            break;
        }
    } else {
        switch (protoCommand) {
        case SSL_Referee_Command_PREPARE_KICKOFF_BLUE:
            command = parsian_msgs::ssl_refree_command::PREPARE_KICKOFF_US;
            break;
        case SSL_Referee_Command_PREPARE_KICKOFF_YELLOW:
            command = parsian_msgs::ssl_refree_command::PREPARE_KICKOFF_THEM;
            break;
        case SSL_Referee_Command_PREPARE_PENALTY_BLUE:
            command = parsian_msgs::ssl_refree_command::PREPARE_PENALTY_US;
            break;
        case SSL_Referee_Command_PREPARE_PENALTY_YELLOW:
            command = parsian_msgs::ssl_refree_command::PREPARE_PENALTY_THEM;
            break;
        case SSL_Referee_Command_DIRECT_FREE_BLUE:
            command = parsian_msgs::ssl_refree_command::DIRECT_FREE_US;
            break;
        case SSL_Referee_Command_DIRECT_FREE_YELLOW:
            command = parsian_msgs::ssl_refree_command::DIRECT_FREE_THEM;
            break;
        case SSL_Referee_Command_INDIRECT_FREE_BLUE:
            command = parsian_msgs::ssl_refree_command::INDIRECT_FREE_US;
            break;
        case SSL_Referee_Command_INDIRECT_FREE_YELLOW:
            command = parsian_msgs::ssl_refree_command::INDIRECT_FREE_THEM;
            break;
        case SSL_Referee_Command_TIMEOUT_BLUE:
            command = parsian_msgs::ssl_refree_command::TIMEOUT_US;
            break;
        case SSL_Referee_Command_TIMEOUT_YELLOW:
            command = parsian_msgs::ssl_refree_command::TIMEOUT_THEM;
            break;
        case SSL_Referee_Command_GOAL_BLUE:
            command = parsian_msgs::ssl_refree_command::GOAL_US;
            break;
        case SSL_Referee_Command_GOAL_YELLOW:
            command = parsian_msgs::ssl_refree_command::GOAL_THEM;
            break;
        case SSL_Referee_Command_BALL_PLACEMENT_BLUE:
            command = parsian_msgs::ssl_refree_command::BALL_PLACEMENT_US;
            break;
        case SSL_Referee_Command_BALL_PLACEMENT_YELLOW:
            command = parsian_msgs::ssl_refree_command::BALL_PLACEMENT_THEM;
            break;
        }
    }

    parsian_msgs::ssl_refree_command rosCommand;
    rosCommand.command = command;

    return rosCommand;
}


/**
 * Converts protoBuf TeamInfo to the ROS version.
 */
parsian_msgs::ssl_refree_teaminfo convert_referee_team_info(SSL_Referee_TeamInfo protoInfo) {
    parsian_msgs::ssl_refree_teaminfo rosInfo;

    rosInfo.name = protoInfo.name();
    rosInfo.score = protoInfo.score();
    rosInfo.red_cards = protoInfo.red_cards();

    for (int i = 0; i < protoInfo.yellow_card_times_size(); ++i) {
        rosInfo.yellow_card_times.push_back(protoInfo.yellow_card_times(i));
    }

    rosInfo.yellow_cards = protoInfo.yellow_cards();
    rosInfo.timeouts = protoInfo.timeouts();
    rosInfo.timeout_time = protoInfo.timeout_time();
    rosInfo.goalie = protoInfo.goalie();

    return rosInfo;
}

}
