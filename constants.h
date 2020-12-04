/*
 * File to hold constant values
 */
#include "vmath.h"

//Walls
#define wall_width 1.0f
#define wall_height 10.0f
#define short_wall_length 25.0f
#define long_wall_length 40.0f
vmath::vec4 wall_color = {0.5f, 0.5f, 0.5f, 1.0f};
vmath::vec4 floor_color = {0.3f, 0.2f, 0.1f, 1.0f};

//Front wall
#define wall_split_size (long_wall_length/2.0f)-(wall_height/4.0f)+2.0f)
#define wall_split_loc_l (long_wall_length/2.0f)+2.0f
#define wall_split_loc_r -(long_wall_length/2.0f)-2.0f

//Table
#define table_leg_width 0.25f
#define table_leg_height 2.5f
#define table_top_height 1.0f
#define table_top_width 2.0f
#define table_top_length 3.0f
vmath::vec4 table_color = {0.5f, 0.2f, 0.1f, 1.0f};

//Chairs
#define chair_height table_leg_height*0.66f
#define chair_width 1.2f
vmath::vec4 chair_color = {1.0f, 0.0f, 1.0f, 1.0f};

//Soda
#define soda_width 0.25f
#define soda_height 0.32f
#define soda_loc_height (table_leg_height + table_top_height)+1.8f
vmath::vec4 soda_color = {1.0f, 0.0f, 0.0f, 1.0f};

//Window/Mirror, they are the same size
#define window_height wall_height*0.66f
#define window_length short_wall_length*0.66f
#define window_width wall_width

//Light switch
#define switch_width 0.1f
#define switch_height 0.15f
#define switch_loc_height wall_height/2.0f
vmath::vec4 switch_color = {1.0f, 1.0f, 0.0f, 1.0f};

//Art piece
#define art_height wall_height*0.66f
#define art_width wall_width
#define art_length long_wall_length*0.66f
vmath::vec4 art_color = {0.0f, 0.9f, 1.0f, 1.0f};

//Fan
#define fan_width 0.2f
#define fan_height 0.2f
#define fan_depth 0.2f
vmath::vec3 fan_loc = {0.0f, 21.0f, 0.0f};