byte NONE    = 0
byte VECTOR  = 1
byte SEGMENT = 2
byte RECT    = 3
byte CIRCLE  = 4
byte POLYGON = 5
byte TEXT    = 6

byte type

std_msgs/ColorRGBA color
bool filled
vector2D primary # {circle-center, vector pos, text-pos, line-start, rect-topleft}
vector2D secondary # {line-end, rect-botright, start-ang&end-ang of circle}
string text
float64 size # {circle-radius, text-size}
vector2D[] polygon

