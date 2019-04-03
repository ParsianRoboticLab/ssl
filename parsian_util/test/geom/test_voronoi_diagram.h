//
// Created by raziyeh on 3/30/19.
//
/*
#ifndef PARSIAN_UTIL_TEST_VORONOI_DIAGRAM_H
#define PARSIAN_UTIL_TEST_VORONOI_DIAGRAM_H

#include <parsian_util/geom/vector_2d.h>
#include <parsian_util/geom/voronoi_diagram.h>
#include "test_consts.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;


void testEmptyVoronoi()
{
    VoronoiDiagram v;

    ASSERT_EQ( static_cast< size_t >( 0 ), v.vertices().size() );
    ASSERT_EQ( static_cast< size_t >( 0 ), v.segments().size() );
    ASSERT_EQ( static_cast< size_t >( 0 ), v.rays().size() );

    v.compute();

    ASSERT_EQ( static_cast< size_t >( 0 ), v.vertices().size() );
    ASSERT_EQ( static_cast< size_t >( 0 ), v.segments().size() );
    ASSERT_EQ( static_cast< size_t >( 0 ), v.rays().size() );
}

void testVoronoi()
{
    const Vector2D p0(   0.0,   0.0 );
    const Vector2D p1( +10.0, +10.0 );
    const Vector2D p2( -10.0, +10.0 );
    const Vector2D p3( -10.0, -10.0 );
    const Vector2D p4( +10.0, -10.0 );

    const Vector2D p5( +20.0,   0.0 );
    const Vector2D p6(   0.0, +20.0 );
    const Vector2D p7( -20.0,   0.0 );
    const Vector2D p8(   0.0, -20.0 );

    //
    // input points
    //

    //                               //
    //                 |             //
    // +20             *p6           //
    //                 |             //
    //                 |             //
    //           p2    |    p1       //
    // +10        *    |    *        //
    //                 |             //
    //                 |             //
    //                 |             //
    //      p7         |             //
    //   0 --*---------*---------*-- //
    //                 |p0       p5  //
    //                 |             //
    //                 |             //
    //                 |             //
    // -10        *    |    *        //
    //           p3    |    p4       //
    //                 |             //
    //                 |             //
    //                 |             //
    // -20             *p8           //
    //                 |             //
    //                               //
    //      -20  -10   0   +10  +20  //


    std::cerr << "\ninput points=\n "
              << p0 << "\n "
              << p1 << "\n "
              << p2 << "\n "
              << p3 << "\n "
              << p4 << "\n "
              << p5 << "\n "
              << p6 << "\n "
              << p7 << "\n "
              << p8 << std::endl;

    VoronoiDiagram v;

    v.addPoint( p0 );
    v.addPoint( p1 );
    v.addPoint( p2 );
    v.addPoint( p3 );
    v.addPoint( p4 );
    v.addPoint( p5 );
    v.addPoint( p6 );
    v.addPoint( p7 );
    v.addPoint( p8 );

    v.compute();


    //
    // result
    //

    //           \           /        //
    //            \    |    /         //
    // +20         \   *p6 /          //
    //              \  |  /           //
    //     \         \ | /         /  //
    //      \     p2  \|/   p1    /   //
    // +10   \     *   .    *    /    //
    //        \       /|\       /     //
    //         \     / | \     /      //
    //          \   /  |  \   /       //
    //      p7   \ /   |   \ /        //
    //   0 --*----.----*----.----*--  //
    //           / \   |p0 / \    p5  //
    //          /   \  |  /   \       //
    //         /     \ | /     \      //
    //        /       \|/       \     //
    // -10   /    *    .    *    \    //
    //      /    p3   /|\    p4   \   //
    //     /         / | \         \  //
    //              /  |  \           //
    //             /   |   \          //
    // -20        /    *p8  \         //
    //           /     |     \        //
    //          /             \       //
    //      -20  -10   0   +10  +20   //


    //
    // check points
    //
    int n_points = 0;
    for ( VoronoiDiagram::Vector2DCont::const_iterator p = v.vertices().begin(),
                  end = v.vertices().end();
          p != end;
          ++p )
    {
        n_points ++;

    }


    //
    // check segments
    //
    int n_segments = 0;
    for ( VoronoiDiagram::Segment2DCont::const_iterator s = v.segments().begin(),
                  end = v.segments().end();
          s != end;
          ++s )
    {
        n_segments ++;

    }
    ASSERT_EQ( 4, n_segments );


    //
    // check rays
    //
    int n_rays = 0;
    for ( VoronoiDiagram::Ray2DCont::const_iterator r = v.rays().begin(),
                  end = v.rays().end();
          r != end;
          ++r )
    {
        n_rays ++;
    }
    ASSERT_EQ( 8, n_rays );
}


#endif //PARSIAN_UTIL_TEST_VORONOI_DIAGRAM_H
*/