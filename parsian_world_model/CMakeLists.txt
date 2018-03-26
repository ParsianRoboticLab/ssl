cmake_minimum_required(VERSION 2.8.3)
project(parsian_world_model)

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

find_package(Qt4 REQUIRED COMPONENTS QtCore)
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${QT_DEFINITIONS})

file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.hpp)

qt4_wrap_cpp(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.cpp)


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

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
		cfg/worldmodel.cfg
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
		LIBRARIES ${PROJECT_NAME} ${PROJECT_NAME}_util ${PROJECT_NAME}_nodelet
		CATKIN_DEPENDS dynamic_reconfigure parsian_msgs roscpp rospy nodelet parsian_util
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
		src/wm/visionclient.cpp
		src/wm/visionbelief.cpp
		src/wm/worldmodel.cpp
		src/wm/movingobject.cpp
		src/wm/rawobject.cpp
		src/wm/halfworld.cpp
		src/wm/robot.cpp
		src/wm/ball.cpp
		src/config.cpp
		)

add_library(${PROJECT_NAME}_util
		src/util/robottracker.cpp
		src/util/kalman.cpp
                src/util/newkalman.cpp
                src/util/balltracker.cpp
		src/util/tracker.cpp
		src/util/matrix.cpp
		)

add_library(${PROJECT_NAME}_nodelet
		src/worldmodel_nodelet.cpp
		)

add_executable(${PROJECT_NAME}_node
		src/worldmodel_node.cpp
		)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
add_dependencies(${PROJECT_NAME}
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

add_dependencies(${PROJECT_NAME}_util
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		)

add_dependencies(${PROJECT_NAME}_node
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		)

add_dependencies(${PROJECT_NAME}_nodelet
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		)
## Specify libraries to link a library or executable target against


target_link_libraries(${PROJECT_NAME}
		${catkin_LIBRARIES}
		${QT_LIBRARIES}
		${PROJECT_NAME}_util
		)


target_link_libraries(${PROJECT_NAME}_util
		${catkin_LIBRARIES}
		${QT_LIBRARIES}
		)


## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}_node
		${catkin_LIBRARIES}
		${PROJECT_NAME}_nodelet
		)

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}_nodelet
		${catkin_LIBRARIES}
		${QT_LIBRARIES}
		${PROJECT_NAME}
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
install(TARGETS ${PROJECT_NAME}_util ${PROJECT_NAME} ${PROJECT_NAME}_nodelet ${PROJECT_NAME}_node
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

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_parsian_world_model.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
