#!/bin/bash

# Save the directory this script is located in
SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
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
source "$PARSIAN_ROOT/devel/setup.bash"
export ROSCONSOLE_FORMAT='[${severity}][${node}->${function}]: ${message}'


function pullgit() {
	if ! git pull --rebase; then
		echo "---"
		read -n1 -p "Did the pull get refused because of unstaged changes [y/N]?" response
		echo
		if [[ $response == "y" || $response == "Y" ]]; then
			echo "Yes: Ok, I'm temporarily stashing away those changes..."
			git status
			git stash save "Changes stashed by nimbro pull to allow a rebase" && {
				git pull --rebase
				git stash pop || echo "Couldn't pop the stashed changes! Please check 'git stash list'..."
				echo "The stashed changes have been reapplied to the working directory!"
			}
		else
			echo "No: Ok, then please resolve the problem and try again."
		fi
	fi
}


function parsian() {
	echo "Parsian Function"
}
