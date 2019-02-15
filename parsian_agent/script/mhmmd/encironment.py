

class ENV:
    def __init__(self):
        self.observation_space = list(range(0, 3))
        self.action_space = list(range(-1200, 1201, 3))
        self.state = 0
        self.done = False
        self.reward = 0
        self.canStep = False

    def reset(self):
        self.done = False
        self.state = 0
        self.reward = 0
        return self.state

    def step(self, s):
        self.state = s+1
        if self.state > 1:
            self.done = True
        self.reward = calc_reward()
        return self.state, self.reward, self.done
    def checkStep(self, fwd):
        self.canStep = False
        self.canStep = fwd

def calc_reward():
    return 0
