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
\file    soccerview.cpp
\brief   C++ Implementation: GLSoccerView
\author  Joydeep Biswas (C) 2011
*/
//========================================================================

#include <rqt_parsian_gui/graphical/soccerview.h>

#include <rqt_parsian_gui/graphical/field.h>
#include <rqt_parsian_gui/graphical/field_default_constants.h>
#include <ros/ros.h>
#include <parsian_msgs/grsim_ball_replacement.h>

using namespace rqt_parsian_gui;

using FieldConstants::kNumFieldLines;
using FieldConstants::kNumFieldArcs;
using FieldConstants::kFieldLines;
using FieldConstants::kFieldArcs;

const double GLSoccerView::minZValue = -10;
const double GLSoccerView::maxZValue = 10;
const double GLSoccerView::FieldZ = 1.0;
const double GLSoccerView::RobotZ = 2.0;
const double GLSoccerView::BallZ = 3.0;
const double GLSoccerView::DebugZ = 4.0;
const int GLSoccerView::PreferedWidth = 1024;
const int GLSoccerView::PreferedHeight = 768;
const uint64_t GLSoccerView::MinRedrawInterval = 20000000; ///Minimum time between graphics updates (limits the fps)
const int GLSoccerView::unknownRobotID = -1;

GLSoccerView::FieldDimensions::FieldDimensions() :
    field_length(FieldConstants::kFieldLength),
    field_width(FieldConstants::kFieldWidth),
    boundary_width(FieldConstants::kBoundaryWidth) {
    for (size_t i = 0; i < kNumFieldLines; ++i) {
        lines.push_back(new FieldLine(kFieldLines[i]));
    }
    for (size_t i = 0; i < kNumFieldArcs; ++i) {
        arcs.push_back(new FieldCircularArc(kFieldArcs[i]));
    }

}

GLSoccerView::GLSoccerView(QWidget* parent) :
    QGLWidget(QGLFormat(
                  QGL::DoubleBuffer | QGL::DepthBuffer | QGL::SampleBuffers),parent) {
    viewScale =
            (fieldDim.field_length + fieldDim.boundary_width) / sizeHint().width();
    viewScale = max(viewScale,
                    (fieldDim.field_width + fieldDim.boundary_width) /
                    sizeHint().height());

    viewXOffset = viewYOffset = 0.0;
    setAutoFillBackground(false); //Do not let painter auto fill the widget's background: we'll do it manually through openGl
    connect(this, SIGNAL(postRedraw()), this, SLOT(redraw()));
    blueRobotShape = GL_INVALID_VALUE;
    yellowRobotShape = GL_INVALID_VALUE;
    greyRobotShape = GL_INVALID_VALUE;
    blueCircleRobotShape = GL_INVALID_VALUE;
    yellowCircleRobotShape = GL_INVALID_VALUE;
    greyCircleRobotShape = GL_INVALID_VALUE;
    QFont RobotIDFont = this->font();
    RobotIDFont.setWeight(QFont::Bold);
    RobotIDFont.setPointSize(80);
    glText = GLText(RobotIDFont);
    tLastRedraw = 0;
    debugs.reset(new parsian_msgs::parsian_draws());
    debugs2.reset(new parsian_msgs::parsian_draws());
    grayColor = false;
}

void GLSoccerView::redraw()
{
//    if(ros::Time::now().toNSec() - tLastRedraw < MinRedrawInterval)
//        return;
    update();
//    tLastRedraw = ros::Time::now().toNSec();
}


void GLSoccerView::mousePressEvent(QMouseEvent* event)
{
    leftButton = event->buttons().testFlag(Qt::LeftButton);
    midButton = event->buttons().testFlag(Qt::MidButton);
    rightButton = event->buttons().testFlag(Qt::RightButton);

    QPointF mp = mouseToFieldPos(event->pos());

    if(leftButton)
        setCursor(Qt::ClosedHandCursor);
    if(midButton) {
        setCursor(Qt::SizeVerCursor);
    }

    if (rightButton) {
        parsian_msgs::grsim_ball_replacementRequest req;
        parsian_msgs::grsim_ball_replacementResponse rep;
        req.vx = req.vy = 0;
        req.x = mp.x(); req.y = mp.y();
        ballClinet->call(req, rep);
    }

    if(leftButton || midButton){
        // Start Pan / Zoom
        mouseStartX = event->x();
        mouseStartY = event->y();
        m_mousepos = event->pos();
        postRedraw();
    }
    if (!ball.empty()) {
        if (distVec(event->x(), event->y(), ball.last().x, ball.last().y) < 21) {
            choosen.id = -1;
            choosen.team = teamUnknown;
        }
    }

    for (const auto& r : robots) {
        if (distVec(event->x(), event->y(), r.loc.x, r.loc.y) < 90) {
            choosen.team = r.team;
            choosen.id = r.id;
        }
    }
}

