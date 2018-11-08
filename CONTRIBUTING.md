How Name Nodes, Topics and Nodelet
===================



Parsian standard format for naming package, nodes, nodelet and topics.

Table of Contents
-----------------
[TOC]

----------


Packages
-------------

They should start with **parsian_** and then continue with the appropriate name that describe generally functionality of packages and its nodes. 

--------------

Nodes
-------------
In case that you package contain a `c++` node you should put the `.cpp` file of node inside `src` directory and name it like : `parsian_<package_name>_node.cpp`
If you have more than one `c++` node you should make a separate folder for each in `src` directory and put the main file of your nodes inside its own directory, Naming multiple node be like : 
`parsian_<package_name>_<node_name>_node.cpp`

-------------

Nodelet
--------------
If you using nodelet for your node (recommended) you put nodelet class `c++` file beside its node source code and put header file inside a directory in `include` directory with same name of directory that contain source code. 

----------------

Topics
----------------
There's no real rule for naming a topic but you should attention if you'll running multiple of these node in same time it's better to use private node handle instead of variant name from public node handle.

> **Example** :
> use `agent0/command` instead of `agent_command0`

------------

How remap/rename nodes name in launch files ?
--------------

Of Course `parsian_<pkg_name>_<node_name>_node` is a long name to use, as a shortcut just remap them to `<node_name>` or if you have more than one node remap them to `<node_name>_<num>` and `<num>` start from 0. 


> **Note:**

> - if you wanna subscribe to a topic attention to node name standard for private message publisher.

Coding Style
-------------
install `astyle` and use following command
find -regextype egrep -regex '.*\.[ch](pp)?$' -exec astyle '{}' -A2 --indent=spaces=4 --pad-oper --unpad-paren --pad-header --convert-tabs \;

