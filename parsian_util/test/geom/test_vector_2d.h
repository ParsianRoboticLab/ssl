//
// Created by raziyeh on 3/30/19.
//

#ifndef PARSIAN_UTIL_TEST_VECTOR_2D_H
#define PARSIAN_UTIL_TEST_VECTOR_2D_H

#include <parsian_util/geom/vector_2d.h>
#include "config.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;


bool in_distance( const double & x,
             const double & y )
{
    return std::fabs( x - y ) < DISTANCE;
}

bool in_distance2( const double & x,
              const double & y )
{
    return std::fabs( x - y ) < DISTANCE * DISTANCE;
}

void testAssign()
{
    //
    const Vector2D p0;
    ASSERT_TRUE( in_distance( p0.x, 5000.0 ) );
    ASSERT_TRUE( in_distance( p0.y, 5000.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    ASSERT_TRUE( in_distance( p1.x, 1.0 ) );
    ASSERT_TRUE( in_distance( p1.y, -2.0 ) );

    //
    const Vector2D p2( -3.5, 4.5 );

    //
    const Vector2D p3 = p2;
    ASSERT_TRUE( in_distance( p3.x, -3.5 ) );
    ASSERT_TRUE( in_distance( p3.y, 4.5 ) );

    //
    Vector2D p4;
    p4 = p2;
    ASSERT_TRUE( in_distance( p4.x, -3.5 ) );
    ASSERT_TRUE( in_distance( p4.y, 4.5 ) );
}

void testDistance()
{
    //
    const Vector2D p0;
    //ASSERT_TRUE( in_distance( p0.dist( Vector2D::ORIGIN ), 0.0 ) );
    ASSERT_TRUE( in_distance( p0.dist( ERROR_VALUE ), 0.0 ) );
    ASSERT_TRUE( in_distance( p0.dist( Vector2D() ), 0.0 ) );
    //ASSERT_TRUE( in_distance2( p0.dist2( Vector2D::ORIGIN ), 0.0 ) );
    ASSERT_TRUE( in_distance2( p0.dist2( ERROR_VALUE ), 0.0 ) );
    ASSERT_TRUE( in_distance2( p0.dist2( Vector2D() ), 0.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    //ASSERT_TRUE( in_distance( p1.dist( Vector2D::ORIGIN ), std::sqrt( 5.0 ) ) );
    ASSERT_TRUE( in_distance( p1.dist( ZERO ), std::sqrt( 5.0 ) ) );
    //ASSERT_TRUE( in_distance2( p1.dist2( Vector2D::ORIGIN ), 5.0 ) );
    ASSERT_TRUE( in_distance2( p1.dist2( ZERO ), 5.0 ) );
    //
    const Vector2D p2( 4.0, 2.0 );
    ASSERT_TRUE( in_distance( p2.dist( p1 ), 5.0 ) );
    ASSERT_TRUE( in_distance( p2.dist2( p1 ), 25.0 ) );
}

void testEquals()
{
    //
    const Vector2D p0;
    //ASSERT_TRUE( p0 == Vector2D::ORIGIN );
    ASSERT_TRUE( p0 == ERROR_VALUE );
    ASSERT_TRUE( p0 == Vector2D() );
    ASSERT_TRUE( p0 != Vector2D( DISTANCE * 2.0, DISTANCE * 2.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    //ASSERT_TRUE( p1 != Vector2D::ORIGIN );
    ASSERT_TRUE( p1 != ZERO );
    ASSERT_TRUE( p1 == p1 );
    ASSERT_TRUE( p1 == Vector2D( 1.0, -2.0 ) );
    ASSERT_TRUE( p1 == Vector2D( 1.0 + DISTANCE3 * 2.0, -2.0 + DISTANCE3 * 2.0 ) );
    ASSERT_TRUE( p1 != Vector2D( 1.0 + DISTANCE2 * 2.0, -2.0 + DISTANCE2 * 2.0 ) );
}


void testRotateVector()
{
    const Vector2D v( 1.0, 1.0 );
    const AngleDeg rot = -30.0;

    std::cerr << '\n';

    Vector2D v1 = v.rotatedVector( rot );
    std::cerr << "v1=" << v1 << " th=" << v1.th() << std::endl;
    EXPECT_NEAR( ( v.th() + rot ).degree(), v1.th().degree(), 1.0e-5 );

    Vector2D v2( v.x * rot.cos() - v.y * rot.sin(),
                 v.x * rot.sin() + v.y * rot.cos() );
    std::cerr << "v2=" << v2 << " th=" << v2.th() << std::endl;
    EXPECT_NEAR( ( v.th() + rot ).degree(), v2.th().degree(), 1.0e-5 );
}

#endif //PARSIAN_UTIL_TEST_VECTOR_2D_H