void GLSoccerView::mouseReleaseEvent(QMouseEvent* event)
{
    setCursor(Qt::ArrowCursor);
}

void GLSoccerView::mouseMoveEvent(QMouseEvent* event)
{
    static const bool debug = false;
    bool leftButton = event->buttons().testFlag(Qt::LeftButton);
    bool midButton = event->buttons().testFlag(Qt::MidButton);
    bool rightButton = event->buttons().testFlag(Qt::RightButton);

    if(leftButton){
        //Pan
        viewXOffset -= viewScale*double(event->x() - mouseStartX);
        viewYOffset += viewScale*double(event->y() - mouseStartY);
        mouseStartX = event->x();
        mouseStartY = event->y();
        recomputeProjection();
        postRedraw();
    }else if(midButton){
        //Zoom
        double zoomRatio = double(event->y() - mouseStartY)/500.0;
        viewScale = viewScale*(1.0+zoomRatio);
        recomputeProjection();
        mouseStartX = event->x();
        mouseStartY = event->y();
        postRedraw();
    }
}

void GLSoccerView::wheelEvent(QWheelEvent* event)
{
    double zoomRatio = -double(event->delta())/1000.0;
    viewScale *= (1.0+zoomRatio);
    recomputeProjection();
    postRedraw();
}

void GLSoccerView::keyPressEvent(QKeyEvent* event)
{
    ROS_INFO_STREAM("KeyPress: " << event->key());
    if(event->key() == Qt::Key_Space)
        resetView();
    if(event->key() == Qt::Key_Escape)
        close();
}

void GLSoccerView::resetView()
{
    viewScale =
            (fieldDim.field_length + fieldDim.boundary_width) / width();
    viewScale = max(viewScale,
                    (fieldDim.field_width + fieldDim.boundary_width) / height());

    viewXOffset = viewYOffset = 0.0;
    recomputeProjection();
    postRedraw();
}

void GLSoccerView::resizeEvent(QResizeEvent* event)
{
    QGLWidget::resizeEvent(event);
    redraw();
}

void GLSoccerView::recomputeProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5*viewScale*width()+viewXOffset, 0.5*viewScale*width()+viewXOffset, -0.5*viewScale*height()+viewYOffset, 0.5*viewScale*height()+viewYOffset, minZValue, maxZValue);
    glMatrixMode(GL_MODELVIEW);
}

void GLSoccerView::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    recomputeProjection();
}

