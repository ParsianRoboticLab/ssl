//
// Created by rebinnaf on 10/19/17.
//
#ifndef RQT_PARSIAN_GUI_MONITORWIDGET_H
#define RQT_PARSIAN_GUI_MONITORWIDGET_H

#include "rqt_parsian_gui/guiDrawer.h"
#include <ros/ros.h>
#include <QWidget>
#include <QTimer>
#include <QString>
#include <qcolor.h>
#include <QMouseEvent>
#include <GL/gl.h>
#include <qopenglwidget.h>
#include <QGridLayout>
#include <QtOpenGL/qgl.h>
#include <GL/glu.h>
#include <parsian_util/core/field.h>
#include <parsian_msgs/vector2D.h>



#define _PI         3.14159265358979323
#define _DEG2RAD    (_PI/180.0)
#define _GOAL_WIDTH             1.200
#define _FIELD_WIDTH            12.0
#define _FIELD_HEIGHT           9.00
#define _CENTER_CIRCLE_RAD      0.600
#define _PENALTY_WIDTH          2.400
#define _GOAL_RAD               1.200
#define _STADIUM_WIDTH   13.80
#define _STADIUM_HEIGHT  10.40



namespace rqt_parsian_gui
{
    class MonitorWidget
            :public QOpenGLWidget {
    Q_OBJECT
    public:
        MonitorWidget();
        ~MonitorWidget();
        void drawField();
        CguiDrawer *drawerBuffer;
        ros::NodeHandle n;
        ros::Publisher monitor_pub;
        parsian_msgs::vector2DPtr mousePos;
        //TODO: get these values from util CRobot
        const double robot_radius_new = 0.0890;
        const double robot_radius_old = 0.0900;
        int getViewportWidth();
        void showLogMode(bool isLogMode,bool isReplayMode);
        QTimer *recShowTimer;
        bool recShowBool;


    public slots:
        void showHideRec();


    protected:

        void initializeGL();
        void paintGL();

        double cameraX,cameraY;
        double scaleFactor;
        void resizeGL(int width, int height);
        QPainter painter;
        void mousePressEvent(QMouseEvent *event);


        void wheelEvent(QWheelEvent *event);
    private:
        bool isLogging,isReplaying;
        QSizeF viewportSize;
        QSizeF stadiumSize;
        QRectF field;
        QRectF fieldCenter;
        QRectF leftPenalty;
        QRectF rightPenalty;
        GLuint list;
        QColor stadiumGreen;
        QColor fieldGreen;
        double viewportWidth;
        double WH_RATIO;
        double coeff;


        Vector2D centralPoint;
        void qglClearColor(QColor clearColor);

        void setViewportWidth(int width);

        GLuint drawArc(double centerX, double centerY, double radius, int start, int end, QColor color = QColor(255, 255, 255),
                       bool fill = false, bool fullFill = false);

        GLuint drawRect(double topLeftX, double topLeftY, double buttomRightX, double buttomRightY,
                        QColor color = QColor(255, 255, 255), bool fill = false);

        GLuint drawLine(double x1, double y1, double x2, double y2, QColor color = QColor(255, 255, 255));

        GLuint drawPoint(double x, double y, QColor color = QColor(0, 0, 0));


        void drawText(double x, double y, QString text, QColor color, int size);
        void drawRobot(double x, double y, double ang, int ID, int comID, QColor color,QString str="", bool newRobots=false);
    };




}  // namespace rqt_example_cpp

#endif //RQT_PARSIAN_GUI_MONITORWIDGET_H
