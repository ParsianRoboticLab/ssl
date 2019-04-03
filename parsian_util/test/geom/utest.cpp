#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_util/geom/rect_2d.h>
#include "test_polygon_2d.h"
#include "test_matrix_2d.h"
#include "test_rect_2d.h"
#include "test_segment_2d.h"
#include "test_triangle_2d.h"
#include "test_vector_2d.h"
#include "test_voronoi_diagram.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

#include <vector>
#include <cstdlib>

using namespace rcsc;

// Declare a test
TEST(Polygon2DTest, polygon2DTest) {
    testEmpty();
    testPointPolygon();
    testGetBoundingBox();
    testContains1();
    testContains2();
    testContains3();
    testContains4();
    testContains5();
    testContains6();
    testContains7();
    testContains8();
    testEmptyArea();
    testScissoring();
    testGetDistance();
    testXYCenter();
    testSignedArea2();
}

TEST(TestMatrix2D, testMatrix2D){
    testTranslate();
    testScale();
    testRotate();
    testMultiplication();
}

TEST(TestRect2D, testRect2D){
    testSet ();
    //TODO add intersection test
}

TEST(TestSegment2D,testSegment2D){
    testLength();
    testIntersection();
    testExistIntersectionExceptTerminalPoint();
    testExistIntersection();
    testExistIntersectionAtTerminalPoints();
    testIntersectsAtTerminalPoints();
    testIntersectsAtTerminalPointsParallelHorizontal();
    testIntersectsAtTerminalPointsParallelVertical();
    testIntersectWithPointSegment();
    testNearestPoint();
    testDistanceFromPoint();
    testDistanceFromPointOnLine();
    testDistanceFromPointComplex();
    testDistanceFromSegment();
    testOnSegmentStrictly();
}

TEST(TestTraingle2D, testTraingle2D){
    testSignedArea();
    testCentroid();
}

TEST(TestVector2D, testVector2D){
    testAssign();
    testDistance();
    testEquals();
    testRotateVector();
}
/*
TEST(TestVoronoiDiagram, testVoronoiDiagram){
    testEmptyVoronoi();
    testVoronoi();
}
*/
// Run all the tests that were declared with TEST(
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    ros::init(argc, argv, "tester");
    ros::NodeHandle nh;
    return RUN_ALL_TESTS();
}
