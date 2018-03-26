#!/bin/bash


INSTALL_SCRIPTS_DIR=$(builtin cd -q "`dirname "$0"`" > /dev/null && pwd)
# Find the parent directory that corresponds to the Parsian root
PARSIAN_ROOT="$INSTALL_SCRIPTS_DIR"
while [[ ! -d "$PARSIAN_ROOT/.catkin_tools" ]]; do
        PARSIAN_ROOT=$(dirname "$PARSIAN_ROOT")
        if [[ "$PARSIAN_ROOT" == "/" ]]; then
                echo "Warning: Could not find ROS catkin workspace!"
                return
        fi
done
$LAST_WORKING_DIR=$(pwd)

sudo apt-get install vim git --assume-yes
sudo apt-get install zsh curl --assume-yes
sh -c "$(curl -fsSL https://raw.githubusercontent.com/robbyrussell/oh-my-zsh/master/tools/install.sh)"

# install ROS
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key 421C365BD9FF1F717815A3895523BAEEB01FA116
sudo apt-get update
case $(lsb_release -sr) in
1[5-6]*)
	echo "ROS KINETIC"
	sudo apt-get install ros-kinetic-desktop-full --assume-yes
	apt-cache search ros-kinetic
	echo "source /opt/ros/kinetic/setup.bash" >> ~/.bashrc
	echo "source /opt/ros/kinetic/setup.zsh" >> ~/.zshrc
	;;
1[3-4]*)
	echo "ROS INDIGO"
	sudo apt-get install ros-indigo-desktop-full --assume-yes
	apt-cache search ros-indigo
	echo "source /opt/ros/indigo/setup.bash" >> ~/.bashrc
	echo "source /opt/ros/indigo/setup.zsh" >> ~/.zshrc
	;;
17*)
	echo "ROS LUNAR"
	sudo apt-get install ros-lunar-desktop-full --assume-yes
	apt-cache search ros-lunar	
	echo "source /opt/ros/lunar/setup.bash" >> ~/.bashrc
	echo "source /opt/ros/lunar/setup.zsh" >> ~/.zshrc
	;;
*)
	echo "ROS NOT FOUND"
	;;
esac

sudo apt-get install python-rosinstall python-rosinstall-generator python-wstool build-essential --assume-yes

# Install Catkin tools
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu `lsb_release -sc` main" > /etc/apt/sources.list.d/ros-latest.list'
wget http://packages.ros.org/ros.key -O - | sudo apt-key add -
sudo apt-get update
sudo apt-get install python-catkin-tools --assume-yes


cd $PARSIAN_ROOT
for i in `ls`;do
	if [ -d $i ];then
		cd $i
		if [ -e ./deps.sh ];then
			./deps.sh
		fi
		cd ..
	fi
done
./parsian_msgs/scripts/meta/install_deps.sh
./parsian_communication/script/install_dep.sh
sudo apt-get install protobuf-compiler libprotobuf-dev --assume-yes
