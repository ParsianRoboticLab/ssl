#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_util/geom/rect_2d.h>

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;


// Declare a test
TEST(Polygon2DTest, polygon2DTest) {

    ///
    const Polygon2D empty_polygon;
    EXPECT_TRUE( !empty_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ));

    //
    const Vector2D p( +100.0, +100.0 );

    std::vector< rcsc::Vector2D > v;
    v.push_back( p );

    const rcsc::Polygon2D point_polygon( v );

    EXPECT_TRUE( !point_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

    // strict checks
    EXPECT_TRUE(  point_polygon.contains( p ) );
    EXPECT_TRUE( !point_polygon.contains( p, false ) );

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