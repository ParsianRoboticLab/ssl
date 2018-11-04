#include <parsian_ai/util/defpos.h>

double CDefPos::penaltyAreaOffset = 0.2;

CDefPos::CDefPos() {
    penaltyAreaRadius = 1.33;
    penaltyAreaCircle.assign(wm->field->ourGoal() - Vector2D(penaltyAreaOffset, 0.001), penaltyAreaRadius);
    oneDefThr = 0;
}

Vector2D CDefPos::getXYByAngle(double _angle, double _radius) {
    double tempX;
    double tempY;
    _angle += M_PI_2;
    tempX = _radius * cos(M_PI - _angle);
    tempY = _radius * sin(_angle);
    tempX += wm->field->ourGoal().x - penaltyAreaOffset;
    if (tempX < wm->field->ourGoal().x + 0.08) {
        tempX = wm->field->ourGoal().x + 0.08;
    }
    return Vector2D{tempX, tempY};
}

double CDefPos::getRobotAngle(double _radius) {
    return atan(Robot::robot_radius_old / _radius) * 2;
}

double CDefPos::getAngleByXY(Vector2D _point) {
    double w = wm->field->ourGoal().y - _point.y;
    double h = _point.x - (wm->field->ourGoal().x - penaltyAreaOffset);
    return _PI - atan2(h, w);
}

Intersection CDefPos::getIntersections(Vector2D _ballPos, double _radius, bool isNearPenaltyArea) {
    Intersection tempAngles{};
    Vector2D inter1, inter2, inter3;
    Circle2D tempCircle(wm->field->ourGoal() - Vector2D(penaltyAreaOffset, 0), _radius);
    if (tempCircle.contains(_ballPos)) {
        Segment2D tempSegment(wm->field->ourGoal(),
                              (_ballPos - wm->field->ourGoal()).norm() * 3 + (wm->field->ourGoal() - Vector2D(penaltyAreaOffset, 0)));
        tempCircle.intersection(tempSegment, &inter3, &inter1);
        _ballPos = inter3;
    }
    Segment2D tempSeg1(wm->field->ourGoalL(), _ballPos);
    Segment2D tempSeg2(wm->field->ourGoalR(), _ballPos);
    if (isNearPenaltyArea) {
        inter1 = wm->field->ourPAreaIntersect(tempSeg1)[0];
        inter2 = wm->field->ourPAreaIntersect(tempSeg2)[0];
        tempAngles.radius[0] = (wm->field->ourGoal() - Vector2D(penaltyAreaOffset, 0)).dist(inter1);
        tempAngles.radius[1] = (wm->field->ourGoal() - Vector2D(penaltyAreaOffset, 0)).dist(inter2);
    } else {
        tempCircle.intersection(tempSeg1, &inter1, &inter3);
        tempCircle.intersection(tempSeg2, &inter2, &inter3);
    }


    tempAngles.angle[0] = getAngleByXY(inter1);
    tempAngles.angle[1] = getAngleByXY(inter2);
    if (tempAngles.angle[0] > tempAngles.angle[1]) {
        double tempSwap;
        tempSwap = tempAngles.angle[1];
        tempAngles.angle[0] = tempAngles.angle[1];
        tempAngles.angle[1] = tempSwap;
    }
    return tempAngles;
}

