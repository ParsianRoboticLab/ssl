#ifndef _CONVERT_REFEREE
#define _CONVERT_REFEREE

#include "parsian_protobuf_wrapper/referee.pb.h"

#include "parsian_msgs/ssl_refree_wrapper.h"

namespace pr {
/**
 * Converts protoBuf Referee messages to the ROS version.
 */
parsian_msgs::ssl_refree_wrapper convert_referee_data(SSL_Referee protoRef, bool us_is_yellow);

/**
 * Converts protoBuf Referee states to the ROS version.
 */
parsian_msgs::ssl_refree_stage convert_referee_stage(SSL_Referee_Stage protoStage);

/**
 * Converts protoBuf Referee commands to the ROS version.
 */
parsian_msgs::ssl_refree_command convert_referee_command(SSL_Referee_Command protoCommand, bool us_is_yellow);

/**
 * Converts protoBuf TeamInfo to the ROS version.
 */
parsian_msgs::ssl_refree_teaminfo convert_referee_team_info(SSL_Referee_TeamInfo protoInfo);
}

#endif // _CONVERT_REFEREE