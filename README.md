# Parsian SSL

[![CircleCI](https://circleci.com/gh/ParsianRoboticLab/ssl/tree/develop.svg?style=svg)](https://circleci.com/gh/ParsianRoboticLab/ssl/tree/develop)
[![CodeFactor](https://www.codefactor.io/repository/github/parsianroboticlab/ssl/badge?s=ae124388adc531f2cb4c8fc1621e1f251f0c9747)](https://www.codefactor.io/repository/github/parsianroboticlab/ssl)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

The **Parsian SSL** stack is the full software framework used by [Parsian Robotic Lab](https://github.com/ParsianRoboticLab) to compete in the [RoboCup Small Size League (SSL)](https://ssl.robocup.org/). It is built on top of [ROS](https://www.ros.org/) and provides a complete pipeline from raw vision and referee data all the way down to individual robot motion commands.

---

## Table of Contents

- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building](#building)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

---

## Architecture

The stack is organized as a collection of ROS packages, each responsible for a distinct layer of the pipeline:

```
SSL Vision / Referee / grSim
          │
          ▼
parsian_protobuf_wrapper   ← decodes UDP protobuf packets into ROS messages
          │
          ▼
parsian_world_model        ← sensor fusion & Kalman filtering → world model
          │
          ▼
parsian_ai                 ← multi-agent behavioral AI & play execution
          │
          ▼
parsian_agent              ← per-robot path planning & motion control
          │
          ▼
parsian_communication      ← serializes commands & sends to robots via radio
```

| Package | Description |
|---|---|
| [`parsian_protobuf_wrapper`](parsian_protobuf_wrapper/) | Wraps SSL Vision, grSim, and SSL Referee protobuf UDP streams into ROS topics |
| [`parsian_world_model`](parsian_world_model/) | Reads vision data, runs a Kalman-filter based tracker, and publishes a fused world-model |
| [`parsian_ai`](parsian_ai/) | Behavioral AI layer: evaluates game state, selects plays, and assigns per-robot tasks |
| [`parsian_agent`](parsian_agent/) | Per-robot node: consumes task messages and produces low-level motion commands via path planning |
| [`parsian_communication`](parsian_communication/) | Encodes robot commands into binary packets and transmits them over a serial/radio link |
| [`parsian_msgs`](parsian_msgs/) | Shared ROS message and service definitions used across all packages |
| [`parsian_util`](parsian_util/) | Shared utility library: geometry primitives, action helpers, and math utilities |
| [`parsian_tools`](parsian_tools/) | Miscellaneous tooling and helper nodes |
| [`rqt_parsian_gui`](rqt_parsian_gui/) | rqt-based graphical monitor and operator interface |

---

## Prerequisites

- **OS**: Ubuntu 16.04 (Xenial) or later
- **ROS**: [ROS Kinetic](http://wiki.ros.org/kinetic/Installation) (or a compatible distro)
- **Build tools**: `catkin_tools` (`catkin build`)
- **System libraries**:
  - `libqt4-dev`, `libqjson-dev`
  - `protobuf-compiler`, `libprotobuf-dev`
  - `libeigen3-dev`
  - `curl`
- **ROS packages**: `dynamic-reconfigure`, `nodelet`, `roslint`, `rqt-gui-cpp`, `rqt-gui`

---

## Installation

A bootstrapping script handles cloning all required repositories and installing dependencies:

```bash
sh -c "$(curl -fsSL https://gist.githubusercontent.com/mahi97/60295c82e21215701d42d4c1e679ac1f/raw/66662032274aa55888099138884748cd2a47f092/install.sh clone)"
```

> **Note:** Review the script before running it. It will clone this repository into a catkin workspace and install the necessary system and ROS dependencies.

---

## Building

After installation, build the entire workspace with:

```bash
cd ~/catkin_ws          # or wherever the workspace was created
catkin build
source devel/setup.bash
```

---

## Usage

Launch the full stack (vision receiver → world model → AI → agent → communication):

```bash
# Start the protobuf wrappers (SSL Vision, Referee, grSim)
roslaunch parsian_protobuf_wrapper protos.launch

# Start the world model
roslaunch parsian_world_model parsian_world_model.launch

# Start the AI
roslaunch parsian_ai ai.launch

# Open the operator GUI
rqt --perspective-file $(rospack find rqt_parsian_gui)/perspectives/Main.perspective
```

For simulation with **grSim**, make sure grSim is running and listening on the default ports before launching the wrappers.

---

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on naming conventions, coding style, and how to submit changes.

---

## License

This project is licensed under the **GNU Lesser General Public License v3.0**. See [LICENSE](LICENSE) for the full text.

---

## Acknowledgements

Parsian SSL is built and maintained by the Parsian Robotic Lab team at Amirkabir University of Technology (Tehran Polytechnic). Core contributors include:

- Mohammad Mahdi Rahimi
- Ali Gavahi
- Mohammad Mahdi Shirazi
- Kian Behzad
- Hamidreza Roodabeh
- Fateme Hashemi
- Nadia Moradi

See [CHANGELOG](CHANGELOG) for a full history of contributions.


