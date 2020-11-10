#version 400 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform mat4 cam_matrix;
uniform mat4 norm_matrix;

uniform vec3 EyePosition;
out vec4 Position;
out vec3 Normal;
out vec3 View;

void main( )
{
    // Compute transformed vertex position in view space
    gl_Position = proj_matrix*(cam_matrix*(model_matrix*vPosition));

    // Compute n (transformed by normal matrix) (passed to fragment shader)
    Normal = vec3(normalize(norm_matrix * normalize(vec4(vNormal,0.0f))));

    // Compute vertex position in world coordinates (passed to fragment shader)
    Position = model_matrix*vPosition;

    // Compute v (camera location - transformed vertex) (passed to fragment shader)
    View = normalize(EyePosition - Position.xyz);

}
