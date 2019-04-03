//
// Created by raziyeh on 3/30/19.
//

#ifndef PARSIAN_UTIL_TEST_TRIANGLE_2D_H
#define PARSIAN_UTIL_TEST_TRIANGLE_2D_H

#include <parsian_util/geom/triangle_2d.h>
#include "config.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;

void testSignedArea()
{
    //
    // basic checks
    //
    {
        const rcsc::Vector2D p1( 0.0, 0.0 );
        const rcsc::Vector2D p2( 3.0, 0.0 );
        const rcsc::Vector2D p3( 3.0, 4.0 );

        const rcsc::Triangle2D t1( p1, p2, p3 );
        const rcsc::Triangle2D t2( p3, p2, p1 );

        EXPECT_NEAR( + 6.0 - t1.signedArea(), 0.0, EPS );
        EXPECT_NEAR( +12.0 - t1.doubleSignedArea(), 0.0, EPS );

        EXPECT_NEAR( - 6.0 - t2.signedArea(), 0.0, EPS );
        EXPECT_NEAR( -12.0 - t2.doubleSignedArea(), 0.0, EPS );
    }


    //
    // points on a line
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2(  600, 200 );
        const rcsc::Vector2D p3(    0, 200 );

        const rcsc::Triangle2D tri( p1, p2, p3 );

        // should be EXACTRY equal to 0
        EXPECT_NEAR( tri.doubleSignedArea(), 0.0, EPS );
    }


    //
    // same 2 points
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2( + 50, 100 );

        const rcsc::Triangle2D tri1( p1, p1, p2 );
        const rcsc::Triangle2D tri2( p1, p2, p1 );
        const rcsc::Triangle2D tri3( p2, p1, p1 );

        // should be EXACTRY equal to 0
        EXPECT_NEAR( tri1.doubleSignedArea(), 0.0, EPS );
        EXPECT_NEAR( tri2.doubleSignedArea(), 0.0, EPS );
        EXPECT_NEAR( tri3.doubleSignedArea(), 0.0, EPS );
    }


    //
    // same 3 points
    //
    {
        const rcsc::Vector2D p( -100, 200 );

        const rcsc::Triangle2D tri( p, p, p );

        // should be EXACTRY equal to 0
        EXPECT_NEAR( tri.doubleSignedArea(), 0.0, EPS );
    }
}


void testCentroid()
{
    {
        rcsc::Vector2D p1( 5.1245, 9.1038 );
        rcsc::Vector2D p2( 3.0, -5.6978 );
        rcsc::Vector2D p3( 3.0, 4.0 );

        rcsc::Triangle2D tri( p1, p2, p3 );


        rcsc::Vector2D c = p1 + p2 + p3;

        EXPECT_NEAR( c.r() - tri.centroid().r() * 3.0, 0.0, EPS );
    }
}

#endif //PARSIAN_UTIL_TEST_TRIANGLE_2D_H
