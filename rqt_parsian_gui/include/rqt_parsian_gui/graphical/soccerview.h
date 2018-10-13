//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
\file    soccerview.h
\brief   C++ Interface: GLSoccerView
\author  Joydeep Biswas (C) 2011
*/
//========================================================================

#include <QMouseEvent>
#include <QWidget>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFormat>
#include <QMutex>
#include <QVector>
#include <QtOpenGL/QGL>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <cstdio>
#include "field_default_constants.h"
#include "timer.h"
#include "geometry.h"
#include "util.h"
#include "gltext.h"
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_draws.h>
#include <parsian_msgs/parsian_draw_buffer.h>
#include <parsian_msgs/parsian_team_config.h>

#ifndef SOCCERVIEW_H
#define SOCCERVIEW_H

using namespace std;

#define FIELD_COLOR_GREEN 0.0,0.5686,0.0980,1.0
#define FIELD_COLOR_GRAY  0.5,0.5,0.5,1.0
#define FIELD_LINES_COLOR 1.0,1.0,1.0,1.0


namespace rqt_parsian_gui {

    class GLSoccerView : public QGLWidget {
    Q_OBJECT

    public:
        struct FieldDimensions {
            vector<FieldLine *> lines;
            vector<FieldCircularArc *> arcs;
            vector<FieldTriangle *> tris;
            double field_length;
            double field_width;
            double boundary_width;

            FieldDimensions();
        };

        struct Robot {
            bool hasAngle;
            vector2d loc;
            double angle;
            int id;
            double conf;
            int team;
            int cameraID;
        };

        typedef enum {
            teamUnknown = 0,
            teamBlue,
            teamYellow
        } TeamTypes;

    private:
        static const double minZValue;
        static const double maxZValue;
        static const double FieldZ;
        static const double RobotZ;
        static const double BallZ;
        static const double DebugZ;
        static const int PreferedWidth;
        static const int PreferedHeight;
        static const uint64_t MinRedrawInterval; ///Minimum time between graphics updates (limits the fps)
        static const int unknownRobotID;

        QVector<Robot> robots;
        vector2d ball;
        GLText glText;

        GLuint blueRobotShape;
        GLuint yellowRobotShape;
        GLuint greyRobotShape;
        GLuint blueCircleRobotShape;
        GLuint yellowCircleRobotShape;
        GLuint greyCircleRobotShape;

        double viewScale; /// Ratio of world space to screen space coordinates
        double viewXOffset;
        double viewYOffset;

        bool leftButton;
        bool midButton;
        bool rightButton;
        int mouseStartX;
        int mouseStartY;

        uint64_t tLastRedraw;

        parsian_msgs::parsian_drawsPtr debugs;
        parsian_msgs::parsian_drawsPtr debugs2;
        FieldDimensions fieldDim;

        bool grayColor;

    private:
        void drawFieldLines(FieldDimensions &dimensions);

        void drawRobots();

        void drawDebugs();

        void drawQuad(vector2d loc1, vector2d loc2, double z = 0.0, bool filled = true);

        void drawQuad(double x1, double y1, double x2, double y2, double z = 0.0, bool filled = true) {
            drawQuad(vector2d(x1, y1), vector2d(x2, y2), z, filled);
        }

        void drawArc(vector2d loc, double r1, double r2, double theta1,
                     double theta2, double z = 0.0, double dTheta = -1);

        void drawArc(double x, double y, double r1, double r2, double theta1, double theta2, double z = 0.0,
                     double dTheta = -1) { drawArc(vector2d(x, y), r1, r2, theta1, theta2, z, dTheta); }

        void drawVectors(const double& x, const double& y, const double& size, const std_msgs::ColorRGBA& color);

        void recomputeProjection();

        void drawRobot(vector2d loc, double theta, double conf, int robotID, int team, bool hasAngle);

        void drawRobot(int team, bool hasAngle, bool useDisplayLists);

        void drawBall(vector2d loc);

        QColor toQColor(const std_msgs::ColorRGBA& _color);

    protected:
        void paintEvent(QPaintEvent *event);

        void wheelEvent(QWheelEvent *event);

        void mouseMoveEvent(QMouseEvent *event);

        void mousePressEvent(QMouseEvent *event);

        void mouseReleaseEvent(QMouseEvent *event);

        void keyPressEvent(QKeyEvent *event);

        void resizeEvent(QResizeEvent *event);

        void initializeGL();

        void resizeGL(int width, int height);

        QSize sizeHint() const { return QSize(PreferedWidth, PreferedHeight); }

    public:
        GLSoccerView(QWidget *parent = 0);

        void updateDB(const parsian_msgs::parsian_draw_bufferConstPtr &_packet);

        void toggleColor();
        void updateConfig(const parsian_msgs::parsian_team_configConstPtr &_config);

    public slots:

        void resetView();
        void redraw();

    signals:

        void postRedraw();
    };
}
#endif
