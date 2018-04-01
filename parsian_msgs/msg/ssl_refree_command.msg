# A referee command.
# This can take on any of the values defined below.
uint8 command

# All robots should completely stop moving.
uint8 HALT = 0
# Robots must keep 50 cm from the ball.
uint8 STOP = 1
# A prepared kickoff or penalty may now be taken.
uint8 NORMAL_START = 2
# The ball is dropped and free for either team.
uint8 FORCE_START = 3
# The us team may move into kickoff position.
uint8 PREPARE_KICKOFF_US = 4
# The them team may move into kickoff position.
uint8 PREPARE_KICKOFF_THEM = 5
# The us team may move into penalty position.
uint8 PREPARE_PENALTY_US = 6
# The them team may move into penalty position.
uint8 PREPARE_PENALTY_THEM = 7
# The us team may take a direct free kick.
uint8 DIRECT_FREE_US = 8
# The them team may take a direct free kick.
uint8 DIRECT_FREE_THEM = 9
# The us team may take an indirect free kick.
uint8 INDIRECT_FREE_US = 10
# The them team may take an indirect free kick.
uint8 INDIRECT_FREE_THEM = 11
# The us team is currently in a timeout.
uint8 TIMEOUT_US = 12
# The them team is currently in a timeout.
uint8 TIMEOUT_THEM = 13
# The us team just scored a goal.
# For information only.
# For rules compliance, teams must treat as STOP.
uint8 GOAL_US = 14
# The them team just scored a goal.
uint8 GOAL_THEM = 15
# Equivalent to STOP, but the us team must pick up the ball and
# drop it in the Designated Position.
uint8 BALL_PLACEMENT_US = 16
# Equivalent to STOP, but the them team must pick up the ball and drop
# it in the Designated Position.
uint8 BALL_PLACEMENT_THEM = 17
