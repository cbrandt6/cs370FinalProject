// CS370 Final Project
// Fall 2020

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "vgl.h"
#include "objloader.h"
#include "utils.h"
#include "vmath.h"
#include "lighting.h"
#include "constants.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

enum VAO_IDs {Cube, NumVAOs};
enum Buffer_IDs {CubePosBuffer, NumBuffers};

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

GLint numVertices[NumVAOs];
GLint posCoords = 4;
vec4 cube_color = {1.0f, 0.0f, 0.0f,1.0f};

// Camera
vec3 eye = {3.0f, 3.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
GLuint program;
GLuint vPos;
GLuint vCol;
GLuint model_mat_loc;
GLuint proj_mat_loc;
GLuint cam_mat_loc;
const char *vertex_shader = "../color_mesh.vert";
const char *frag_shader = "../color_mesh.frag";

// Global state
const char *models[NumVAOs] = {"../models/cube.obj"};
mat4 proj_matrix;
mat4 camera_matrix;
vec3 axis = {0.0f, 1.0f, 0.0f};

// Global spherical coord values
GLfloat azimuth = 0.0f;
GLfloat daz = 10.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 3.0f;
GLfloat dr = 10.0f;
GLfloat min_radius = 2.0f;

//"Walking" values

GLfloat delta = 0.0f;
GLfloat step = 3.0f;
vec3 dir = vec3(0.0f, 0.0f, 0.0f);
vec3 gaze = vec3(5.0f, 6.0f, 0.0f);

// Global screen dimensions
GLint ww,hh;

void build_geometry( );
void display( );
void render_scene( );
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void draw_obj(GLuint VAO, GLuint Buffer, int numVert);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

	// Create geometry buffers
    build_geometry();
    
    // Load shaders and associate shader variables
	ShaderInfo shaders[] = { {GL_VERTEX_SHADER, vertex_shader},{GL_FRAGMENT_SHADER, frag_shader},{GL_NONE, NULL} };
	program = LoadShaders(shaders);
    vPos = glGetAttribLocation(program, "vPosition");
    vCol = glGetUniformLocation(program, "vColor");
    model_mat_loc = glGetUniformLocation(program, "model_matrix");
    proj_mat_loc = glGetUniformLocation(program, "proj_matrix");
    cam_mat_loc = glGetUniformLocation(program, "camera_matrix");

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    eye = vec3(x, y, z);
    eye = vec3(0.0f, 6.0f, 0.0f);
    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;
}

