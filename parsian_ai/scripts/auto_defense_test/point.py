import math


class Point:
    def __init__(self, point):
        self.x = point.x
        self.y = point.y

    def __init__(self, x, y):
        self.x = x
        self.y = y

    def distance(self, point):
        return math.hypot(self.y - point.y, self.x - point.x)

    def difX(self, point):
        return point.x - self.x

    def difY(self, point):
        return point.y - self.y

    def minus(self,p):
        return Point(self.x -p.x, self.y - p.y )

    def angle(self, point):
        return math.atan2(self.difY(point), self.difX(point))

    def length(self):
        return math.hypot(self.x, self.y)

    def unitPoint(self):
        tempX = self.x/self.length()
        tempY = self.y/self.length()
        return Point(tempX,tempY)
