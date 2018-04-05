cmake_minimum_required(VERSION 2.8.3)
project(parsian_ai)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
		dynamic_reconfigure
		parsian_msgs
		parsian_util
		roscpp
		rospy
		nodelet
		roslint
		)

roslint_cpp()
roslint_python()

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)
file(GLOB_RECURSE LibFiles "include/*.h")
add_custom_target(headers SOURCES ${LibFiles})

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui)
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${QT_DEFINITIONS})

file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.hpp)

qt4_wrap_cpp(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.cpp)

## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
catkin_python_setup()
################################################
## Declare ROS dynamic reconfigure parameters ##
################################################

## To declare and build dynamic reconfigure parameters within this
## package, follow these steps:
## * In the file package.xml:
##   * add a build_depend and a run_depend tag for "dynamic_reconfigure"
## * In this file (CMakeLists.txt):
##   * add "dynamic_reconfigure" to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * uncomment the "generate_dynamic_reconfigure_options" section below
##     and list every .cfg file to be processed

## Generate dynamic reconfigure parameters in the 'cfg' folder
generate_dynamic_reconfigure_options(
	###	cfg/pass.cfg
		cfg/direct.cfg
		cfg/mahi.cfg
		cfg/move_forward.cfg
		cfg/ai.cfg
		cfg/def_test.cfg
		cfg/behavior_server.cfg
)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
		INCLUDE_DIRS include
		LIBRARIES ${PROJECT_NAME}_nodelet ${PROJECT_NAME}
		CATKIN_DEPENDS dynamic_reconfigure parsian_msgs parsian_util roscpp rospy nodelet
		#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
		include
		${catkin_INCLUDE_DIRS}
)

## Declare a C++ library
add_library(${PROJECT_NAME}
		src/${PROJECT_NAME}/ai.cpp
		src/${PROJECT_NAME}/soccer.cpp
		src/${PROJECT_NAME}/gamestate.cpp
		src/${PROJECT_NAME}/config.cpp
		src/${PROJECT_NAME}/coach.cpp
		src/${PROJECT_NAME}/plays/dynamicattack.cpp
		src/${PROJECT_NAME}/plays/masterplay.cpp
		src/${PROJECT_NAME}/plays/ourballplacement.cpp
		src/${PROJECT_NAME}/plays/playoff.cpp
		src/${PROJECT_NAME}/plays/ourpenalty.cpp
		src/${PROJECT_NAME}/plays/theirpenalty.cpp
		src/${PROJECT_NAME}/plays/theirkickoff.cpp
		src/${PROJECT_NAME}/plays/theirindirect.cpp
		src/${PROJECT_NAME}/plays/theirdirect.cpp
		src/${PROJECT_NAME}/plays/theirballplacement.cpp
		src/${PROJECT_NAME}/plays/theirpenaltyshootout.cpp
		src/${PROJECT_NAME}/plays/ourpenaltyshootout.cpp
		src/${PROJECT_NAME}/plays/stopplay.cpp
		src/${PROJECT_NAME}/plays/halftimelineup.cpp
		src/${PROJECT_NAME}/plans/defenseplan.cpp
		src/${PROJECT_NAME}/plans/markplan.cpp
		src/${PROJECT_NAME}/plans/plan.cpp
		src/${PROJECT_NAME}/plans/positioningplan.cpp
		src/${PROJECT_NAME}/roles/role.cpp
		src/${PROJECT_NAME}/roles/block.cpp
		src/${PROJECT_NAME}/roles/playoffrole.cpp
		src/${PROJECT_NAME}/roles/stop.cpp
		src/${PROJECT_NAME}/roles/fault.cpp
		src/${PROJECT_NAME}/roles/dynamicrole.cpp
		src/${PROJECT_NAME}/roles/playmake.cpp
		)

add_library(${PROJECT_NAME}_util
		src/parsian_ai/util/defpos.cpp
		src/parsian_ai/util/knowledge.cpp
		src/parsian_ai/util/worldmodel.cpp
		src/parsian_ai/util/agent.cpp)

add_library(${PROJECT_NAME}_nodelet
		src/parsian_ai/ai_nodelet.cpp
		)

## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide

add_executable(${PROJECT_NAME}_node
		src/parsian_ai/ai_node.cpp
		)

### BEHAVIORS ### DO NOT REMOVE THIS LINE, IT'S FOR AUTO-GENERATION
#####include(./pass_CMakeLists.txt)
#include(./direct_CMakeLists.txt)
#include(./mahi_CMakeLists.txt)
#include(./move_forward_CMakeLists.txt)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
add_dependencies(${PROJECT_NAME}_node
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

add_dependencies(${PROJECT_NAME}_nodelet
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

## Specify libraries to link a library or executable target against

target_link_libraries(${PROJECT_NAME}_nodelet
		${catkin_LIBRARIES}
		${PROJECT_NAME}
		${QT_LIBRARIES}

		)
target_link_libraries(${PROJECT_NAME}_node
		${catkin_LIBRARIES}
		${QT_LIBRARIES}
		)

add_dependencies(${PROJECT_NAME}
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

add_dependencies(${PROJECT_NAME}_util
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

target_link_libraries(${PROJECT_NAME}
		${catkin_LIBRARIES}
		${QT_LIBRARIES}
		${PROJECT_NAME}_util
		)

target_link_libraries(${PROJECT_NAME}_util
		${QT_LIBRARIES}
		${catkin_LIBRARIES}
		)


#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
install(PROGRAMS
   		scripts/plan_service_server
		scripts/plan_service_client
   		DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
		)

## Mark executables and/or libraries for installation
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_nodelet ${PROJECT_NAME}_node
		ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
		LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
		RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
		)

## Mark cpp header files for installation
install(DIRECTORY include/${PROJECT_NAME}/
		DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
		FILES_MATCHING PATTERN "*.h"
		#   PATTERN ".svn" EXCLUDE
		)

## Mark other files for installation (e.g. launch and bag files, etc.)
install(FILES
		nodelet.xml
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
		)

catkin_install_python(PROGRAMS
scripts/plan_server/planserver_node.py
scripts/behavior_server/behavior_server_node.py
scripts/auto_defense_test/def_test_node.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_parsian_ai.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
