cmake_minimum_required(VERSION 2.8.3)
project(parsian_util)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)


FILE(GLOB_RECURSE LibFiles "include/*.h")
add_custom_target(headers SOURCES ${LibFiles})

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
        roscpp
        rospy
        parsian_msgs
	roslint
        )

roslint_cpp()
roslint_python()

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui)

include(${QT_USE_FILE})
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.hpp)

qt4_wrap_cpp(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.cpp)
add_definitions(${QT_DEFINITIONS})



## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

## To declare and build messages, services or actions from within this
## package, follow these steps:
## * Let MSG_DEP_SET be the set of packages whose message types you use in
##   your messages/services/actions (e.g. std_msgs, actionlib_msgs, ...).
## * In the file package.xml:
##   * add a build_depend tag for "message_generation"
##   * add a build_depend and a run_depend tag for each package in MSG_DEP_SET
##   * If MSG_DEP_SET isn't empty the following dependency has been pulled in
##     but can be declared for certainty nonetheless:
##     * add a run_depend tag for "message_runtime"
## * In this file (CMakeLists.txt):
##   * add "message_generation" and every package in MSG_DEP_SET to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * add "message_runtime" and every package in MSG_DEP_SET to
##     catkin_package(CATKIN_DEPENDS ...)
##   * uncomment the add_*_files sections below as needed
##     and list every .msg/.srv/.action file to be processed
##   * uncomment the generate_messages entry below
##   * add every package in MSG_DEP_SET to generate_messages(DEPENDENCIES ...)

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   std_msgs  # Or other packages containing msgs
# )

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
# generate_dynamic_reconfigure_options(
#   cfg/DynReconf1.cfg
#   cfg/DynReconf2.cfg
# )

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
        LIBRARIES ${PROJECT_NAME}_geom ${PROJECT_NAME}_tools ${PROJECT_NAME}_core ${PROJECT_NAME}_math ${PROJECT_NAME}_actions
        CATKIN_DEPENDS parsian_msgs
	#        DEPENDS QT
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
        include
        ${catkin_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
)

## Declare a C++ library
add_library(${PROJECT_NAME}_geom
        src/geom/angle_deg.cpp
        src/geom/circle_2d.cpp
        src/geom/line_2d.cpp
        src/geom/matrix_2d.cpp
        src/geom/polygon_2d.cpp
        src/geom/ray_2d.cpp
        src/geom/segment_2d.cpp
        src/geom/rect_2d.cpp
        src/geom/sector_2d.cpp
        src/geom/triangle_2d.cpp
        src/geom/vector_2d.cpp
        )

add_library(${PROJECT_NAME}_math
        src/mathtools.cpp
        src/matrix.cpp
        src/statemachine.cpp
        )

add_library(${PROJECT_NAME}_tools
        src/tools/drawer.cpp
        src/tools/filewatcher.cpp
        )


add_library(${PROJECT_NAME}_actions
        src/action/action.cpp
        src/action/autogenerate/gotopointaction.cpp
        src/action/autogenerate/gotopointavoidaction.cpp
        src/action/autogenerate/receivepassaction.cpp
        src/action/autogenerate/onetouchaction.cpp
        src/action/autogenerate/kickaction.cpp
        src/action/autogenerate/noaction.cpp
        )

add_library(${PROJECT_NAME}_core
        src/core/agent.cpp
        src/core/worldmodel.cpp
        src/core/field.cpp
        src/core/ball.cpp
        src/core/team.cpp
        src/core/robot.cpp
        src/core/knowledge.cpp
        src/core/movingobject.cpp
        )

## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
add_dependencies(${PROJECT_NAME}_geom 
	${${PROJECT_NAME}_EXPORTED_TARGETS} 
	${catkin_EXPORTED_TARGETS}
)

add_dependencies(${PROJECT_NAME}_tools 
	${${PROJECT_NAME}_EXPORTED_TARGETS} 
	${catkin_EXPORTED_TARGETS}
)

add_dependencies(${PROJECT_NAME}_math 
	${${PROJECT_NAME}_EXPORTED_TARGETS} 
	${catkin_EXPORTED_TARGETS}
)

add_dependencies(${PROJECT_NAME}_core 
	${${PROJECT_NAME}_EXPORTED_TARGETS} 
	${catkin_EXPORTED_TARGETS}
)

add_dependencies(${PROJECT_NAME}_actions 
	${${PROJECT_NAME}_EXPORTED_TARGETS} 
	${catkin_EXPORTED_TARGETS}
)

## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide
# add_executable(${PROJECT_NAME}_node src/parsian_util_node.cpp)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
#add_dependencies(${PROJECT_NAME}_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}_geom
        ${catkin_LIBRARIES}
        )

target_link_libraries(${PROJECT_NAME}_math
        ${catkin_LIBRARIES}
        ${QT_LIBRARIES}
        )

target_link_libraries(${PROJECT_NAME}_tools
        ${catkin_LIBRARIES}
        ${QT_LIBRARIES}
        ${PROJECT_NAME}_geom
        )

target_link_libraries(${PROJECT_NAME}_core
        ${catkin_LIBRARIES}
        ${QT_LIBRARIES}
        ${PROJECT_NAME}_geom
        )


target_link_libraries(${PROJECT_NAME}_actions
        ${catkin_LIBRARIES}
        ${QT_LIBRARIES}
        ${PROJECT_NAME}_geom
        )

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
install(TARGETS 
		${PROJECT_NAME}_geom 
		${PROJECT_NAME}_math 
		${PROJECT_NAME}_actions 
		${PROJECT_NAME}_core
		${PROJECT_NAME}_tools
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
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_parsian_util.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
