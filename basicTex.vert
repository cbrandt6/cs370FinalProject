#version 400 core
uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform mat4 cam_matrix;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;

out vec4 Position;
out vec2 texCoord;

void main( )
{
    // Compute transformed vertex position in view space
    gl_Position = proj_matrix*(cam_matrix*(model_matrix*vPosition));

    // TODO: Pass texture coordinate to frag shader
    texCoord = vTexCoord;
}
