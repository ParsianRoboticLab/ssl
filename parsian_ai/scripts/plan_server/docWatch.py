import time
import os
import signal
import sys

import re
import json
import random
import math

from parsian_msgs.msg import parsian_plan
from parsian_msgs.msg import parsian_plan_GUI

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import rospkg


class Watcher:
    DIRECTORY_TO_WATCH = ""

    def __init__(self):
        signal.signal(signal.SIGINT, self.signal_handler)
        self.path = rospkg.RosPack().get_path("parsian_ai")
        print ("pack path: " + self.path)
        self.path += "/plans/"
        self.__observer = Observer()
        # DIRECTORY_TO_WATCH = DIRECTORY_TO_WATCH + self.__p
        self.__event_handler = Handler(self.path)
        # l=[]
        # l.append("/home/fateme/Workspace/parsian_ws/src/parsian_ssl/parsian_ai/plans/2Pass/pass.json")
        # self.__event_handler.update_master_active(l, True, True)

    def run(self):
        self.__observer.schedule(self.__event_handler, self.path, recursive=True)
        self.__observer.start()
        try:
            while True:
                time.sleep(5)
        except:
            self.__observer.stop()
            # print("Error")
            self.__observer.join()
            sys.exit(0)

    def get_all_plans(self):
        return self.__event_handler.get_all_plans_gui_msgs()

    def update_master_active(self, name_list, plan_index, is_master, is_active):
        if len(name_list) != len(plan_index):
            print("not all plans are indexed")
            return None
        return self.__event_handler.update_master_active(name_list, plan_index, is_master, is_active)

    def choose_plan(self, player_num, game_mode, ball_x, ball_y):
        return self.__event_handler.choose_plan(player_num, game_mode, ball_x, ball_y)

    def signal_handler(self, signal, frame):
        print("\nctrl+C pressed!")
        sys.exit(0)