DefPos CDefPos::getDefPositions(const Vector2D& _ballPos, int _size, double _limit1, double _limit2) {
    DefPos tempDefPos;
    tempDefPos.size = _size;
    if (_size <= 0) {
        return tempDefPos;
    }
    //double ballDistLimit = _ballPos.dist(wm->field->ourGoal())/2;
    double tempBestRadius = _ballPos.dist(wm->field->ourGoal()) / 2;
    if (findBestRadius(tempDefPos.size) != -1) {
        tempBestRadius = findBestRadius(tempDefPos.size);
    }

    bool isNearPenaltyArea{false};
    if (tempBestRadius > _limit2) {
        ///////////// Added By AHZ for One Defense better //////////////////////
        if (_size == 2) {
            tempBestRadius = _limit2;
        } else if (_size == 1) {
            if (tempBestRadius > 3) {
                tempBestRadius = 3;
            }
        }
        ///////////////////////////////////////////////////////////////////////
        isNearPenaltyArea = false;
    } else if (tempBestRadius < _limit1) {
        tempBestRadius = _limit1;
        isNearPenaltyArea = true;
    } else {
        isNearPenaltyArea = false;
    }

    Intersection tempAngles = getIntersections(_ballPos, tempBestRadius, isNearPenaltyArea);
    double tempOpenAngle = std::fabs(tempAngles.angle[1] - tempAngles.angle[0]);
    double agentAngle = getRobotAngle(tempBestRadius);
    double openAngleAfterPositioning = tempOpenAngle - agentAngle * _size;

    if (openAngleAfterPositioning > 0) {
        if (_size <= 1) {
            if (isNearPenaltyArea) {
                tempBestRadius = tempAngles.radius[0];
            }
            if (_ballPos.y + wm->ball->vel.y < 0 + oneDefThr) {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle[0] + agentAngle / 2, tempBestRadius);
                oneDefThr = 0.2;
            } else {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle[1] - agentAngle / 2, tempBestRadius);
                oneDefThr = -0.2;
            }
        } else if (_size == 2 && isNearPenaltyArea) {
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempBestRadius = tempAngles.radius[i];
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        } else if (_size == 3) {
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempBestRadius = tempAngles.radius[i];
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
            Segment2D robotConnectSegment = Segment2D(tempDefPos.pos[0], tempDefPos.pos[1]);
            Line2D robotConnectBisector = robotConnectSegment.perpendicularBisector();
            Vector2D middle = (tempDefPos.pos[0] + tempDefPos.pos[1]) / 2;
            double biggerX = (tempDefPos.pos[0].x > tempDefPos.pos[1].x) ? tempDefPos.pos[0].x : tempDefPos.pos[1].x;
            biggerX += 0.5;
            double biggerY = robotConnectBisector.getY(biggerX);
            tempDefPos.pos[2] = Vector2D(biggerX, biggerY);

        } else {
            oneDefThr = 0;
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    } else {
        oneDefThr = 0;
        if (_size <= 1) {
            tempDefPos.pos[0] = getXYByAngle((tempAngles.angle[0] + tempAngles.angle[1]) / 2, tempBestRadius);
        } else {
            double angleOffset = _PI / 720;
            double defAngle = (tempAngles.angle[0] + openAngleAfterPositioning / 2) + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    }
    return tempDefPos;
}

double CDefPos::findBestRadius(int _numOfDefs) {
    Vector2D BallPos = wm->ball->pos;
    Vector2D ourGoalL = wm->field->ourGoalL();
    Vector2D ourGoalR = wm->field->ourGoalR();
    Segment2D leftBarSeg(BallPos , ourGoalL);
    Segment2D rightBarSeg(BallPos , ourGoalR);
    Segment2D goalSeg(ourGoalL, ourGoalR);
    Segment2D downFieldLine(Vector2D(-wm->field->_FIELD_WIDTH / 2, -wm->field->_FIELD_HEIGHT / 2), Vector2D(-wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2));
    double ballheight = BallPos.dist(downFieldLine.nearestPoint(BallPos));
    double vatarKazeb;
    Segment2D biggerBarSeg = (leftBarSeg.length() < rightBarSeg.length()) ? rightBarSeg : leftBarSeg;
    Segment2D shorterBarSeg = (leftBarSeg.length() > rightBarSeg.length()) ? rightBarSeg : leftBarSeg;
    vatarKazeb = goalSeg.intersection(shorterBarSeg).dist(biggerBarSeg.nearestPoint(goalSeg.intersection(shorterBarSeg)));
    Line2D mainLine(goalSeg.intersection(shorterBarSeg), biggerBarSeg.nearestPoint(goalSeg.intersection(shorterBarSeg)));
    if (vatarKazeb > (_numOfDefs) * 2 * Robot::robot_radius_new) {
        mainLine = Line2D(Vector2D(BallPos.x - ((_numOfDefs + 0.3) * 1.5 * Robot::robot_radius_new * ballheight / vatarKazeb), BallPos.y), Vector2D(BallPos.x - ((_numOfDefs + 0.3) * 1.5 * Robot::robot_radius_new * ballheight / vatarKazeb), BallPos.y - 0.1));
    }
    if (biggerBarSeg.intersection(mainLine).valid()) {
        return biggerBarSeg.intersection(mainLine).dist(wm->field->ourGoal()) + penaltyAreaOffset;
    }
    return -1;
}

