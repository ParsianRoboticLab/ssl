//
// Created by raziyeh on 3/29/19.
//

#ifndef PARSIAN_UTIL_TEST_SEGMENT_2D_H
#define PARSIAN_UTIL_TEST_SEGMENT_2D_H

#include <parsian_util/geom/segment_2d.h>
#include "config.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;

void testLength()
{
    //
    // check length of segment
    //
    const Segment2D s1( Vector2D( 0.0, 0.0 ),
                        Vector2D( 3.0, 4.0 ) );

    EXPECT_NEAR( 5.0 - s1.length(), 0.0, EPS );
}

void testIntersection()
{
    const double delta = 1.0e-6;
    const Segment2D segment( Vector2D( 0.0, 0.0 ),
                             Vector2D( 2.0, 0.0 ) );

    //
    Vector2D result;
    Segment2D s( Vector2D( 0.0, 0.0 ), Vector2D( 0.0, 0.0 ) );
//1
    s.assign( Vector2D( 0.0, 0.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 0.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 0.0, EPS );
//2
    s.assign( Vector2D( 0.0, 1.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 0.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 1.0, EPS );
//3
    s.assign( Vector2D( 0.0, 2.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 0.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 2.0, EPS );
//4
    s.assign( Vector2D( 1.0, 0.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 1.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 0.0, EPS );
//5
    s.assign( Vector2D( 0.0, -1.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 1.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), std::sqrt( 2.0 ), EPS );
//6
    s.assign( Vector2D( -1.0, -2.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 1.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), std::sqrt( 2.0 ) * 2.0, EPS );
//7
    s.assign( Vector2D( 2.0, 0.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 2.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 0.0, EPS );
//8
    s.assign( Vector2D( 2.0, -1.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 2.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 1.0, EPS );
//9
    s.assign( Vector2D( 2.0, -2.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
    EXPECT_NEAR( result.dist( segment.origin() ), 2.0, EPS );
    EXPECT_NEAR( result.dist( s.origin() ), 2.0, EPS );
//10
    s.assign( Vector2D( 0.0, -delta * 2.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE( result.isValid() );
//11
    s =  Segment2D( Vector2D( -delta * 2.0, 1.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//12
    s.assign( Vector2D( 0.0, 2.0 ), 2.0 - delta * 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//13
    s.assign( Vector2D( 1.0 + delta * 2.0, delta * 2.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//14
    s.assign( Vector2D( -1.0, -2.0 ), std::sqrt( 2.0 ) * 2.0 - delta * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//15
    s.assign( Vector2D( 2.0, delta * 2.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//16
    s.assign( Vector2D( 2.0 + delta * 2.0, -1.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
//17
    s.assign( Vector2D( 2.0, -2.0 ), 2.0 - delta * 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s );
    ASSERT_TRUE(  result.isValid() );
}

void testExistIntersectionExceptTerminalPoint()
{
    //
    // check existIntersectionExceptTerminalPoint()
    //
    const rcsc::Segment2D s1( rcsc::Vector2D( 0.0, 0.0 ),
                              rcsc::Vector2D( 3.0, 4.0 ) );

    const rcsc::Segment2D s2( rcsc::Vector2D( 0.0, 2.0 ),
                              rcsc::Vector2D( 5.0, 2.0 ) );

    ASSERT_TRUE( s1.existIntersectionExceptEndpoint( s2 ) );
    ASSERT_TRUE( s2.existIntersectionExceptEndpoint( s1 ) );

    ASSERT_TRUE( s1.intersection( s2 ).isValid() );
    ASSERT_TRUE( s2.intersection( s1 ).isValid() );

    //     const rcsc::Segment2D s3( rcsc::Vector2D( 100.0, 200.0 ),
    //                               rcsc::Vector2D( 300.0, 400.0 ) );
    //     const rcsc::Segment2D s3( rcsc::Vector2D( s1.origin().x - 1.0, s1.origin().y + 1.0 ),
    //                               rcsc::Vector2D( s1.origin().x + 1.0, s1.origin().y - 1.0 ) );
    const rcsc::Segment2D s3( rcsc::Vector2D( s1.terminal().x - 1.0, s1.terminal().y + 1.0 ),
                              rcsc::Vector2D( s1.terminal().x + 1.0, s1.terminal().y - 1.0 ) );

    ASSERT_TRUE( ! s3.existIntersectionExceptEndpoint( s1 ) );
    ASSERT_TRUE( ! s3.existIntersectionExceptEndpoint( s2 ) );
    ASSERT_TRUE( ! s1.existIntersectionExceptEndpoint( s3 ) );
    ASSERT_TRUE( ! s2.existIntersectionExceptEndpoint( s3 ) );

    ASSERT_TRUE( s3.intersection( s1 ).isValid() );
    ASSERT_TRUE( ! s3.intersection( s2 ).isValid() );
    ASSERT_TRUE( s1.intersection( s3 ).isValid() );
    ASSERT_TRUE( ! s2.intersection( s3 ).isValid() );


    // 2 segments on a line
    const rcsc::Segment2D s1_2( rcsc::Vector2D( 6.0,  8.0 ),
                                rcsc::Vector2D( 9.0, 12.0 ) );

    ASSERT_TRUE( ! s1.existIntersectionExceptEndpoint( s1_2 ) );
    ASSERT_TRUE( ! s1_2.existIntersectionExceptEndpoint( s1 ) );

    ASSERT_TRUE( ! s1.intersection( s1_2 ).isValid() );
    ASSERT_TRUE( ! s1_2.intersection( s1 ).isValid() );


    const rcsc::Segment2D s4( rcsc::Vector2D( -100.0, 4.0 ),
                              rcsc::Vector2D( +100.0, 4.0 ) );

    ASSERT_TRUE( s1.existIntersection( s4 ) );
    ASSERT_TRUE( s4.existIntersection( s1 ) );

    ASSERT_TRUE( s1.intersection( s4 ).isValid() );
    ASSERT_TRUE( s4.intersection( s1 ).isValid() );
}

void testExistIntersection()
{
    //
    // check existIntersection()
    //
    const rcsc::Segment2D t1( rcsc::Vector2D( 100, 100 ),
                              rcsc::Vector2D(   0, 200 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( -100, 200 ),
                              rcsc::Vector2D(  600, 200 ) );

    ASSERT_TRUE( t1.existIntersection( t2 ) );
    ASSERT_TRUE( t2.existIntersection( t1 ) );

    ASSERT_TRUE( t1.intersection( t2 ).isValid() );
    ASSERT_TRUE( t2.intersection( t1 ).isValid() );
}

void testExistIntersectionAtTerminalPoints()
{
    // existIntersection at terminal points
    const rcsc::Segment2D t1( rcsc::Vector2D( -200.0, -100.0 ),
                              rcsc::Vector2D(    0.0, +100.0 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D(    0.0, +100.0 ),
                              rcsc::Vector2D( +200.0, -100.0 ) );

    const rcsc::Segment2D t_check( rcsc::Vector2D( 0.0, -300.0 ),
                                   rcsc::Vector2D( 0.0, +900.0 ) );

    ASSERT_TRUE( t1.existIntersection( t_check ) );
    ASSERT_TRUE( t_check.existIntersection( t1 ) );

    ASSERT_TRUE( t1.intersection( t_check ).isValid() );
    ASSERT_TRUE( t_check.intersection( t1 ).isValid() );

    //

    ASSERT_TRUE( t2.existIntersection( t_check ) );
    ASSERT_TRUE( t_check.existIntersection( t2 ) );

    ASSERT_TRUE( t2.intersection( t_check ).isValid() );
    ASSERT_TRUE( t_check.intersection( t2 ).isValid() );
}

void testIntersectsAtTerminalPoints()
{
    // intersects at terminal points
    const rcsc::Segment2D t1( rcsc::Vector2D(  200, 100 ),
                              rcsc::Vector2D( 2000, 100 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D(  200, 100 ),
                              rcsc::Vector2D(  200, 500 ) );

    ASSERT_TRUE( t1.existIntersection( t2 ) );
    ASSERT_TRUE( t2.existIntersection( t1 ) );

    ASSERT_TRUE( t1.intersection( t2 ).isValid() );
    ASSERT_TRUE( t2.intersection( t1 ).isValid() );
}

void testIntersectsAtTerminalPointsParallelHorizontal()
{
    // intersects at terminal points (parallel, horizontal)
    const rcsc::Segment2D t1( rcsc::Vector2D( +200, +100 ),
                              rcsc::Vector2D( +500, +100 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( +200, +100 ),
                              rcsc::Vector2D( -100, +100 ) );

    ASSERT_TRUE( t1.existIntersection( t2 ) );
    ASSERT_TRUE( t2.existIntersection( t1 ) );

    ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
    ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
}

void testIntersectsAtTerminalPointsParallelVertical()
{
    // intersects with terminal points (parallel, vertical)
    const rcsc::Segment2D t1( rcsc::Vector2D( +100, +200 ),
                              rcsc::Vector2D( +100, +500 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( +100, +200 ),
                              rcsc::Vector2D( +100, -100 ) );

    ASSERT_TRUE( t1.existIntersection( t2 ) );
    ASSERT_TRUE( t2.existIntersection( t1 ) );

    EXPECT_NEAR( t1.dist( t2 ), 0.0, EPS );
    EXPECT_NEAR( t2.dist( t1 ), 0.0, EPS );

    ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
    ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
}

void testIntersectWithPointSegment()
{
    // intersect with point segment 1
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( 0,    0 ),
                                  rcsc::Vector2D( 0, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +100, +500 ),
                                  rcsc::Vector2D( +100, +500 ) );

        ASSERT_TRUE( ! t1.existIntersection( t2 ) );
        ASSERT_TRUE( ! t2.existIntersection( t1 ) );

        ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
        ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
    }

    // intersect with point segment 2
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +300, +500 ),
                                  rcsc::Vector2D( +200, +400 ) );


        ASSERT_TRUE( ! t1.existIntersection( t2 ) );
        ASSERT_TRUE( ! t2.existIntersection( t1 ) );

        ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
        ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
    }

    // intersect with point segment 3
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +300, +300 ),
                                  rcsc::Vector2D( +300, +300 ) );


        ASSERT_TRUE( ! t1.existIntersection( t2 ) );
        ASSERT_TRUE( ! t2.existIntersection( t1 ) );

        ASSERT_TRUE( t1.existIntersection( t1 ) );
        ASSERT_TRUE( t2.existIntersection( t2 ) );

        ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
        ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
    }

    // intersect with point segment 4
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D(    0, +500 ),
                                  rcsc::Vector2D( +100, +500 ) );


        ASSERT_TRUE( ! t1.existIntersection( t2 ) );
        ASSERT_TRUE( ! t2.existIntersection( t1 ) );

        ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
        ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
    }

    // intersect with point segment 5
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +500,    0 ),
                                  rcsc::Vector2D( +500, +100 ) );

        ASSERT_TRUE( ! t1.existIntersection( t2 ) );
        ASSERT_TRUE( ! t2.existIntersection( t1 ) );

        ASSERT_TRUE( ! t1.intersection( t2 ).isValid() );
        ASSERT_TRUE( ! t2.intersection( t1 ).isValid() );
    }
}

void testNearestPoint()
{
    //
    // check nearestPoint()
    //
    const rcsc::Vector2D s1( -500, 100 );
    const rcsc::Vector2D s2( +500, 100 );

    const rcsc::Segment2D s( s1, s2 );

    EXPECT_NEAR( rcsc::Vector2D( 0.0, 100.0 ).dist( s.nearestPoint( rcsc::Vector2D( 0.0, 0.0 ) ) ),
                                  0.0,
                                  EPS );

    EXPECT_NEAR( rcsc::Vector2D( 200.0, 100.0 ).dist( s.nearestPoint( rcsc::Vector2D( 200.0, 0.0 ) ) ),
                                  0.0,
                                  EPS );

    for ( long i = 0 ; i < 100000 ; i += 10 )
    {
        const rcsc::Vector2D p( i, +500 );

        rcsc::Vector2D c;

        if ( i <= 500 )
        {
            c = s.nearestPoint( +p );
            EXPECT_NEAR( rcsc::Vector2D( (+p).x, 100 ).dist( c ), 0.0, EPS );

            c = s.nearestPoint( -p );
            EXPECT_NEAR( rcsc::Vector2D( (-p).x, 100 ).dist( c ), 0.0, EPS );
        }
        else
        {
            c = s.nearestPoint( +p );
            EXPECT_NEAR( s2.dist( c ), 0.0, EPS );

            c = s.nearestPoint( -p );
            EXPECT_NEAR( s1.dist( c ), 0.0, EPS );
        }
    }
}

void testDistanceFromPoint()
{
    //
    // check distance of segment and point
    //
    const rcsc::Segment2D seg1( rcsc::Vector2D( -100.0, 0.0 ),
                                rcsc::Vector2D(    0.0, 0.0 ) );
    const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 0.0 ),
                                rcsc::Vector2D( -100.0, 0.0 ) );

    const rcsc::Vector2D p( 400.0, 300.0 );

    EXPECT_NEAR( 500.0 - seg1.dist( p ), 0.0, EPS );
    EXPECT_NEAR( 500.0 - seg2.dist( p ), 0.0, EPS );
}

void testDistanceFromPointOnLine()
{
    // distance from point (segment and point are on a line)
    const rcsc::Segment2D seg( rcsc::Vector2D( -100, 0.0 ),
                               rcsc::Vector2D( +100, 0.0 ) );

    const rcsc::Vector2D p( +150.0, 0.0 );

    EXPECT_NEAR(  50.0 - seg.dist( p ), 0.0, EPS );
    EXPECT_NEAR( 250.0 - seg.farthestDist( p ), 0.0, EPS );
}

void testDistanceFromPointComplex()
{
    // distance from point (complex)
    const rcsc::Vector2D s1( -100, 0 );
    const rcsc::Vector2D s2( +100, 0 );

    const rcsc::Segment2D seg( s1, s2 );

    const rcsc::Vector2D p1( 0, +150 );

    EXPECT_NEAR( 150.0 - seg.dist( +p1 ), 0.0, EPS );
    EXPECT_NEAR( 150.0 - seg.dist( -p1 ), 0.0, EPS );

    const rcsc::Vector2D p2( 300, 0 );
    EXPECT_NEAR( 200.0 - seg.dist( +p2 ), 0.0, EPS );
    EXPECT_NEAR( 200.0 - seg.dist( -p2 ), 0.0, EPS );

    const rcsc::Vector2D p3( 20000, 0 );
    EXPECT_NEAR( 19900.0 - seg.dist( +p3 ), 0.0, EPS );
    EXPECT_NEAR( 19900.0 - seg.dist( -p3 ), 0.0, EPS );

    for ( long  i = 0  ;  i < 100000  ;  i += 10 )
    {
        const rcsc::Vector2D p( i, +500 );

        if ( i <= 100 )
        {
            EXPECT_NEAR( 500.0 - seg.dist( +p ), 0.0, EPS );

            EXPECT_NEAR( 500.0 - seg.dist( -p ), 0.0, EPS );
        }
        else
        {
            EXPECT_NEAR( (s2 - p).r() - seg.dist( +p ), 0.0, EPS );

            EXPECT_NEAR( (s1 - (-p)).r() - seg.dist( -p ), 0.0, EPS );
        }
    }
}

void testDistanceFromSegment()
{
    //
    // distance segment and segment
    //
    const rcsc::Segment2D seg1( rcsc::Vector2D( +100.0, 100.0 ),
                                rcsc::Vector2D( -100.0, 100.0 ) );

    const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 300.0 ),
                                rcsc::Vector2D( +100.0, 400.0 ) );

    EXPECT_NEAR( 200.0 - seg1.dist( seg2 ), 0.0, EPS );
    EXPECT_NEAR( 200.0 - seg2.dist( seg1 ), 0.0, EPS );
}

void testOnSegmentStrictly()
{
    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ), rcsc::Vector2D( 0.0, 10.0 ) );
        ASSERT_TRUE( s.onSegment( rcsc::Vector2D( 0.0, 5.0 ) ) );
        ASSERT_TRUE( ! s.onSegment( rcsc::Vector2D( 1.0e-7, 0.0 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ),
                           rcsc::Vector2D( 10.0, 10.0 ) );
        ASSERT_TRUE( s.onSegment( rcsc::Vector2D( 5.0, 5.0 ) ) );
        ASSERT_TRUE( ! s.onSegment( rcsc::Vector2D( 6.0, 6.0 + 1.0e-7 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 3.148595, 582.2 ),
                           rcsc::Vector2D( -1838.235, 23.21145 ) );
        rcsc::Vector2D dir = s.terminal() - s.origin();
        dir.normalize();
        ASSERT_TRUE( ! s.onSegment( s.origin() + dir * 2.462134 ) );
    }
}

#endif //PARSIAN_UTIL_TEST_SEGMENT_2D_H
