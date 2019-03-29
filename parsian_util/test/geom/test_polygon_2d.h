//
// Created by raziyeh on 3/29/19.
//

#ifndef PARSIAN_UTIL_TEST_H
#define PARSIAN_UTIL_TEST_H

#include <parsian_util/geom/polygon_2d.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_util/geom/rect_2d.h>
#include "test_polygon_2d.h"


#include <gtest/gtest.h>
#include <ros/ros.h>

#include <vector>
#include <cstdlib>

using namespace rcsc;
void testEmpty()
{
    Polygon2D empty_polygon;
    EXPECT_TRUE(!empty_polygon.contains(rcsc::Vector2D(0.0, 0.0)));
}

void testPointPolygon(){
    const Vector2D p(+100.0, +100.0);

    std::vector<rcsc::Vector2D> v;
    v.push_back(p);

    const rcsc::Polygon2D point_polygon(v);

    EXPECT_TRUE(!point_polygon.contains(rcsc::Vector2D(0.0, 0.0)));

    // strict checks
    EXPECT_TRUE(point_polygon.contains(p));
    EXPECT_TRUE(!point_polygon.contains(p, false));
}

void testGetBoundingBox(){
    std::vector<rcsc::Vector2D> rect;
    rect.emplace_back(rcsc::Vector2D(+200.0, +100.0));
    rect.emplace_back(rcsc::Vector2D(-200.0, +100.0));
    rect.emplace_back(rcsc::Vector2D(-200.0, -100.0));
    rect.emplace_back(rcsc::Vector2D(+200.0, -100.0));

    const rcsc::Polygon2D rectangle(rect);


    //
    // getBoundingBox()
    //
    const rcsc::Rect2D r = rectangle.getBoundingBox();

    ASSERT_DOUBLE_EQ(-200.0 - r.minX(), 0.0);
    ASSERT_DOUBLE_EQ(+200.0 - r.maxX(), 0.0);
    ASSERT_DOUBLE_EQ(-100.0 - r.minY(), 0.0);
    ASSERT_DOUBLE_EQ(+100.0 - r.maxY(), 0.0);
}

void testContains1(){
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( rcsc::Vector2D( +200.0, +100.0 ) );
    rect.emplace_back( rcsc::Vector2D( -200.0, +100.0 ) );
    rect.emplace_back( rcsc::Vector2D( -200.0, -100.0 ) );
    rect.emplace_back( rcsc::Vector2D( +200.0, -100.0 ) );

    const rcsc::Polygon2D rectangle( rect );

    //
    // contains
    //
    ASSERT_TRUE(  rectangle.contains( rcsc::Vector2D(    0.0,    0.0 ) ) );
    ASSERT_TRUE(  rectangle.contains( rcsc::Vector2D(   50.0,   50.0 ) ) );
    ASSERT_TRUE(  rectangle.contains( rcsc::Vector2D(  199.9,   99.9 ) ) );
    ASSERT_TRUE(  rectangle.contains( rcsc::Vector2D( -199.9, - 99.9 ) ) );
    ASSERT_TRUE( !rectangle.contains( rcsc::Vector2D(  200.1,  100.1 ) ) );
    ASSERT_TRUE( !rectangle.contains( rcsc::Vector2D( -200.1, -100.1 ) ) );
    ASSERT_TRUE( !rectangle.contains( rcsc::Vector2D( +500.0, +500.0 ) ) );
    ASSERT_TRUE( !rectangle.contains( rcsc::Vector2D(    0.0, +500.0 ) ) );
}

void testContains2(){
    //
    // contains 2
    //
    std::vector< rcsc::Vector2D > tri;
    tri.emplace_back( rcsc::Vector2D( -200.0, -100.0 ) );
    tri.emplace_back( rcsc::Vector2D(    0.0, +100.0 ) );
    tri.emplace_back( rcsc::Vector2D( +200.0, -100.0 ) );

    const rcsc::Polygon2D triangle( tri );

    ASSERT_TRUE(  triangle.contains( rcsc::Vector2D( 0.0,    0.0 ) ) );
    ASSERT_TRUE( !triangle.contains( rcsc::Vector2D( 0.0, -300.0 ) ) );
    ASSERT_TRUE( !triangle.contains( rcsc::Vector2D( 0.1, -300.0 ) ) );
}

