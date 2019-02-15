import math
import point

EPSILON = 1.0e-5

class Line:
    def __init__(self, p1,p2):
        self.M_a = -(p2.y - p1.y)
        self.M_b = p2.x - p1.x
        self.M_c = -1*self.M_a * p1.x - self.M_b * p1.y
    def intersection(self, L2):
        tmp = self.M_a * L2.M_b - self.M_b * L2.M_a
        print(tmp)
        print('ma :' , self.M_a , 'mb :',L2.M_b)
        if (abs(tmp) < EPSILON):
            return point.Point(-5000,-5000)
        return point.Point((self.M_b*L2.M_c - L2.M_b*self.M_c)/tmp ,(self.M_c*L2.M_a - L2.M_c*self.M_a)/tmp )


class Seg:
    def __init__(self, p1, p2):
        self.p1 = p1
        self.p2 = p2
    def contains(self, p):
        return ((p.x - self.p1.x) * (p.x - self.p2.x) <= 1.0e-5 and (p.y - self.p1.y) * (p.y - self.p2.y) <= 1.0e-5)

    def intersection(self, seg2):
        my_line = Line(self.p1,self.p2)
        other_line = Line(seg2.p1,seg2.p2)
        tmp_sol = my_line.intersection(other_line)
        print(tmp_sol.x, tmp_sol.y)

        if tmp_sol.x == -5000 and tmp_sol.y == -5000:
            print("inja ride")
            return point.Point(-5000,-5000)
        if not(self.contains(tmp_sol)) or not(seg2.contains(tmp_sol)):
            return point.Point(-5000, -5000)
        return tmp_sol

