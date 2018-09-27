//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    field_default_constants.h
  \brief   Definition of field dimensions
  \author  Joydeep Biswas, (C) 2013
*/
//========================================================================

#include <rqt_parsian_gui/graphical/field_default_constants.h>
#include <parsian_util/core/field.h>
#include <math.h>

namespace FieldConstants {

const std::size_t kNumFieldLines = 18;
const FieldLine kFieldLines[kNumFieldLines] = {
    FieldLine("TopTouchLine", -kFieldLength/2, kFieldWidth/2, kFieldLength/2, kFieldWidth/2, kLineThickness),
    FieldLine("BottomTouchLine", -kFieldLength/2, -kFieldWidth/2, kFieldLength/2, -kFieldWidth/2, kLineThickness),
    FieldLine("LeftGoalLine", -kFieldLength/2, -kFieldWidth/2, -kFieldLength/2, kFieldWidth/2, kLineThickness),
    FieldLine("RightGoalLine", kFieldLength/2, -kFieldWidth/2, kFieldLength/2, kFieldWidth/2, kLineThickness),
    FieldLine("HalfwayLine", 0, -kFieldWidth/2, 0, kFieldWidth/2, kLineThickness),
    FieldLine("CenterLine", -kFieldLength/2, 0, kFieldLength/2, 0, kLineThickness),

    FieldLine("LeftPenaltyStretchU", -kFieldLength/2, kPenaltyAreaWidth/2, -kFieldLength/2 + kPenaltyAreaDepth, kPenaltyAreaWidth/2, kLineThickness),
    FieldLine("LeftPenaltyStretchM", -kFieldLength/2 + kPenaltyAreaDepth, -kPenaltyAreaWidth/2, -kFieldLength/2 + kPenaltyAreaDepth, kPenaltyAreaWidth/2, kLineThickness),
    FieldLine("LeftPenaltyStretchB", -kFieldLength/2, -kPenaltyAreaWidth/2, -kFieldLength/2 + kPenaltyAreaDepth, -kPenaltyAreaWidth/2, kLineThickness),

    FieldLine("RightPenaltyStretchU", kFieldLength/2 - kPenaltyAreaDepth, kPenaltyAreaWidth/2, kFieldLength/2, kPenaltyAreaWidth/2, kLineThickness),
    FieldLine("RightPenaltyStretchM", kFieldLength/2 - kPenaltyAreaDepth, -kPenaltyAreaWidth/2, kFieldLength/2 - kPenaltyAreaDepth, kPenaltyAreaWidth/2, kLineThickness),
    FieldLine("RightPenaltyStretchB", kFieldLength/2 - kPenaltyAreaDepth, -kPenaltyAreaWidth/2, kFieldLength/2, -kPenaltyAreaWidth/2, kLineThickness),

    FieldLine("LeftGoalU", -kFieldLength/2 - kGoalDepth, kGoalWidth/2, -kFieldLength/2, kGoalWidth/2, kLineThickness),
    FieldLine("LeftGoalM", -kFieldLength/2 - kGoalDepth, kGoalWidth/2, -kFieldLength/2 - kGoalDepth, -kGoalWidth/2, kLineThickness),
    FieldLine("LeftGoalB", -kFieldLength/2 - kGoalDepth, -kGoalWidth/2, -kFieldLength/2, -kGoalWidth/2, kLineThickness),
    FieldLine("RightGoalU", kFieldLength/2, kGoalWidth/2, kFieldLength/2 + kGoalDepth, kGoalWidth/2, kLineThickness),
    FieldLine("RightGoalM", kFieldLength/2 + kGoalDepth, kGoalWidth/2, kFieldLength/2 + kGoalDepth, -kGoalWidth/2, kLineThickness),
    FieldLine("RightGoalB", kFieldLength/2, -kGoalWidth/2, kFieldLength/2 + kGoalDepth, -kGoalWidth/2, kLineThickness)

};

const std::size_t kNumFieldArcs = 1;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
    FieldCircularArc("CenterCircle", 0, 0, kCenterCircleRadius, 0, 2.0 * M_PI, kLineThickness)
    };

}  // namespace FieldConstantsRoboCup2012
