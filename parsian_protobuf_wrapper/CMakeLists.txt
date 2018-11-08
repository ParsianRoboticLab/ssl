cmake_minimum_required(VERSION 2.8.3)
project(parsian_protobuf_wrapper)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
		dynamic_reconfigure
		parsian_msgs
		roscpp
		rospy
		nodelet
		roslint
		)


roslint_cpp()

## System dependencies are found with CMake's conventions


## Qt4 
find_package(Qt4 REQUIRED COMPONENTS QtNetwork QtCore)

include(${QT_USE_FILE})
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.hpp)

qt4_wrap_cpp(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.cpp)

## Protobuf
find_package(Protobuf REQUIRED)

## Set Parameter For Generate Protobuf Files
set(proto_dir ${PROJECT_SOURCE_DIR}/proto)
file(GLOB proto_files "${proto_dir}/*.proto")
message(STATUS "Proto Source Dir: ${proto_dir}")
message(STATUS "Proto Source Files: ${proto_files}")

# Set up destination directories
catkin_destinations()
set(proto_gen_cpp_dir ${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_INCLUDE_DESTINATION})
set(proto_gen_python_dir ${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_PYTHON_DESTINATION})
file(MAKE_DIRECTORY ${proto_gen_cpp_dir})
file(MAKE_DIRECTORY ${proto_gen_python_dir})
set(protogen_include_dirs ${proto_gen_cpp_dir}/../ ${proto_gen_python_dir})

# Create lists of files to be generated.
set(proto_gen_cpp_files "")
set(proto_gen_python_files "")


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

## Generate dynamic reconfigure parameters in the 'cfg' folder
generate_dynamic_reconfigure_options(
		cfg/vision.cfg
		cfg/referee.cfg
		cfg/grsim.cfg
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
		INCLUDE_DIRS
		include
		${protogen_include_dirs}
		LIBRARIES pthread
		CATKIN_DEPENDS nodelet dynamic_reconfigure parsian_msgs
		DEPENDS PROTOBUF
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
		include
		${catkin_INCLUDE_DIRS}
		${CMAKE_CURRENT_BINARY_DIR} ##dont know if its important-kian-
		${PROTOBUF_INCLUDE_DIR}
		${PROTOBUF_INCLUDE_DIR}/../../
)

### Compile Protobuf files

foreach(proto_file ${proto_files})
	get_filename_component(proto_name ${proto_file} NAME_WE)
	list(APPEND proto_gen_cpp_files ${proto_gen_cpp_dir}/${proto_name}.pb.h ${proto_gen_cpp_dir}/${proto_name}.pb.cc)
	list(APPEND proto_gen_python_files ${proto_gen_python_dir}/${proto_name}_pb2.py)
endforeach()