void build_geometry( )
{
    // Model vectors
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);
    glGenBuffers(NumBuffers, Buffers);

    // Bind cube vertex array
    glBindVertexArray(VAOs[Cube]);

    // TODO: Load cube model and store number of vertices
    loadOBJ(models[Cube], vertices, uvCoords, normals);
    numVertices[Cube] = vertices.size();


    // Bind cube vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[CubePosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[Cube], vertices.data(), GL_STATIC_DRAW);

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // TODO: Set perspective projection matrix
    //GLfloat world_size = 70.0f;
    //proj_matrix = ortho(-world_size*xratio, world_size*xratio, -world_size*yratio, world_size*yratio, -world_size*2.0f, world_size);
    GLfloat perspective_size = 1.0f;
    GLfloat near = 1.0f;
    GLfloat far = 70.0f;
    proj_matrix = frustum(-perspective_size * xratio, perspective_size * xratio , -perspective_size * yratio, perspective_size * yratio, near, far);
    // TODO: Set camera matrix
    camera_matrix = lookat(eye, gaze, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    mat4 model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Select shader program
    glUseProgram(program);
    // Pass projection matrix to shader
    glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, proj_matrix);
    // Pass camera matrix to shader
    glUniformMatrix4fv(cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Set cube transformation matrix
//    trans_matrix = translate(0.0f, 0.0f, 0.0f);
//    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
//    scale_matrix = scale(1.0f, 1.0f, 1.0f);
//	model_matrix = trans_matrix*rot_matrix*scale_matrix;
//	glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
//	glUniform4fv(vCol, 1, cube_color);

    //Draw floor
    scale_matrix = scale(long_wall_length, wall_width, short_wall_length);
    model_matrix = scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, floor_color);
	draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

	//Draw left wall
    scale_matrix = scale(wall_width, wall_height, short_wall_length);
    trans_matrix = translate(-long_wall_length, wall_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw right wall
    scale_matrix = scale(wall_width, wall_height, short_wall_length);
    trans_matrix = translate(long_wall_length, wall_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw back wall
    scale_matrix = scale(long_wall_length, wall_height, wall_width);
    trans_matrix = translate(0.0f, wall_height, -short_wall_length);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Front wall
    scale_matrix = scale((wall_split_size, wall_height, wall_width);
    trans_matrix = translate(wall_split_loc_l, wall_height, short_wall_length);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    scale_matrix = scale((wall_split_size, wall_height, wall_width);
    trans_matrix = translate(wall_split_loc_r, wall_height, short_wall_length);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    scale_matrix = scale(wall_height/4.0f, wall_height/2.0f, wall_width);
    trans_matrix = translate(0.0f, wall_height+(wall_height/2.0f), short_wall_length);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, wall_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw door
    scale_matrix = scale(wall_height/4.0f, wall_height/2.0f, wall_width);
    trans_matrix = translate(0.0f, wall_height/2.0f, short_wall_length);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, floor_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw window
    scale_matrix = scale(window_width, window_height, window_length);
    trans_matrix = translate(long_wall_length-1.0f, wall_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, vec4(0.0f, 0.0f, 1.0f, 1.0f));
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw mirror
    scale_matrix = scale(wall_width, wall_height*0.66f, short_wall_length*0.66f);
    trans_matrix = translate(-long_wall_length+1.0f, wall_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, vec4(0.0f, 0.0f, 1.0f, 1.0f));
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw art
    scale_matrix = scale(art_length, art_height, art_width);
    trans_matrix = translate(0.0f, wall_height, -short_wall_length+1.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, art_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw table
    scale_matrix = scale(table_top_length, table_leg_height, table_top_width);
    trans_matrix = translate(0.0f, table_leg_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, table_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Chairs
    scale_matrix = scale(chair_width, chair_height, chair_width);
    trans_matrix = translate(-table_top_length-1.0f, chair_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, chair_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    scale_matrix = scale(chair_width, chair_height, chair_width);
    trans_matrix = translate(table_top_length+1.0f, chair_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, chair_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    scale_matrix = scale(chair_width, chair_height, chair_width);
    trans_matrix = translate(0.0f, chair_height, -table_top_width-1.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, chair_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    scale_matrix = scale(chair_width, chair_height, chair_width);
    trans_matrix = translate(0.0f, chair_height, table_top_width+1.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, chair_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);

    //Draw soda
    scale_matrix = scale(soda_width, soda_height, soda_width);
    trans_matrix = translate(0.0f, soda_loc_height, 0.0f);
    model_matrix = trans_matrix * scale_matrix;
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniform4fv(vCol, 1, soda_color);
    draw_obj(VAOs[Cube], Buffers[CubePosBuffer], numVertices[Cube]);


}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Adjust azimuth
    if (key == GLFW_KEY_D) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }

    } else if (key == GLFW_KEY_A) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }

    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        delta = -1.0f;
        dir = vec3(eye[0] - gaze[0], 0.0f, eye[2] - gaze[2]);
        eye = vec3(eye[0] + dir[0] * delta, eye[1], eye[2] + dir[2] * delta);

    }
    else if (key == GLFW_KEY_S)
    {
        delta = 1.0f;
        dir = vec3(eye[0] - gaze[0], 0.0f, eye[2] - gaze[2]);
        eye = vec3(eye[0] + dir[0] * delta, eye[1], eye[2] + dir[2] * delta);

    }

    // Adjust elevation
    //TODO Ensure proper functionality of looking up and down
    if (key == GLFW_KEY_X)
    {
        radius -= dr;

    }
    else if (key == GLFW_KEY_Z)
    {
        radius += dr;

    }

    // Compute updated camera position
    gaze = vec3(eye[0] + cos(azimuth*DEG2RAD), eye[1] + sin(radius*DEG2RAD), eye[2] + sin(azimuth*DEG2RAD));



}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

void draw_obj(GLuint VAO, GLuint Buffer, int numVert) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Buffer);
    glVertexAttribPointer(vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(vPos);
    glDrawArrays(GL_TRIANGLES, 0, numVert);
}
