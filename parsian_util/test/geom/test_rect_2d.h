//
// Created by raziyeh on 3/29/19.
//

#ifndef PARSIAN_UTIL_TEST_RECT_2D_H
#define PARSIAN_UTIL_TEST_RECT_2D_H

#include <parsian_util/geom/rect_2d.h>
#include "config.h"

#include <gtest/gtest.h>
#include <ros/ros.h>

using namespace rcsc;

void testSet()
{
    rcsc::Rect2D r( rcsc::Vector2D( 0.0, 0.0 ),
                    rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r1 = r;

    r1.setTopLeft( -5.0, -5.0 );
    EXPECT_NEAR( -5.0, r1.left(), EPS );
    EXPECT_NEAR( 5.0, r1.right(), EPS );
    EXPECT_NEAR( -5.0, r1.top(), EPS );
    EXPECT_NEAR( -15.0, r1.bottom(), EPS );
    EXPECT_NEAR( 10.0, r1.size().length(), EPS );
    EXPECT_NEAR( 10.0, r1.size().width(), EPS );

}

#endif //PARSIAN_UTIL_TEST_RECT_2D_H
