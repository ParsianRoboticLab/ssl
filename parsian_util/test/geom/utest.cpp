#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_util/geom/rect_2d.h>
#include "test_polygon_2d.h"

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

}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    ros::init(argc, argv, "tester");
    ros::NodeHandle nh;
    return RUN_ALL_TESTS();
}