void GLSoccerView::initializeGL()
{
    blueRobotShape = glGenLists(1);
    if(blueRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(blueRobotShape, GL_COMPILE);
    drawRobot(teamBlue,true,false);
    glEndList();

    yellowRobotShape = glGenLists(1);
    if(yellowRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(yellowRobotShape, GL_COMPILE);
    drawRobot(teamYellow,true,false);
    glEndList();

    greyRobotShape = glGenLists(1);
    if(greyRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(greyRobotShape, GL_COMPILE);
    drawRobot(teamUnknown,true,false);
    glEndList();

    blueCircleRobotShape = glGenLists(1);
    if(blueRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(blueCircleRobotShape, GL_COMPILE);
    drawRobot(teamBlue,false,false);
    glEndList();

    yellowCircleRobotShape = glGenLists(1);
    if(yellowRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(yellowCircleRobotShape, GL_COMPILE);
    drawRobot(teamYellow,false,false);
    glEndList();

    greyCircleRobotShape = glGenLists(1);
    if(greyRobotShape==GL_INVALID_VALUE){
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(greyCircleRobotShape, GL_COMPILE);
    drawRobot(teamUnknown,false,false);
    glEndList();
}

void GLSoccerView::paintEvent(QPaintEvent* event)
{


    makeCurrent();
    if (grayColor) {
        glClearColor(FIELD_COLOR_GRAY);
    } else {
        glClearColor(FIELD_COLOR_GREEN);
    }
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    drawFieldLines(fieldDim);
    drawDebugs();
    drawRobots();
    drawBall(ball);
    glPopMatrix();
    swapBuffers();
}

void GLSoccerView::drawQuad(vector2d loc1, vector2d loc2, double z, bool filled)
{
    glBegin((filled) ? GL_QUADS : GL_LINE_LOOP);
    glVertex3d(loc1.x,loc1.y,z);
    glVertex3d(loc2.x,loc1.y,z);
    glVertex3d(loc2.x,loc2.y,z);
    glVertex3d(loc1.x,loc2.y,z);
    glEnd();
}

void GLSoccerView::drawArc(vector2d loc, double r1, double r2, double theta1, double theta2, double z, double dTheta)
{
    static const double tesselation = 1.0;
    if(dTheta<0){
        dTheta = tesselation/r2;
    }
    glBegin(GL_QUAD_STRIP);
    for(double theta=theta1; theta<theta2; theta+=dTheta){
        double c1 = cos(theta), s1 = sin(theta);
        glVertex3d(r2*c1+loc.x,r2*s1+loc.y,z);
        glVertex3d(r1*c1+loc.x,r1*s1+loc.y,z);
    }
    double c1 = cos(theta2), s1 = sin(theta2);
    glVertex3d(r2*c1+loc.x,r2*s1+loc.y,z);
    glVertex3d(r1*c1+loc.x,r1*s1+loc.y,z);
    glEnd();
}

void GLSoccerView::drawRobot(int team, bool hasAngle, bool useDisplayLists)
{
    if(useDisplayLists){
        switch ( team ){
        case teamBlue:{
            if(hasAngle)
                glCallList(blueRobotShape);
            else
                glCallList(blueCircleRobotShape);
            break;
        }
        case teamYellow:{
            if(hasAngle)
                glCallList(yellowRobotShape);
            else
                glCallList(yellowCircleRobotShape);
            break;
        }
        default:{
            if(hasAngle)
                glCallList(greyRobotShape);
            else
                glCallList(greyCircleRobotShape);
            break;
        }
        }
        return;
    }
    switch ( team ){
        case teamBlue:{
            glColor3d(0.2549, 0.4941, 1.0);
            break;
        }
        case teamYellow:{
            glColor3d(1.0, 0.9529, 0.2431);
            break;
        }
        default:{
            glColor3d(0.5882,0.5882,0.5882);
            break;
        }
    }
    double theta1 = hasAngle?RAD(40):0.0;
    double theta2 = 2.0*M_PI - theta1;
    drawArc(0,0,0,90,theta1, theta2, RobotZ);
    glBegin(GL_TRIANGLES);
    glVertex3d(0,0,RobotZ);
    glVertex3d(90.0*cos(theta1),90.0*sin(theta1),RobotZ);
    glVertex3d(90.0*cos(theta2),90.0*sin(theta2),RobotZ);
    glEnd();

    switch ( team ){
        case teamBlue:{
            glColor3d(0.0706, 0.2314, 0.6275);
            break;
        }
        case teamYellow:{
            glColor3d(0.8, 0.6157, 0.0);
            break;
        }
        default:{
            glColor3d(0.2745,0.2745,0.2745);
            break;
        }
    }
    drawArc(0,0,80,90,theta1, theta2, RobotZ+0.01);
    drawQuad(90.0*cos(theta1)-10,90.0*sin(theta1), 90.0*cos(theta2),90.0*sin(theta2),RobotZ+0.01);

}

void GLSoccerView::drawRobot(vector2d loc, double theta, double conf, int robotID, int team, bool hasAngle)
{
    glPushMatrix();
    glLoadIdentity();
    glTranslated(loc.x,loc.y,0);
    glColor3d(0.0,0.0,0.0);
    char buf[1024];
    if(robotID!=unknownRobotID)
        snprintf(buf,1023,"%X",robotID);
    else
        snprintf(buf,1023,"?");
    glText.drawString(loc,0,100,buf,GLText::CenterAligned,GLText::MiddleAligned);
    glRotated(theta,0,0,1.0);
    drawRobot(team, hasAngle, true);
    glPopMatrix();
}

void GLSoccerView::drawFieldLines(FieldDimensions& dimensions)
{
    glColor4f(FIELD_LINES_COLOR);
    for (auto &i : fieldDim.lines) {
        const FieldLine& line = *i;
        const double half_thickness = 0.5 * line.thickness;
        const vector2d p1(line.p1_x, line.p1_y);
        const vector2d p2(line.p2_x, line.p2_y);
        const vector2d perp = (p2 - p1).norm().perp();
        const vector2d corner1 = p1 - half_thickness * perp;
        const vector2d corner2 = p2 + half_thickness * perp;
        drawQuad(corner1, corner2, FieldZ);
    }

    for (auto &i : fieldDim.arcs) {
        const FieldCircularArc& arc = *i;
        const double half_thickness = 0.5 * arc.thickness;
        const double radius = arc.radius;
        const vector2d center(arc.center_x, arc.center_y);
        const double a1 = arc.a1;
        const double a2 = arc.a2;
        drawArc(center, radius - half_thickness, radius + half_thickness, a1, a2,
                FieldZ);
    }

}

void GLSoccerView::drawBall(QVector<vector2d> loc) {

    for (const auto& l : loc) {
        glColor3d(1.0,0.5059,0.0);
        drawArc(l,15,21,-M_PI,M_PI,BallZ);

    }
    glColor3d(0.8706,0.3490,0.0);
    if (!loc.empty()) drawArc(loc.last(),0,16,-M_PI,M_PI,BallZ);

}

void GLSoccerView::drawRobots() {
    for (auto r : robots) {
        drawRobot(r.loc,r.angle,r.conf,r.id,r.team,r.hasAngle);
    }
}

void GLSoccerView::drawDebugs() {

    for(const auto& d : debugs2->draws) {
        switch (d.type) {
            case parsian_msgs::parsian_draw::CIRCLE:
                glColor4d(d.color.r, d.color.g, d.color.b, d.color.a);
                if (d.filled) {
                    drawArc(vector2d(d.primary.x*1000, d.primary.y*1000), 0, d.size*1000, d.secondary.x, d.secondary.y, DebugZ);
                } else {
                    glBegin(GL_LINE_LOOP);
                    glVertex3d(d.primary.x*1000, d.primary.y*1000, DebugZ);
                    for (double i = d.secondary.x; i <= d.secondary.y; i += M_PI/50)
                        glVertex3d(d.primary.x*1000 + (d.size*1000 * cos(i)),
                                   d.primary.y*1000 + (d.size*1000 * sin(i)), DebugZ);
                    glEnd();

                }
                break;
            case parsian_msgs::parsian_draw::VECTOR:
                drawVectors(d.primary.x*1000, d.primary.y*1000, d.size*1000, d.color);
                break;
            case parsian_msgs::parsian_draw::TEXT:
                glText.drawString(d.primary.x*1000, d.primary.y*1000, 0, d.size*10, d.text.c_str(), toQColor(d.color));
                break;
            case parsian_msgs::parsian_draw::SEGMENT:
                glColor4d(d.color.r, d.color.g, d.color.b, d.color.a);
                glBegin(GL_LINES);
                glVertex3d(d.primary.x*1000, d.primary.y*1000, DebugZ);
                glVertex3d(d.secondary.x*1000, d.secondary.y*1000, DebugZ);
                glEnd();
                break;
            case parsian_msgs::parsian_draw::RECT:
                glColor4d(d.color.r, d.color.g, d.color.b, d.color.a);
                drawQuad(vector2d(d.primary.x*1000, d.primary.y*1000),
                         vector2d(d.secondary.x*1000, d.secondary.y*1000), DebugZ,
                         d.filled);
                break;
            case parsian_msgs::parsian_draw::POLYGON:
                glBegin((d.filled) ? GL_POLYGON : GL_LINE_LOOP);
                glColor4d(d.color.r, d.color.g, d.color.b, d.color.a);
                for(const auto& pp : d.polygon) glVertex3d(pp.x*1000, pp.y*1000, DebugZ);
                glEnd();
                break;
            case parsian_msgs::parsian_draw::NONE:
            default:break;
        }
    }

}

void GLSoccerView::updateConfig2(const parsian_msgs::parsian_team_configConstPtr &_config) {
}

void GLSoccerView::drawVectors(const double &x, const double &y, const double& size, const std_msgs::ColorRGBA &color) {
    glColor4d(color.r, color.g, color.b, color.a);
    glBegin(GL_LINES);
    glVertex3d(x+size, y+size, DebugZ);
    glVertex3d(x-size, y-size, DebugZ);
    glVertex3d(x-size, y+size, DebugZ);
    glVertex3d(x+size, y-size, DebugZ);
    glEnd();
}

QColor GLSoccerView::toQColor(const std_msgs::ColorRGBA &_color) {
    QColor c;
    c.fromRgbF(_color.r, _color.g, _color.b, _color.a);
    return c;
}

void GLSoccerView::updateDB(const parsian_msgs::parsian_draw_bufferConstPtr &_packet) {
    isSideLeft = _packet->wm.isLeft;
    if (ball.size() > m_config.ballHistory) ball.pop_front();
    robots.clear();

    for(int i=0; i < _packet->wm.our.size(); i++){
        Robot robot{};
        robot.loc.set(_packet->wm.our[i].pos.x*1000, _packet->wm.our[i].pos.y*1000);
        robot.id = i;
        robot.hasAngle = true;
        if(robot.hasAngle) robot.angle = DEG(std::atan2(_packet->wm.our[i].dir.y,_packet->wm.our[i].dir.x));
        robot.team = (_packet->wm.isYellow) ? teamYellow : teamBlue;
        robot.cameraID = 0;
        robot.conf = 0.9;
        robots.append(robot);
    }

    for(int i=0; i < _packet->wm.opp.size(); i++){
        Robot robot{};
        robot.loc.set(_packet->wm.opp[i].pos.x*1000, _packet->wm.opp[i].pos.y*1000);
        robot.id = i;
        robot.hasAngle = true;
        if(robot.hasAngle) robot.angle = DEG(std::atan2(_packet->wm.opp[i].dir.y,_packet->wm.opp[i].dir.x));
        robot.team = (!_packet->wm.isYellow) ? teamYellow : teamBlue;
        robot.cameraID = 0;
        robot.conf = 0.9;
        robots.append(robot);
    }
    vector2d tball;
    tball.x = _packet->wm.ball.pos.x*1000;
    tball.y = _packet->wm.ball.pos.y*1000;
    ball.push_back(tball);
    debugs2->draws.clear();
    for (const auto &a : _packet->draws.draws) debugs2->draws.push_back(a);

}

void GLSoccerView::toggleColor() {
    grayColor = !grayColor;
    redraw();
}

void GLSoccerView::updateConfig(const monitor_config::monitorConfig &_config) {
    m_config = _config;
}

QPoint GLSoccerView::getMousePos() {
    return m_mousepos;
}

void GLSoccerView::setRobotsReplceService(ros::ServiceClient &_client) {
    robotsClinet = &_client;
}

void GLSoccerView::setBallReplceService(ros::ServiceClient& _client) {
    ballClinet = &_client;
}

double GLSoccerView::distVec(double x1, double y1, double x2, double y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

QPointF GLSoccerView::mouseToFieldPos(QPoint _mouse) {
    double mahi = (fieldDim.field_length + fieldDim.boundary_width)/width();
    double mahi2 = (fieldDim.field_width + fieldDim.boundary_width)/height();
    double max;
    if (mahi > mahi2) {
        max = mahi;
        mahi2 = height()*mahi;
        mahi = (fieldDim.field_length + fieldDim.boundary_width);
    } else {
        max = mahi2;
        mahi = width()*mahi2;
        mahi2 = (fieldDim.field_width + fieldDim.boundary_width);
    }
    double reverse = (isSideLeft) ? 1 : -1;
    return {
            reverse*(1 - 2*(_mouse.x() + (viewXOffset/viewScale)) / width())*(-mahi) / 2 * (viewScale/max)/1000
            ,reverse*((1 - 2*(_mouse.y() - (viewYOffset/viewScale))/ height())*(mahi2)/2) * (viewScale/max)/1000
    };
}