# Run protoc and generate language-specific headers.
add_custom_command(
		OUTPUT ${proto_gen_cpp_files} ${proto_gen_python_files}
		COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${proto_dir} --cpp_out=${proto_gen_cpp_dir} --python_out=${proto_gen_python_dir} ${proto_files}

		DEPENDS ${PROTOBUF_PROTOC_EXECUTABLE} ${proto_files}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_source_files_properties(${proto_gen_cpp_files} PROPERTIES GENERATED TRUE)
set_source_files_properties(${proto_gen_python_files} PROPERTIES GENERATED TRUE)

# Make the python message folder a python package.
file(APPEND ${proto_gen_python_dir}/__init__.py "")

add_custom_target(${PROJECT_NAME}_generate_headers
		DEPENDS ${proto_gen_cpp_files} ${proto_gen_python_files}
		)



## Declare a C++ library

add_library(${PROJECT_NAME}_proto
		${proto_gen_cpp_files}
		)

add_library(${PROJECT_NAME}_network
		src/common/net/netraw.cpp
		src/common/net/robocup_ssl_client.cpp
		src/common/net/udpsend.cpp
		)

add_library(${PROJECT_NAME}_vision_convert
		src/ssl-vision/convert/convert_geometry.cpp
		src/ssl-vision/convert/convert_units.cpp
		src/ssl-vision/convert/convert_detection.cpp)

add_library(${PROJECT_NAME}_refbox_convert
		src/ssl-refbox/convert/convert_referee.cpp)


add_library(${PROJECT_NAME}_grsim_nodelet
		src/grsim/grsim_nodelet.cpp
		)

add_library(${PROJECT_NAME}_vision_nodelet
		src/ssl-vision/parsian_vision_nodelet.cpp
		)

add_library(${PROJECT_NAME}_refbox_nodelet
		src/ssl-refbox/parsian_referee_nodelet.cpp
		)

## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
add_dependencies(
		${PROJECT_NAME}_proto
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_generate_headers)

add_dependencies(
		${PROJECT_NAME}_network
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
)

add_dependencies(
		${PROJECT_NAME}_vision_convert
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_proto
)


add_dependencies(
		${PROJECT_NAME}_refbox_convert
		${PROJECT_NAME}_vision_convert
		${PROJECT_NAME}_gencfg
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_proto
)

add_dependencies(${PROJECT_NAME}_vision_nodelet
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)


add_dependencies(${PROJECT_NAME}_refbox_nodelet
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

add_dependencies(${PROJECT_NAME}_grsim_nodelet
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)


## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide

add_executable(${PROJECT_NAME}_vision_node
		src/ssl-vision/parsian_vision_node.cpp)

add_executable(${PROJECT_NAME}_refbox_node
		src/ssl-refbox/parsian_referee_node.cpp
		)

add_executable(${PROJECT_NAME}_grsim_node
		src/grsim/grsim_node.cpp
		)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
set_target_properties(${PROJECT_NAME}_vision_node PROPERTIES OUTPUT_NAME vision PREFIX "")
set_target_properties(${PROJECT_NAME}_refbox_node PROPERTIES OUTPUT_NAME refbox PREFIX "")
set_target_properties(${PROJECT_NAME}_grsim_node PROPERTIES OUTPUT_NAME grsim PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
add_dependencies(${PROJECT_NAME}_vision_node
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

add_dependencies(${PROJECT_NAME}_refbox_node
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)


add_dependencies(${PROJECT_NAME}_grsim_node
		${${PROJECT_NAME}_EXPORTED_TARGETS}
		${catkin_EXPORTED_TARGETS}
		${PROJECT_NAME}_gencfg
		)

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}_proto
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		)


target_link_libraries(${PROJECT_NAME}_network
		${PROJECT_NAME}_proto
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		${QT_LIBRARIES}
		)

target_link_libraries(${PROJECT_NAME}_vision_convert
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		${PROJECT_NAME}_proto
		)

target_link_libraries(${PROJECT_NAME}_refbox_convert
		${PROJECT_NAME}_vision_convert
		${catkin_LIBRARIES}
		)

target_link_libraries(${PROJECT_NAME}_vision_node
		${catkin_LIBRARIES}
		)

target_link_libraries(${PROJECT_NAME}_vision_nodelet
		${PROJECT_NAME}_vision_convert
		${PROJECT_NAME}_network
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		${PROJECT_NAME}_proto
		)

target_link_libraries(${PROJECT_NAME}_refbox_node
		${catkin_LIBRARIES}

		)

target_link_libraries(${PROJECT_NAME}_refbox_nodelet
		${PROJECT_NAME}_refbox_convert
		${PROJECT_NAME}_network
		${PROJECT_NAME}_vision_convert
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		${PROJECT_NAME}_proto
		)

target_link_libraries(${PROJECT_NAME}_grsim_nodelet
		${PROJECT_NAME}_proto
		${PROJECT_NAME}_network
		${catkin_LIBRARIES}
		${PROTOBUF_LIBRARIES}
		)

target_link_libraries(${PROJECT_NAME}_grsim_node
		${catkin_LIBRARIES}
		${PROJECT_NAME}_grsim_nodelet
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
# install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_node
#   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )
install(TARGETS
		${PROJECT_NAME}_proto ${PROJECT_NAME}_network
		${PROJECT_NAME}_grsim_nodelet ${PROJECT_NAME}_grsim_node
		${PROJECT_NAME}_vision_convert ${PROJECT_NAME}_vision_nodelet ${PROJECT_NAME}_vision_node
		${PROJECT_NAME}_refbox_convert ${PROJECT_NAME}_refbox_nodelet ${PROJECT_NAME}_refbox_node
		ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
		LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
		RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
		)


## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )
install(DIRECTORY ${proto_gen_cpp_dir}/
		DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
		FILES_MATCHING PATTERN "*.h"
		)

install(DIRECTORY ${proto_gen_python_dir}/
		DESTINATION ${CATKIN_PACKAGE_PYTHON_DESTINATION}
		FILES_MATCHING PATTERN "*.py"
		)

## Mark other files for installation (e.g. launch and bag files, etc.)
install(FILES
		vision_nodelet_plugin.xml
		refree_nodelet_plugin.xml
		grsim_nodelet_plugin.xml
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
		)

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_parsian_protobuf_wrapper.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)


#if(CATKIN_ENABLE_TESTING)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") # TODO : FIX THIS
#  add_rostest_gtest(VisionTest test/VisionTest.test test/VisionTest.cpp)
#  catkin_add_gtest(VisionTest test/VisionTest.cpp)
#  target_link_libraries(VisionTest
#                        ${catkin_LIBRARIES}
#                        vision_convert
#                        )
#endif()

