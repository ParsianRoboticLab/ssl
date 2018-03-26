import math


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def distance(self, point):
        return math.hypot(self.y - point.y, self.x - point.x)

    def difX(self, point):
        return point.x - self.x

    def difY(self, point):
        return point.y - self.y

    def angle(self, point):
        return math.atan2(self.difY(point), self.difX(point))
