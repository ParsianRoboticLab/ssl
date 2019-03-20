#!/usr/bin/env python

import rospy
import os
import json
import math
from random import randint
from watchdog.events import FileSystemEventHandler
from parsian_msgs.msg import parsian_plan
from parsian_msgs.msg import vector2D
from parsian_msgs.msg import parsian_plan_agent
from parsian_msgs.msg import parsian_plan_position
from parsian_msgs.msg import parsian_plan_skill
from parsian_msgs.srv import plan_service
from parsian_msgs.srv import plan_serviceResponse
from parsian_msgs.srv import plan_serviceRequest






class Watcher(FileSystemEventHandler):
    patterns = ['*.json']
    def __init__(self, path):
        #initiate
        self.path = path
        rospy.Timer(rospy.Duration(0.5), self.my_callback)
        self.is_newEvent_happend = True

        #update_plans
        self.all_jsons_root = []        #all files with json extension
        self.all_ignoredjsons_root = [] #all ignored files with json extension
        self.all_desiredjsons_root = [] #all json files that are not in ignore file
        self.all_badjsons_root = []     #all json files that cant be opend
        self.desired_plans = {}         #all desired plans -> filepath: [parsian_plan]


    def on_any_event(self, event):
        self.is_newEvent_happend = True

    ##update all plans every 0.5 sec if a new event happend in directory
    def my_callback(self, event):
        if not self.is_newEvent_happend:
            return

        self.is_newEvent_happend = False

        self.get_all_jsons_root()

        self.get_all_ignoredjsons_root()

        self.get_all_desiredjsons_root()

        self.get_desired_plans()


    def get_all_jsons_root(self):
        self.all_jsons_root = []
        for root, dirs, files in os.walk(self.path, topdown=False):
            for name in files:
                if name.lower().endswith(".json"):
                    self.all_jsons_root.append(os.path.join(root, name))

    def get_all_ignoredjsons_root(self):
        os.chdir(self.path)
        self.all_ignoredjsons_root = []
        if not os.path.exists("plans.ignore") and not os.path.isfile("plans.ignore"):
            rospy.loginfo("plans.ignore not found")
            return
        clear_comment_lines = []#ignore all commented parts
        with open("plans.ignore") as file:
            for line in file:
                if line.find('#') == 0:
                    continue
                elif line.find('#') > 0:
                    clear_comment_lines.append(line[0: line.find('#')].rstrip())
                elif not len(line.strip()) == 0:#not a blank line
                    clear_comment_lines.append(line.rstrip())

        for i in range(len(clear_comment_lines)):
            if clear_comment_lines[i].startswith('/'):
                clear_comment_lines[i] = clear_comment_lines[i][1:]


        for line in clear_comment_lines:
            os.chdir(self.path)
            if os.path.isfile(line) and line.lower().endswith(".json"):
                self.all_ignoredjsons_root.append(os.path.join(self.path, line))
            elif os.path.isdir(os.path.join(self.path, line)):
                for root, dirs, files in os.walk(os.path.join(self.path, line), topdown=False):
                    for name in files:
                        if name.lower().endswith(".json"):
                            self.all_ignoredjsons_root.append(os.path.join(root, name))

    def get_all_desiredjsons_root(self):
        self.all_desiredjsons_root = []
        self.all_badjsons_root = []
        not_commented = []
        for line in self.all_jsons_root:
            if not line in self.all_ignoredjsons_root:
                not_commented.append(line)

        for line in not_commented:
            is_correct = True
            with open(line) as json_file:
                try:
                    json.load(json_file)
                except:
                    is_correct = False
                if is_correct:
                    self.all_desiredjsons_root.append(line)
                else:
                    self.all_badjsons_root.append(line)

    def get_desired_plans(self):
        self.desired_plans = {}
        for root in self.all_desiredjsons_root:
            self.desired_plans[root] = self.generate_parsianplan_from_json(root)

    def generate_parsianplan_from_json(self, planpath):
        plan_json = None
        with open(planpath) as json_file:
            try:
                plan_json = json.load(json_file)
            except:
                return False

        plan_message = parsian_plan()
        plan_message.planFile = planpath
        plan_message.isActive = True                                #need change in client request
        plan_message.isMaster = False                               #need change in client request
        plan_message.symmetry = False                               #need change in ai requests
        plan_message.chance = plan_json["plans"][0]["chance"]       #could be toggled in client or ai in future
        plan_message.lastDist = plan_json["plans"][0]["lastDist"]

        ##start agentsize
        #all agents that thier initPos isnt -100, except for the first kicker
        agentSize = 0
        agentSize += 1 #first kicker
        for agentsPos in plan_json["plans"][0]["agentInitPos"]:
            if agentsPos["x"] != -100:
                agentSize += 1
        plan_message.agentSize = agentSize
        ##finish agentsize

        plan_message.tags = [str(tag) for tag in plan_json["plans"][0]["tags"]]
        plan_message.planMode = str(plan_json["plans"][0]["planMode"])

        ##start ballInitPos
        ballpos = vector2D()
        ballpos.x = plan_json["plans"][0]["ballInitPos"]["x"]
        ballpos.y = plan_json["plans"][0]["ballInitPos"]["y"]
        plan_message.ballInitPos = ballpos
        ##finish ballInitPos

        plan_message.successRate = 0                                #could be toggled in client or ai in future
        plan_message.planRepeat = 0                                 #could be toggled here in future

        ##start agentInitPos
        allinitpos = []
        for initpos in plan_json["plans"][0]["agentInitPos"]:
            initpos_tmp = vector2D()
            initpos_tmp.x = initpos["x"]
            initpos_tmp.y = initpos["y"]
            allinitpos.append(initpos_tmp)
        plan_message.agentInitPos[0:len(allinitpos)] = allinitpos
        ##finish agentInitPos

        ##start agents
        agents = []
        for agent in plan_json["plans"][0]["agents"]:
            agent_tmp = parsian_plan_agent()
            agent_tmp.id = agent["ID"]
            positions = []
            for position in agent["positions"]:
                position_tmp = parsian_plan_position()
                position_tmp.angel = position["angel"]
                position_tmp.pos.x = position["pos-x"]
                position_tmp.pos.y = position["pos-y"]
                position_tmp.tolerance = position["tolerance"]
                skills = []
                for skill in position["skills"]:
                    skill_tmp = parsian_plan_skill()
                    skill_tmp.flag = skill["flag"]
                    skill_tmp.name = str(skill["name"])
                    skill_tmp.primary = skill["primary"]
                    skill_tmp.secondry = skill["secondary"]
                    if "target" in skill:
                        skill_tmp.agent = skill["target"]["agent"]
                        skill_tmp.index = skill["target"]["index"]
                    else:
                        skill_tmp.agent = -1
                        skill_tmp.index = -1
                    skills.append(skill_tmp)

                position_tmp.skills[0:len(skills)] = skills
                position_tmp.skillSize = len(skills)
                positions.append(position_tmp)
            agent_tmp.positions[0:len(positions)] = positions
            agent_tmp.posSize = len(positions)
            agents.append(agent_tmp)
        plan_message.agents[0:len(agents)] = agents
        ##finish agents

        return plan_message

    def choose_plan(self, req):
        all_matched_plans = self.get_all_matched_plans(req.plan_req.gameMode, req.plan_req.playersNum, req.plan_req.ballPos.x, req.plan_req.ballPos.y)#{planpath: isSymmetric}

        if len(all_matched_plans.keys()) == 0:
            return
        shuffle = randint(0, len(all_matched_plans.keys()) - 1)

        print(all_matched_plans)

        response = plan_serviceResponse()
        response.the_plan = self.desired_plans[all_matched_plans.keys()[shuffle]]
        response.the_plan.symmetry = all_matched_plans[all_matched_plans.keys()[shuffle]]
        response.time_us = 0
        return response

    def get_all_matched_plans(self, gameMode, playersNum, ballPosX, ballPosY):
        matched = {}
        if gameMode == 3:#KICKOFF
            for plan in self.desired_plans:
                if self.desired_plans[plan].planMode == "KICKOFF":
                    if self.desired_plans[plan].agentSize >= playersNum and self.desired_plans[plan].chance > 0 and self.desired_plans[plan].lastDist >= 0:
                        matched[plan] = False#not symmetric

        else:
            for plan in self.desired_plans:
                if self.desired_plans[plan].agentSize >= playersNum and self.desired_plans[plan].chance > 0 and self.desired_plans[plan].lastDist >= 0:
                    isMatched, isSymmetry = self.check_ballPos(plan, ballPosX, ballPosY)
                    if isMatched:
                        matched[plan] = isSymmetry
        return matched



    def check_ballPos(self, plan, ballPosX, ballPosY):
        actual_distX = self.desired_plans[plan].ballInitPos.x - ballPosX
        actual_distY = self.desired_plans[plan].ballInitPos.y - ballPosY
        actual_dist = math.sqrt(math.pow(actual_distX, 2) + math.pow(actual_distY, 2))

        symm_distX = self.desired_plans[plan].ballInitPos.x - ballPosX
        symm_distY = -self.desired_plans[plan].ballInitPos.y - ballPosY
        symm_dist = math.sqrt(math.pow(symm_distX, 2) + math.pow(symm_distY, 2))

        if actual_dist <= symm_dist and actual_dist < 2:
            return (True, False) #isMatched - isSymmetry
        if actual_dist > symm_dist and symm_dist < 2:
            return (True, True) #isMatched - isSymmetry
        else:
            return (False, False)