void testContains3(){
    //
    // contains 3
    //
    std::vector< rcsc::Vector2D > tri2;
    tri2.emplace_back( rcsc::Vector2D(   0.0,   0.0 ) );
    tri2.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri2.emplace_back( rcsc::Vector2D(   0.0, 200.0 ) );

    const rcsc::Polygon2D triangle2( tri2 );

    ASSERT_TRUE( !triangle2.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
    ASSERT_TRUE(  triangle2.contains( rcsc::Vector2D(   50.0, 100.0 ) ) );
}

void testContains4(){
    //
    // contains 4
    //
    std::vector< rcsc::Vector2D > tri3;
    tri3.emplace_back( rcsc::Vector2D(   0.0,   0.0 ) );
    tri3.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri3.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri3.emplace_back( rcsc::Vector2D(   0.0, 200.0 ) );

    const rcsc::Polygon2D triangle3( tri3 );

    ASSERT_TRUE( !triangle3.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
}

void testContains5(){
    //
    // contains 5
    //
    std::vector< rcsc::Vector2D > tri4;
    tri4.emplace_back( rcsc::Vector2D(   0.0,   0.0 ) );
    tri4.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri4.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri4.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    tri4.emplace_back( rcsc::Vector2D(   0.0, 200.0 ) );

    const rcsc::Polygon2D triangle4( tri4 );

    ASSERT_TRUE( !triangle4.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
}

void testContains6(){
    //
    // contains 6
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( rcsc::Vector2D(  0,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10, 10 ) );
    rect.emplace_back( rcsc::Vector2D(  0, 10 ) );

    const rcsc::Polygon2D r( rect );

    ASSERT_TRUE( ! r.contains( rcsc::Vector2D( -100, 0 ) ) );
}

void testContains7(){
    //
    // contains (grid)
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( rcsc::Vector2D(  0,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10, 10 ) );
    rect.emplace_back( rcsc::Vector2D(  0, 10 ) );

    const rcsc::Polygon2D r( rect );

    int count = 0;

    for ( int x = -100; x <= +100; ++x )
    {
        for ( int y = -100; y <= +100; ++y )
        {
            if (    0 <= x && x <= 10
                    && 0 <= y && y <= 10 )
            {
                continue;
            }

            if ( r.contains( rcsc::Vector2D( x, y ) ) )
            {
                ++count;
            }
        }
    }

    ASSERT_EQ( 0, count );
}

void testContains8(){
    //
    // contains
    //
    std::vector< rcsc::Vector2D > v;
    v.emplace_back( rcsc::Vector2D( 100, 100 ) );
    v.emplace_back( rcsc::Vector2D( 200, 100 ) );
    v.emplace_back( rcsc::Vector2D( 200, 500 ) );

    const rcsc::Polygon2D tri( v );

    //                    //
    //  po1               //
    //                    //
    //  po2          p5   //
    //              /|    //
    //             / |    //
    //            /  |    //
    //           /   |    //
    //          /    |    //
    //         /     |    //
    //  po3  p7  p1  p6   //
    //       /       |    //
    //  po4 p4---p2--p3   //
    //                    //
    //  po5               //

    rcsc::Vector2D p1( 150, 150 );
    rcsc::Vector2D p2( 150, 100 );
    rcsc::Vector2D p3( 200, 100 );
    rcsc::Vector2D p4( 100, 100 );
    rcsc::Vector2D p5( 200, 500 );
    rcsc::Vector2D p6( 200, 150 );
    rcsc::Vector2D p7( 200, 150 );

    rcsc::Vector2D po1( 50, 600 );
    rcsc::Vector2D po2( 50, 500 );
    rcsc::Vector2D po3( 50, 150 );
    rcsc::Vector2D po4( 50, 100 );
    rcsc::Vector2D po5( 50,   0 );


    ASSERT_TRUE( tri.contains( p1 ) );
    ASSERT_TRUE( tri.contains( p1, false ) );

    ASSERT_TRUE(  tri.contains( p2 ) );
    ASSERT_TRUE( !tri.contains( p2, false ) );

    ASSERT_TRUE(  tri.contains( p3 ) );
    ASSERT_TRUE( !tri.contains( p3, false ) );

    ASSERT_TRUE(  tri.contains( p4 ) );
    ASSERT_TRUE( !tri.contains( p4, false ) );

    ASSERT_TRUE(  tri.contains( p5 ) );
    ASSERT_TRUE( !tri.contains( p5, false ) );

    ASSERT_TRUE(  tri.contains( p6 ) );
    ASSERT_TRUE( !tri.contains( p6, false ) );

    ASSERT_TRUE(  tri.contains( p7 ) );
    ASSERT_TRUE( !tri.contains( p7, false ) );


    ASSERT_TRUE( !tri.contains( po1 ) );
    ASSERT_TRUE( !tri.contains( po1, false ) );

    ASSERT_TRUE( !tri.contains( po2 ) );
    ASSERT_TRUE( !tri.contains( po2, false ) );

    ASSERT_TRUE( !tri.contains( po3 ) );
    ASSERT_TRUE( !tri.contains( po3, false ) );

    ASSERT_TRUE( !tri.contains( po4 ) );
    ASSERT_TRUE( !tri.contains( po4, false ) );

    ASSERT_TRUE( !tri.contains( po5 ) );
    ASSERT_TRUE( !tri.contains( po5, false ) );
}

void testEmptyArea(){
    //
    // empty area
    //
    std::vector< rcsc::Vector2D > a0;
    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );

    const rcsc::Polygon2D area_1( a0 );

    a0.emplace_back( rcsc::Vector2D( 100.0, 100.0 ) );
    const rcsc::Polygon2D area_2( a0 );


    ASSERT_TRUE( !area_1.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );
    ASSERT_TRUE( !area_2.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

    // strict checks
    ASSERT_TRUE(  area_1.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
    ASSERT_TRUE( !area_1.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );

    // strict checks
    ASSERT_TRUE(  area_2.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
    ASSERT_TRUE( !area_2.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );
}

void testScissoring(){
    //
    // scissoring
    //
    const rcsc::Rect2D rectangle( rcsc::Vector2D( -100, -100 ),
                                  rcsc::Size2D( /* length of x */ 200, /* length of y */ 200 ) );

    //                         //
    //              (200,200)  //
    //           +---------+   //
    //           |         |   //
    //    -100   |         |   //
    // +100 +----|----+    |   //
    //      |    |    |    |   //
    //      |    |    |    |   //
    //      |    +---------+   //
    //      |   (0,0) |        //
    //      |         |        //
    // -100 +---------+        //
    //                         //

    std::vector< rcsc::Vector2D > v;
    v.emplace_back( rcsc::Vector2D(   0,   0 ) );
    v.emplace_back( rcsc::Vector2D( 200,   0 ) );
    v.emplace_back( rcsc::Vector2D( 200, 200 ) );
    v.emplace_back( rcsc::Vector2D(   0, 200 ) );
    v.emplace_back( rcsc::Vector2D(   0,   0 ) );

    const rcsc::Polygon2D polygon( v );

    const rcsc::Polygon2D result = polygon.getScissoredConnectedPolygon( rectangle );

    ASSERT_DOUBLE_EQ( 10000.0 - result.area(), 0.0);

    const rcsc::Rect2D bbox = result.getBoundingBox();

    ASSERT_DOUBLE_EQ(   0.0 - bbox.minX(), 0.0 );
    ASSERT_DOUBLE_EQ( 100.0 - bbox.maxX(), 0.0 );
    ASSERT_DOUBLE_EQ(   0.0 - bbox.minY(), 0.0 );
    ASSERT_DOUBLE_EQ( 100.0 - bbox.maxY(), 0.0 );
}

void testGetDistance(){
    //
    // get_distance
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( rcsc::Vector2D(  0,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10,  0 ) );
    rect.emplace_back( rcsc::Vector2D( 10, 10 ) );
    rect.emplace_back( rcsc::Vector2D(  0, 10 ) );

    const rcsc::Polygon2D r( rect );

    // out of polygon
    ASSERT_DOUBLE_EQ( 1.0 - r.dist( rcsc::Vector2D( 11.0, 10.0 ) ), 0.0 );

    // in polygon, check as plane
    ASSERT_DOUBLE_EQ( 0.0 - r.dist( rcsc::Vector2D( 5.0, 5.0 ) ), 0.0 );

    // in polygon, check as polyline
    ASSERT_DOUBLE_EQ( 5.0 - r.dist( rcsc::Vector2D( 5.0, 5.0 ), false ), 0.0 );
}

void testXYCenter(){
    //
    // area, xyCenter
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( rcsc::Vector2D( 10, 10 ) );
    rect.emplace_back( rcsc::Vector2D( 20, 10 ) );
    rect.emplace_back( rcsc::Vector2D( 20, 20 ) );
    rect.emplace_back( rcsc::Vector2D( 10, 20 ) );

    const rcsc::Polygon2D r( rect );

    ASSERT_DOUBLE_EQ( +100.0 - r.area(), 0.0 );

    ASSERT_DOUBLE_EQ( rcsc::Vector2D( 15.0, 15.0 ).dist( r.xyCenter() ), 0.0 );
}

void testSignedArea2(){
    //
    // counter clockwise/clockwise, doubleSignedArea
    //
    std::vector< rcsc::Vector2D > points;
    const rcsc::Polygon2D empty(points);

    points.emplace_back( rcsc::Vector2D( 10, 10 ) );
    const rcsc::Polygon2D point(points);

    points.emplace_back( rcsc::Vector2D( 20, 10 ) );
    const rcsc::Polygon2D line(points);

    points.emplace_back( rcsc::Vector2D( 20, 20 ) );
    const rcsc::Polygon2D triangle(points);

    points.emplace_back( rcsc::Vector2D( 10, 20 ) );
    const rcsc::Polygon2D rectangle(points);


    ASSERT_EQ( false, empty.isCounterclockwise() );
    ASSERT_EQ( false, empty.isClockwise() );

    ASSERT_EQ( false, point.isCounterclockwise() );
    ASSERT_EQ( false, point.isClockwise() );

    ASSERT_EQ( false, line.isCounterclockwise() );
    ASSERT_EQ( false, line.isClockwise() );

    ASSERT_EQ( true , triangle.isCounterclockwise() );
    ASSERT_EQ( false, triangle.isClockwise() );

    ASSERT_EQ( true , triangle.isCounterclockwise() );
    ASSERT_EQ( false, triangle.isClockwise() );


    std::vector< rcsc::Vector2D > r_points;
    r_points.emplace_back( rcsc::Vector2D( 10, 20 ) );
    r_points.emplace_back( rcsc::Vector2D( 20, 20 ) );
    r_points.emplace_back( rcsc::Vector2D( 20, 10 ) );
    const rcsc::Polygon2D r_triangle(r_points);

    r_points.emplace_back( rcsc::Vector2D( 10, 10 ) );
    const rcsc::Polygon2D r_rectangle(r_points);

    ASSERT_EQ( false, r_triangle.isCounterclockwise() );
    ASSERT_EQ( true , r_triangle.isClockwise() );

    ASSERT_EQ( false, r_rectangle.isCounterclockwise() );
    ASSERT_EQ( true , r_rectangle.isClockwise() );
}


#endif //PARSIAN_UTIL_TEST_H

