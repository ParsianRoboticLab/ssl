cmake_minimum_required(VERSION 3.12)
project(src)

set(CMAKE_CXX_STANDARD 14)

add_executable(src
        auto_decider/auto_decider_node.cpp
        auto_decider/auto_decider_nodelet.cpp
        commonconfig/common_config_node.cpp
        commonconfig/common_config_nodelet.cpp
        parsian_agent/agent.cpp
        parsian_agent/gotopoint.cpp
        parsian_agent/gotopointavoid.cpp
        parsian_agent/kick.cpp
        parsian_agent/newbangbang.cpp
        parsian_agent/onetouch.cpp
        parsian_agent/receivepass.cpp
        parsian_agent/skill.cpp
        planner/c2dtree.cpp
        planner/obstacle.cpp
        planner/planner.cpp
        planner/planner_node.cpp
        planner/planner_nodelet.cpp
        agent_node.cpp
        agent_nodelet.cpp
        config.cpp)