DefPos CDefPos::getStaticDefPositions(const Vector2D &_ballPos, int _size, double _limit1, double _limit2) {
    DefPos tempDefPos;
    tempDefPos.size = _size;
    if (_size <= 0) {
        return tempDefPos;
    }
    //double ballDistLimit = _ballPos.dist(wm->field->ourGoal())/2;
    double tempBestRadius = _ballPos.dist(wm->field->ourGoal()) / 2;
    if (findBestRadius(tempDefPos.size) != -1) {
        tempBestRadius = findBestRadius(tempDefPos.size);
    }

    bool isNearPenaltyArea{false};
    if (tempBestRadius > _limit2) {
        ///////////// Added By AHZ for One Defense better //////////////////////
        if (_size == 2) {
            tempBestRadius = _limit2;
        } else if (_size == 1) {
            if (tempBestRadius > 3) {
                tempBestRadius = 3;
            }
        }
        ///////////////////////////////////////////////////////////////////////
        isNearPenaltyArea = false;
    } else if (tempBestRadius < _limit1) {
        tempBestRadius = _limit1;
        isNearPenaltyArea = true;
    } else {
        isNearPenaltyArea = false;
    }

    Intersection tempAngles = getIntersections(_ballPos, tempBestRadius, isNearPenaltyArea);
    double tempOpenAngle = std::fabs(tempAngles.angle[1] - tempAngles.angle[0]);
    double agentAngle = getRobotAngle(tempBestRadius);
    double openAngleAfterPositioning = tempOpenAngle - agentAngle * _size;

    if (openAngleAfterPositioning > 0) {
        if (_size <= 1) {
            if (isNearPenaltyArea) {
                tempBestRadius = tempAngles.radius[0];
            }
            if (_ballPos.y + wm->ball->vel.y < 0) {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle[0] + agentAngle / 2, tempBestRadius);
            } else {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle[1] - agentAngle / 2, tempBestRadius);
            }
        } else if (_size == 2 && isNearPenaltyArea) {
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempBestRadius = tempAngles.radius[i];
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        } else if (_size == 3) {
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempBestRadius = tempAngles.radius[i];
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
            Segment2D robotConnectSegment = Segment2D(tempDefPos.pos[0], tempDefPos.pos[1]);
            Line2D robotConnectBisector = robotConnectSegment.perpendicularBisector();
            Vector2D middle = (tempDefPos.pos[0] + tempDefPos.pos[1]) / 2;
            double biggerX = (tempDefPos.pos[0].x > tempDefPos.pos[1].x) ? tempDefPos.pos[0].x : tempDefPos.pos[1].x;
            biggerX += 0.5;
            double biggerY = robotConnectBisector.getY(biggerX);
            tempDefPos.pos[2] = Vector2D(biggerX, biggerY);

        } else {
            double angleOffset = openAngleAfterPositioning / (_size - 1);
            double defAngle = tempAngles.angle[0] + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    } else {
        if (_size <= 1) {
            tempDefPos.pos[0] = getXYByAngle((tempAngles.angle[0] + tempAngles.angle[1]) / 2, tempBestRadius);
        } else {
            double angleOffset = _PI / 720;
            double defAngle = (tempAngles.angle[0] + openAngleAfterPositioning / 2) + agentAngle / 2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    }
    return tempDefPos;
}
