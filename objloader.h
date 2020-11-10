#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vmath.h"


bool loadOBJ(
	const char * path, 
	std::vector<vmath::vec4> & out_vertices,
	std::vector<vmath::vec2> & out_uvs,
	std::vector<vmath::vec3> & out_normals
);

#endif