class Handler(FileSystemEventHandler):
    def __init__(self, p):
        self.__final_list = []
        self.__ignore = []
        self.__shuffleCount = 0
        self.__final_dict = []
        self.__all_master_plans = {}
        self.__all_active_plans = {}

        global final_list, shuffleCount, path
        path = p

        # read_plan(f[5])
        self.__final_list = self.list_valid_plans(p)

        self.plans_to_dict()

        self.shuffle_indexing(self.__final_list)  # call once

        # self.choose_plan(4, 3)
        # print (self.message_generator(self.__final_dict[0]))

        # @staticmethod
    def on_any_event(self, event):
        if event.is_directory:
            return None

        elif event.event_type == 'modified':
            # print("Received modified event - %s" % event.src_path)
            # self.add_plan(event.src_path)
            self.refresh()

        elif event.event_type == 'deleted':
            # print("Received deleted event - %s" % event.src_path)
            # self.remove_plan(event.src_path)
            self.refresh()

        elif event.event_type == 'created':
            self.refresh()
            # print("Received created event - %s." % event.src_path)

    def list_valid_plans(self, path):
        file_list = []
        for path, dirs, files in os.walk(path):
            for filename in files:
                fname = os.path.join(path, filename)
                #         printfname.rsplit('/', 1)[1]   # just file name
                file_list.append(fname)

        ignore_lst = []
        for f in file_list:  # find ignore list
            if f != None:
                if f.endswith('.ignore'):
                    ignore_lst = open(str(f)).read().split("\n")
                    file_list.remove(f)
                    break

        if '' in ignore_lst:
            ignore_lst.remove('')

        no_cmnt_ignr_lst = [li for li in ignore_lst if not li.startswith('#')]
        print(no_cmnt_ignr_lst)
        self.__ignore = no_cmnt_ignr_lst

        bad_files = []
        for f in file_list:
            if f != None:
                if f.endswith('.json'):
                    # if os.stat(str(f)).st_size != 0:
                    # print("adding " + str(f).rsplit('/', 1)[1]
                    # with open(str(f)) as json_data:
                    #     plan_data.append(json.load(json_data))
                    if os.stat(str(f)).st_size == 0:
                        bad_files.append(f)
                        print("empty: " + str(f) + " --> removed!")
                else:
                    bad_files.append(f)
                    # print("not json: " + str(f) + " --> removed!")

        file_list2 = [f for f in file_list if f not in bad_files]
        # printfile_list2

        self.__final_list = self.ignore_plans(file_list2, ignore_lst)
        return self.__final_list

    def add_plan(self, path_to_plan):
        flag = 0
        print ("adding plan...")
        if str(path_to_plan).endswith(".json"):
            if not (str(path_to_plan).split("/plans")[1]) in self.__ignore:
                for pattern in self.__ignore:
                    if pattern != '':
                        try:
                            if (re.search(pattern, str(path_to_plan).split("/plans")[1]) and
                                        re.search(pattern, str(path_to_plan).split("/plans")[1]).start() == 0):
                                flag = 1
                        except:
                            print("Invalid Expression: " + pattern)
            else:
                flag = 1

        if flag == 0:
            self.__final_list.append(path_to_plan)
            print(str(path_to_plan).split("/plans")[1] + " added.")

        return self.__final_list

    def remove_plan(self, path_to_plan):
        if path_to_plan in self.__final_list:
            print(str(path_to_plan).split("/plans")[1] + " removed.")
            self.__final_list.remove(path_to_plan)

    def refresh(self):
        global path
        self.__final_list = self.list_valid_plans(path)
        self.shuffle_indexing(self.__final_list)
        self.plans_to_dict()

    def ignore_plans(self, file_list, ignore_list):
        # files:
        new_file_list = [fi for fi in file_list if (str(fi).split("/plans")[1] not in ignore_list)]
        new_file_list2 = [fi for fi in file_list if fi not in new_file_list]
        for pattern in ignore_list:
            if pattern != '':
                try:
                    for f in new_file_list:
                        if re.search(pattern, str(f).split("/plans")[1]) and re.search(pattern, str(f).split("/plans")[1]).start() == 0:
                            new_file_list2.append(f)
                except:
                    print("Invalid Expression: " + pattern)

        print("ignored plans:")
        for fil in new_file_list2:
            str_list = str(fil).split("/plans")
            if len(str_list) > 0:
                print("\t" + str(fil).split("/plans")[1])

        last = [term for term in new_file_list if not term in new_file_list2]

        print("FINAL LIST:")
        for i in last:
            print("\t" + str(i).split("/plans")[1])

        return last

    def shuffle_indexing(self, alist):
        random.shuffle(alist)

    def choose_plan(self, player_num, game_mode, ball_x, ball_y):
        # DIRECT   = 1  INDIRECT = 2  KICKOFF  = 3
        plan_mode = ""
        if game_mode == 1:
            plan_mode = "DIRECT"
        elif game_mode == 2:
            plan_mode = "INDIRECT"
        elif game_mode == 3:
            plan_mode = "KICKOFF"

        sublist = []
        rad = 1

        active_list = self.get_master_active_plans(self.__final_dict)

        for plan in active_list:
            if self.check_plan(plan, ball_x, ball_y, rad, player_num, plan_mode):
                sublist.append(plan)

        if len(sublist) > 0:
            print("# active and valid plans: "+str(len(sublist))+"\n")
            i = self.__shuffleCount % len(sublist)
            self.__shuffleCount += 1
            print ("\n" + sublist[i]["filename"].split("plans/")[1] +
                   ": "+str(sublist[i]["index"]) + "   " + str(sublist[i]["planMode"]))
            return self.ai_message_generator(sublist[i])
        else:
            print ("of invalid plans ...")
            return self.nearest_plan(player_num, ball_x, ball_y)

    def get_master_active_plans(self, plan_list):
        master_list = []
        active_list = []

        for plan in plan_list:
            if plan["isMaster"]:
                master_list.append(plan)

        if len(master_list) > 0:
            active_list = master_list
        else:
            for plan in plan_list:
                if plan["isActive"]:
                    active_list.append(plan)

        return active_list

    def check_plan(self, plan, ball_x, ball_y, rad, player_num, plan_mode):
        DIRECT = 1
        INDIRECT = 2
        KICKOFF = 3
        if self.circle_contains(ball_x, ball_y, rad, plan["ballInitPos"]["x"], plan["ballInitPos"]["y"]):
            # print("Ball Pos Matched")
            if len(plan["agentInitPos"]) >= player_num \
                    and plan["chance"] > 0 and plan["lastDist"] >= 0 \
                    and (plan["planMode"] == plan_mode or (plan_mode == DIRECT and plan["planMode"] == INDIRECT)):
                plan["symmetry"] = False
                return True
        if self.circle_contains(ball_x, -ball_y, rad, plan["ballInitPos"]["x"], plan["ballInitPos"]["y"]):
            # print("Ball Symm Pos Matched")
            if len(plan["agentInitPos"]) >= player_num \
                    and plan["chance"] > 0 and plan["lastDist"] >= 0 \
                    and plan["planMode"] == plan_mode:
                plan["symmetry"] = True
                return True
        return False

    def nearest_plan(self, player_num, ball_x, ball_y):
        player_num_filter = []

        for plan in self.__final_dict:
            if len(plan["agentInitPos"]) >= player_num:
                player_num_filter.append(plan)

        active_list = self.get_master_active_plans(player_num_filter)

        sublist = sorted(active_list, key=lambda x: self.ball_dist(
            x, x["ballInitPos"]["x"], x["ballInitPos"]["y"], ball_x, ball_y))

        if len(sublist) > 0:
            print("# active and valid plans: " + str(len(sublist)) + "\n")

            print ("\n" + sublist[0]["filename"].split("plans/")[1] +
                   ": " + str(sublist[0]["index"]) + "   " + str(sublist[0]["planMode"]))
            return self.ai_message_generator(sublist[0])
        else:
            print ("There is No master or active plan with proper number of players :/")
            return None

    @staticmethod
    def circle_contains(x, y, r, point_x, point_y):
        if (x - point_x) * (x - point_x) + (y - point_y) * (y - point_y) > r * r:
            return False
        else:
            return True

    @staticmethod
    def ball_dist(plan, x1, y1, x2, y2):
        a = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)
        b = (x1 - x2) * (x1 - x2) + (y1 + y2) * (y1 + y2)
        if a < b:
            plan["symmetry"] = False
            return math.sqrt(a)
        else:
            plan["symmetry"] = True
            return math.sqrt(b)

    def get_all_plans_gui_msgs(self):
        self.refresh();
        plans_msg = []
        for plan in self.__final_dict:
            plans_msg.append(self.gui_message_generator(plan))
        return plans_msg

    def update_master_active(self, name_list, plan_index_list, is_master, is_active):
        for plan in self.__final_dict:
            for i in range(0, len(name_list)):
                if plan["filename"] == name_list[i] and plan["index"] == plan_index_list[i]:
                    plan["isMaster"] = is_master
                    plan["isActive"] = is_active
                    self.__all_active_plans[(plan["filename"], plan_index_list[i])] = is_active
                    self.__all_master_plans[(plan["filename"], plan_index_list[i])] = is_master

        for plan in self.__final_dict:
            print(plan["filename"] + ": " + "master: " + str(plan["isMaster"]) + " , " + "active: " + str(plan["isActive"]))
        return self.get_all_plans_gui_msgs()

    def plans_to_dict(self):
        self.__final_dict = []
        # print ("plans_to_dict")
        # print (len(self.__final_list))
        for plan in self.__final_list:
            with open(str(plan)) as json_data:
                tmp = json.load(json_data)
                for i in range(0, len(tmp["plans"])):
                    dict1 = tmp["plans"][i]
                    dict1.update({"index": i})
                    dict1.update({"filename": str(plan)})
                    dict1.update({"successRate": 0})
                    dict1.update({"planRepeat": 0})
                    dict1.update({"symmetry": False})

                    if (dict1["filename"], dict1["index"]) in self.__all_active_plans:
                        dict1.update({"isActive": self.__all_active_plans[dict1["filename"], dict1["index"]]})
                    else:
                        dict1.update({"isActive": True})

                    if (dict1["filename"], dict1["index"]) in self.__all_master_plans:
                        dict1.update({"isMaster": self.__all_master_plans[dict1["filename"], dict1["index"]]})
                    else:
                        dict1.update({"isMaster": False})

                    self.__final_dict.append(dict1)

        return self.__final_dict

    def gui_message_generator(self, plan_dict):
        plan_gui_msg = parsian_plan_GUI()
        plan_gui_msg.isActive = plan_dict["isActive"]
        plan_gui_msg.isMaster = plan_dict["isMaster"]
        plan_gui_msg.planFile = plan_dict["filename"]
        plan_gui_msg.agentSize = len(plan_dict["agentInitPos"])
        plan_gui_msg.chance = plan_dict["chance"]
        plan_gui_msg.lastDist = plan_dict["lastDist"]
        plan_gui_msg.tags = plan_dict["tags"]
        plan_gui_msg.planMode = plan_dict["planMode"]
        plan_gui_msg.ballInitPos.x = plan_dict["ballInitPos"]["x"]
        plan_gui_msg.ballInitPos.y = plan_dict["ballInitPos"]["y"]
        plan_gui_msg.planRepeat = plan_dict["planRepeat"]
        plan_gui_msg.successRate = plan_dict["successRate"]

        return plan_gui_msg

    def ai_message_generator(self, plan_dict):
        plan_msg = parsian_plan()
        plan_msg.isActive = plan_dict["isActive"]
        plan_msg.isMaster = plan_dict["isMaster"]
        plan_msg.planFile = plan_dict["filename"]
        plan_msg.agentSize = len(plan_dict["agentInitPos"])
        plan_msg.chance = plan_dict["chance"]
        plan_msg.lastDist = plan_dict["lastDist"]
        plan_msg.tags = plan_dict["tags"]
        plan_msg.planMode = plan_dict["planMode"]
        plan_msg.ballInitPos.x = plan_dict["ballInitPos"]["x"]
        plan_msg.ballInitPos.y = plan_dict["ballInitPos"]["y"]
        plan_msg.planRepeat = plan_dict["planRepeat"]
        plan_msg.successRate = plan_dict["successRate"]
        plan_msg.symmetry = plan_dict["symmetry"]

        i = 0
        j = 0
        k = 0
        for agent in plan_dict["agents"]:
            plan_msg.agents[i].id = agent["ID"]
            plan_msg.agents[i].posSize = len(agent["positions"])
            j = 0
            for pos in agent["positions"]:
                # print ("---------------------------------------------pos "+str(i))
                plan_msg.agents[i].positions[j].angel = pos["angel"]
                plan_msg.agents[i].positions[j].pos.x = pos["pos-x"]
                plan_msg.agents[i].positions[j].pos.y = pos["pos-y"]
                plan_msg.agents[i].positions[j].tolerance = pos["tolerance"]
                plan_msg.agents[i].positions[j].skillSize = len(pos["skills"])
                k = 0
                for skill in pos["skills"]:
                    plan_msg.agents[i].positions[j].skills[k].flag = skill["flag"]
                    plan_msg.agents[i].positions[j].skills[k].name = skill["name"]
                    plan_msg.agents[i].positions[j].skills[k].primary = skill["primary"]
                    plan_msg.agents[i].positions[j].skills[k].secondry = skill["secondary"]
                    if "target" in skill:
                        plan_msg.agents[i].positions[j].skills[k].agent = skill["target"]["agent"]
                        plan_msg.agents[i].positions[j].skills[k].index = skill["target"]["index"]
                    else:
                        plan_msg.agents[i].positions[j].skills[k].agent = -1
                        plan_msg.agents[i].positions[j].skills[k].index = -1
                    k += 1
                j += 1
            i += 1

        i = 0
        for pos in plan_dict["agentInitPos"]:
            plan_msg.agentInitPos[i].x = pos["x"]
            plan_msg.agentInitPos[i].y = pos["y"]
            i += 1

        return plan_msg


if __name__ == '__main__':
    # w = Watcher()
    # w.run()
    path = rospkg.RosPack().get_path("parsian_ai")
    print ("pack path: " + path)
    path += "/plans/"

    event_handler = Handler(path)
    event_handler.nearest_plan(2, 2, 2, 1)
    event_handler.get_all_plans_gui_msgs()
