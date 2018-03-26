#!/bin/zsh

# Save the directory this script is located in
SCRIPTS_DIR=$(builtin cd -q "`dirname "$0"`" > /dev/null && pwd)
# Find the parent directory that corresponds to the Parsian root
PARSIAN_ROOT="$SCRIPTS_DIR"
while [[ ! -d "$PARSIAN_ROOT/.catkin_tools" ]]; do
	PARSIAN_ROOT=$(dirname "$PARSIAN_ROOT")
	if [[ "$PARSIAN_ROOT" == "/" ]]; then
		echo "Warning: Could not find ROS catkin workspace!"
		return
	fi
done
# Initialise other variables
INSTALLPATH="/parsian"

# Set up enviroment variable for catkin and ROS
source $PARSIAN_ROOT/devel/setup.zsh
export ROSCONSOLE_FORMAT='[${severity}][${node}->${function}:${line}]: ${message}'


function pullgit() {
	if ! git pull --rebase; then
		echo "---"
		read -n1 -p "Did the pull get refused because of unstaged changes [y/N]?" response
		echo
		if [[ $response == "y" || $response == "Y" ]]; then
			echo "Yes: Ok, I'm temporarily stashing away those changes..."
			git status
			git stash save "Changes stashed by parsian pull to allow a rebase" && {
				git pull --rebase
				git stash pop || echo "Couldn't pop the stashed changes! Please check 'git stash list'..."
				echo "The stashed changes have been reapplied to the working directory!"
			}
		else
			echo "No: Ok, then please resolve the problem and try again."
		fi
	fi
}

function check() {
	ps cax | grep $1 > /dev/null
	if [ $? -eq 0 ]; then
		echo "$1 is running."
	else
		echo "$1 is not running."
		$1 > /dev/null &
		echo "$1 is running ..."
	fi
	
}

function parsian() {
	TEMP_DIR=$(pwd)
	cd $PARSIAN_ROOT
	case "$1" in
		run-grsim)
			ps cax | grep grsim
			if [ $? -eq 0 ]; then
  				echo "Grsim is running."
			else
  				echo "Grsim is not running."
				# TODO : Run Grsim.
			fi
			if [ $# -ge 2 ] && [[ "$2" == '--ai' ]];then
				roslaunch parsian_tools grsim.launch & rosrun parsian_ai parsian_ai_node
			else
				roslaunch parsian_tools grsim.launch
			fi
			;;
		run-real)
			if [ $# -ge 2 ] && [[ "$2" == '--ai' ]];then
				roslaunch parsian_tools real.launch
			else
				roslaunch parsian_tools real-with-ai.launch
			fi
			;;
		rebuild)
			catkin clean -y
			catkin build "${@:2}"
			;;
		behavior)
			case $2 in
			remove) # TODO : This feature is kinda tricky
			;;
			add)
				temp=`pwd`
				cd $PARSIAN_ROOT/src/parsian_ssl/parsian_tools
				cd script/auto-generate
				./behavior.py ${@:3}
				cd $PARSIAN_ROOT/src/parsian_ssl/parsian_ai/cfg
				chmod a+x ./$3.cfg
				cd $temp
			;;
			list)
			roscd parsian_ai
			if [  -d ./src/behavior ];then
				echo "###Behaviors:"
				cd ./src/behavior
				for i in `ls`;do
					j=${i%.cpp}
					echo ${j#behavior}
				done
				echo "###End of Behaviors"
				cd ..
			else
				echo "Behavior Directory Doesn't Exists."
			fi
			;;
			help|-h|--help|*)
				cat <<EOS
Usage parsian behavior [command] [arg=optinal]
Commands:
  add			Add a new behavior if it doesn't exist in parsian_ai
  remove		Remove a current behavior from parsian_ai
  list			Show the list of behaviors inside parsian_ai
  help			Display this help message
EOS
			;;
			esac
			;;
		help|-h|--help)
			cat <<EOS
Usage: parsian [command] [arg=optinal]
Commands:
  help			Display this help message
  run-grsim		Run enough node to run a game in grsim  (arg=--ai run ai along with)
  run-real		Run enough node to run a game in realworld (arg=--ai run ai along with)
  rebuild		clean and then build the specified packages, if nothing mentioned rebuild all packages
  behavior		Behavior Commands
  [catkin]		If command is not valid for parsian, catkin will be run instead of parsian
EOS
			;;
		*)
			if [ $# -eq 0 ];then
				cd $TEMP_DIR
				echo "Unrecognised parsian command!"
				echo "Try: parsian help"
				return
			fi
			catkin "$@"
esac
cd $TEMP_DIR

}
