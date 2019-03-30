//
// Created by raziyeh on 3/29/19.
//

#ifndef PARSIAN_UTIL_TEST_MATRIX_2D_CPP_H
#define PARSIAN_UTIL_TEST_MATRIX_2D_CPP_H

#include <parsian_util/geom/matrix_2d.h>
#include "config.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;

void testTranslate()
{
    {
        rcsc::Matrix2D m;
        m.translate( 2.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        EXPECT_NEAR( rcsc::Vector2D( 2.0, 4.0 ).dist( v ), 0.0, EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_translation( 3.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        EXPECT_NEAR( rcsc::Vector2D( 3.0, 4.0 ).dist( v ), 0.0, EPS );
    }
}

void testScale()
{
    {
        rcsc::Matrix2D m;
        m.scale( 2.0, 4.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( 1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, 3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        EXPECT_NEAR( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
        EXPECT_NEAR( rcsc::Vector2D( 2.0, 4.0 ).dist( v2 ), 0.0, EPS );
        EXPECT_NEAR( rcsc::Vector2D( 4.0, 12.0 ).dist( v3 ), 0.0, EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_scaling( -3.0, 1.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( -1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, -3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        EXPECT_NEAR( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
        EXPECT_NEAR( rcsc::Vector2D( 3.0, 1.0 ).dist( v2 ), 0.0, EPS );
        EXPECT_NEAR( rcsc::Vector2D( -6.0, -3.0 ).dist( v3 ), 0.0, EPS );
    }
}

void testRotate()
{
    rcsc::AngleDeg angle = 90.0;
    rcsc::Matrix2D m = rcsc::Matrix2D::make_rotation( angle );
    //rcsc::Matrix2D m;
    //m.rotate( 90.0 );

    rcsc::Vector2D v1( 0.0, 0.0 );
    rcsc::Vector2D v2( 1.0, 0.0 );
    rcsc::Vector2D v3( 0.0, 1.0 );
    rcsc::Vector2D v4( 2.0, 3.0 );

    rcsc::Vector2D rv1 = v1.rotatedVector( angle );
    rcsc::Vector2D rv2 = v2.rotatedVector( angle );
    rcsc::Vector2D rv3 = v3.rotatedVector( angle );
    rcsc::Vector2D rv4 = v4.rotatedVector( angle );

    m.transform( &v1 );
    m.transform( &v2 );
    m.transform( &v3 );
    m.transform( &v4 );

    EXPECT_NEAR( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
    EXPECT_NEAR( rcsc::Vector2D( 0.0, 1.0 ).dist( v2 ), 0.0, EPS );
    EXPECT_NEAR( rcsc::Vector2D( -1.0, 0.0 ).dist( v3 ), 0.0, EPS );
    EXPECT_NEAR( rcsc::Vector2D( -3.0, 2.0 ).dist( v4 ), 0.0, EPS );

    EXPECT_NEAR( rv1.dist( v1 ), 0.0, EPS );
    EXPECT_NEAR( rv2.dist( v2 ), 0.0, EPS );
    EXPECT_NEAR( rv3.dist( v3 ), 0.0, EPS );
    EXPECT_NEAR( rv4.dist( v4 ), 0.0, EPS );
}

void testMultiplication()
{
    rcsc::Vector2D v( 3.5821, -292.23 );
    rcsc::Vector2D scale( 5.62, 92.092 );
    rcsc::Vector2D translate( 3.2, 5.4 );
    rcsc::AngleDeg rotate = 14.0;

    rcsc::Matrix2D m1;
    m1.rotate( rotate );
    m1.translate( translate.x, translate.y );
    m1.scale( scale.x, scale.y );

    rcsc::Matrix2D m2
            = rcsc::Matrix2D::make_scaling( scale.x, scale.y )
              * rcsc::Matrix2D::make_translation( translate.x, translate.y )
              * rcsc::Matrix2D::make_rotation( rotate );

    EXPECT_NEAR( m1.transform( v ).dist( m2.transform( v ) ), 0.0, EPS );
}

#endif //PARSIAN_UTIL_TEST_MATRIX_2D_CPP_H
