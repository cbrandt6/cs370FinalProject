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
#include "MatNode.h"
#include "TexNode.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

enum VAO_IDs {Cube, Sphere, Torus, Cone, Cylinder, Fan, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Brass, Floor, Wall, Glass, RedAcrylic, RedPlastic,NumMaterials};
enum Buffer_IDs {CubePosBuffer, NumBuffers};
enum Textures {NumTextures};

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

GLint numVertices[NumVAOs];
GLint posCoords = 4;
GLint normCoords = 3;
//vec4 cube_color = {1.0f, 0.0f, 0.0f,1.0f};
const char *objFiles[NumVAOs] = {"../models/cube.obj", "../models/sphere.obj", "../models/torus.obj", "../models/cone.obj", "../models/cylinder.obj", "../models/CeilingFan.obj"};
const char *texFiles = {};
// Camera
vec3 eye = {3.0f, 3.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Lighting Shader variables
GLuint light_program;
GLuint light_vPos;
GLuint light_vNorm;
GLuint light_camera_mat_loc;
GLuint light_model_mat_loc;
GLuint light_proj_mat_loc;
GLuint light_norm_mat_loc;
GLuint lights_block_idx;
GLuint materials_block_idx;
GLuint num_lights_loc;
GLuint material_loc;
GLuint light_eye_loc;
GLuint light_on_loc;
const char *light_vertex_shader = "../phong.vert";
const char *light_frag_shader = "../phong.frag";

// Shader variables
GLuint program;
GLuint vNorm;
GLuint vPos;
GLuint vCol;
GLuint model_mat_loc;
GLuint proj_mat_loc;
GLuint cam_mat_loc;

//Mirror Stuff
//mat4 mirror_proj_matrix;
//mat4 mirror_camera_matrix;

// Texture shader variables
GLuint tex_program;
GLuint tex_proj_mat_loc;
GLuint tex_camera_mat_loc;
GLuint tex_model_mat_loc;
GLuint tex_vPos;
GLuint tex_vTex;

const char *vertex_shader = "../color_mesh.vert";
const char *frag_shader = "../color_mesh.frag";
const char *tex_vertex_shader = "../basicTex.vert";
const char *tex_frag_shader = "../basicTex.frag";

// Global state
//const char *models[NumVAOs] = {"../models/cube.obj"};
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint MaterialIdx[NumMaterials] = {Brass, Floor, Wall, Glass, RedAcrylic};
GLuint numLights;
GLint lightOn[2] = {1,0};
vec3 axis = {0.0f, 1.0f, 0.0f};

// Scene Graph Nodes
MatNode floor_node;
MatNode left_wall;
MatNode right_wall;
MatNode back_wall;
MatNode front_wall_1;
MatNode front_wall_2;
MatNode front_wall_3;
MatNode door;
MatNode table;
MatNode chair_1;
MatNode chair_2;
MatNode chair_3;
MatNode chair_4;
MatNode mirror;
MatNode art;
MatNode soda;
MatNode fan;
MatNode window;

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
void build_lights( );
void build_materials( );
void build_textures( );
void display( );
void render_scene( );
void build_scene_graph( );
void traverse_scene_graph(BaseNode *node, mat4 baseTransform);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void load_object(GLuint obj);
void draw_object(GLuint obj);
void load_tex_object(GLuint obj);
void draw_tex_object(GLuint obj);

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


    
    // Load shaders and associate shader variables
	//ShaderInfo shaders[] = { {GL_VERTEX_SHADER, vertex_shader},{GL_FRAGMENT_SHADER, frag_shader},{GL_NONE, NULL} };
	ShaderInfo light_shaders[] = { {GL_VERTEX_SHADER, light_vertex_shader},{GL_FRAGMENT_SHADER, light_frag_shader},{GL_NONE, NULL} };
	light_program = LoadShaders(light_shaders);
//	program = LoadShaders(shaders);
//    //mesh shader variables
//	vPos = glGetAttribLocation(program, "vPosition");
//    vCol = glGetUniformLocation(program, "vColor");
//    model_mat_loc = glGetUniformLocation(program, "model_matrix");
//    proj_mat_loc = glGetUniformLocation(program, "proj_matrix");
//    cam_mat_loc = glGetUniformLocation(program, "camera_matrix");

    //Light shader variables
    light_vPos = glGetAttribLocation(light_program, "vPosition");
    light_vNorm = glGetAttribLocation(light_program, "vNormal");
    light_camera_mat_loc = glGetUniformLocation(light_program, "cam_matrix");
    light_model_mat_loc = glGetUniformLocation(light_program, "model_matrix");
    light_proj_mat_loc = glGetUniformLocation(light_program, "proj_matrix");
    light_norm_mat_loc = glGetUniformLocation(light_program, "norm_matrix");
    lights_block_idx = glGetUniformBlockIndex(light_program, "LightBuffer");
    materials_block_idx = glGetUniformBlockIndex(light_program, "MaterialBuffer");
    material_loc = glGetUniformLocation(light_program, "Material");
    num_lights_loc = glGetUniformLocation(light_program, "NumLights");
    light_eye_loc = glGetUniformLocation(light_program, "EyePosition");
    light_on_loc = glGetUniformLocation(light_program, "LightOn");

    // Load shadow shader
    ShaderInfo tex_shaders[] = { {GL_VERTEX_SHADER, tex_vertex_shader},{GL_FRAGMENT_SHADER, tex_frag_shader},{GL_NONE, NULL} };
    tex_program = LoadShaders(tex_shaders);

    // Select shader program and associate shader variables
    tex_vPos = glGetAttribLocation(tex_program, "vPosition");
    tex_vTex = glGetAttribLocation(tex_program, "vTexCoord");
    tex_model_mat_loc = glGetUniformLocation(tex_program, "model_matrix");
    tex_proj_mat_loc = glGetUniformLocation(tex_program, "proj_matrix");
    tex_camera_mat_loc = glGetUniformLocation(tex_program, "cam_matrix");

    // Create geometry buffers
    build_geometry();
    // Create light buffers
    build_lights();
    // Create material buffers
    build_materials();
    // Build scene graph
    build_scene_graph();

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //Enable Transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);
    load_object(Cube);
    load_object(Cylinder);
    load_object(Fan);

}

void build_lights( ) {
    // White directional light
    LightProperties whitePointLight = {POINT,
                                       {0.0f, 0.0f, 0.0f}, //pad
                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //specular
                                       vec4(0.0f, 20.0f, 0.0f, 1.0f),  //position
                                       vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
                                       0.0f,
                                       0.0f,
                                       {0.0f, 0.0f} //pad
    };

    LightProperties lessBrightLight = {POINT,
                                       {0.0f, 0.0f, 0.0f}, //pad
                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
                                       vec4(0.2f, 0.2f, 0.2f, 1.0f), //diffuse
                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //specular
                                       vec4(0.0f, 20.0f, 0.0f, 1.0f),  //position
                                       vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
                                       0.0f,
                                       0.0f,
                                       {0.0f, 0.0f} //pad
    };


//    // Green point light
//    LightProperties greenPointLight = {POINT, //type
//                                       {0.0f, 0.0f, 0.0f}, //pad
//                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
//                                       vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
//                                       vec4(0.0f, 1.0f, 0.0f, 1.0f), //specular
//                                       vec4(3.0f, 3.0f, 3.0f, 1.0f),  //position
//                                       vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
//                                       0.0f,   //cutoff
//                                       0.0f,  //exponent
//                                       {0.0f, 0.0f}  //pad2
//    };
//
//    //Red spot light
//    LightProperties redSpotLight = {SPOT, //type
//                                    {0.0f, 0.0f, 0.0f}, //pad
//                                    vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
//                                    vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
//                                    vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
//                                    vec4(0.0f, 6.0f, 0.0f, 1.0f),  //position
//                                    vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
//                                    30.0f,   //cutoff
//                                    30.0f,  //exponent
//                                    {0.0f, 0.0f}  //pad2
//    };


    Lights.push_back(whitePointLight);
    Lights.push_back(lessBrightLight);
//    Lights.push_back(greenPointLight);
//    Lights.push_back(redSpotLight);

    numLights = Lights.size();

    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_materials( ) {
    // Create brass material
    MaterialProperties brass = {vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
                                vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
                                vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
                                27.8f, //shininess
                                {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create red plastic material
    MaterialProperties redPlastic = {vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
                                     vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
                                     vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
                                     32.0f, //shininess
                                     {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties floor = {vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
                                     vec4(0.3f, 0.2f, 0.1f, 1.0f), //diffuse
                                     vec4(0.3f, 0.2f, 0.1f, 1.0f), //specular
                                     32.0f, //shininess
                                     {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties wall = {vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
                                vec4(0.5f, 0.5f, 0.5f, 1.0f), //diffuse
                                vec4(0.5f, 0.5f, 0.5f, 1.0f), //specular
                                32.0f, //shininess
                                {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties glass = {vec4(0.0f, 0.0f, 0.0f, 0.6f), //ambient
                                     vec4(0.0f, 0.3f, 0.3f, 0.6f), //diffuse
                                     vec4(0.0f, 0.25f, 0.25f, 0.6f), //specular
                                     45.0f, //shininess
                                     {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties redAcrylic = {vec4(0.3f, 0.0f, 0.0f, 0.5f), //ambient
                                     vec4(0.6f, 0.0f, 0.0f, 0.5f), //diffuse
                                     vec4(0.8f, 0.6f, 0.6f, 0.5f), //specular
                                     32.0f, //shininess
                                     {0.0f, 0.0f, 0.0f}  //pad
    };
    // Add materials to Materials vector
    Materials.push_back(brass);
    Materials.push_back(floor);
    Materials.push_back(wall);
    Materials.push_back(glass);
    Materials.push_back(redAcrylic);
    Materials.push_back(redPlastic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

//void build_textures( ) {
//    int w, h, n;
//    int force_channels = 4;
//    unsigned char *image_data;
//
//    // Create textures and activate unit 0
//    glGenTextures( NumTextures,  TextureIDs);
//    glActiveTexture( GL_TEXTURE0 );
//
//    for (int i = 0; i < NumTextures; i++) {
//        // Load image from file
//        image_data = stbi_load(texFiles[i], &w, &h, &n, force_channels);
//        if (!image_data) {
//            fprintf(stderr, "ERROR: could not load %s\n", texFiles[i]);
//        }
//        // NPOT check for power of 2 dimensions
//        if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
//            fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
//                    texFiles[i]);
//        }
//        int width_in_bytes = w * 4;
//        unsigned char *top = NULL;
//        unsigned char *bottom = NULL;
//        unsigned char temp = 0;
//        int half_height = h / 2;
//
//        for ( int row = 0; row < half_height; row++ ) {
//            top = image_data + row * width_in_bytes;
//            bottom = image_data + ( h - row - 1 ) * width_in_bytes;
//            for ( int col = 0; col < width_in_bytes; col++ ) {
//                temp = *top;
//                *top = *bottom;
//                *bottom = temp;
//                top++;
//                bottom++;
//            }
//        }
//
//        // TODO: Bind current texture id
//        glBindTexture(GL_TEXTURE_2D, TextureIDs[i]);
//        // TODO: Load image data into texture
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
//                     image_data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//        // TODO: Set scaling modes
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        // TODO: Set wrapping modes
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        // Set maximum anisotropic filtering for system
//        GLfloat max_aniso = 0.0f;
//        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
//        // set the maximum!
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
//    }
//}

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
    //Set number of light
    glUniform1iv(light_on_loc, numLights, lightOn);
    traverse_scene_graph(&floor_node, mat4().identity());
//    // Declare transformation matrices
//    mat4 model_matrix = mat4().identity();
//    mat4 scale_matrix = mat4().identity();
//    mat4 rot_matrix = mat4().identity();
//    mat4 trans_matrix = mat4().identity();
//
//    // Select shader program
//    glUseProgram(light_program);
//    // Pass projection matrix to shader
//    glUniformMatrix4fv(light_proj_mat_loc, 1, GL_FALSE, proj_matrix);
//    // Pass camera matrix to shader
//    glUniformMatrix4fv(light_camera_mat_loc, 1, GL_FALSE, camera_matrix);
//
//    // Bind lights
//    glUniformBlockBinding(light_program, lights_block_idx, 0);
//    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));
//    // Bind materials
//    glUniformBlockBinding(light_program, materials_block_idx, 1);
//    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));
//    // Set num lights
//    glUniform1i(num_lights_loc, numLights);
//    glUniform1iv(light_on_loc, numLights, lightOn);
//
//    //Draw floor
//    scale_matrix = scale(long_wall_length, wall_width, short_wall_length);
//    model_matrix = scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//	draw_object(Cube);
//
//	//Draw left wall
//    scale_matrix = scale(wall_width, wall_height, short_wall_length);
//    trans_matrix = translate(-long_wall_length, wall_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    //Draw right wall
//    scale_matrix = scale(wall_width, wall_height, short_wall_length);
//    trans_matrix = translate(long_wall_length, wall_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    //Draw back wall
//    scale_matrix = scale(long_wall_length, wall_height, wall_width);
//    trans_matrix = translate(0.0f, wall_height, -short_wall_length);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    //Front wall
//    scale_matrix = scale((wall_split_size, wall_height, wall_width);
//    trans_matrix = translate(wall_split_loc_l, wall_height, short_wall_length);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    scale_matrix = scale((wall_split_size, wall_height, wall_width);
//    trans_matrix = translate(wall_split_loc_r, wall_height, short_wall_length);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    scale_matrix = scale(wall_height/4.0f, wall_height/2.0f, wall_width);
//    trans_matrix = translate(0.0f, wall_height+(wall_height/2.0f), short_wall_length);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Wall]);
//    draw_object(Cube);
//
//    //Draw door
//    scale_matrix = scale(wall_height/4.0f, wall_height/2.0f, wall_width);
//    trans_matrix = translate(0.0f, wall_height/2.0f, short_wall_length);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    //Draw table
//    scale_matrix = scale(table_top_length, table_leg_height, table_top_width);
//    trans_matrix = translate(0.0f, table_leg_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    //Chairs
//    scale_matrix = scale(chair_width, chair_height, chair_width);
//    trans_matrix = translate(-table_top_length-1.0f, chair_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    scale_matrix = scale(chair_width, chair_height, chair_width);
//    trans_matrix = translate(table_top_length+1.0f, chair_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    scale_matrix = scale(chair_width, chair_height, chair_width);
//    trans_matrix = translate(0.0f, chair_height, -table_top_width-1.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    scale_matrix = scale(chair_width, chair_height, chair_width);
//    trans_matrix = translate(0.0f, chair_height, table_top_width+1.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    //Draw mirror
//    scale_matrix = scale(wall_width, wall_height*0.66f, short_wall_length*0.66f);
//    trans_matrix = translate(-long_wall_length+1.0f, wall_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    //Draw art
//    scale_matrix = scale(art_length, art_height, art_width);
//    trans_matrix = translate(0.0f, wall_height, -short_wall_length+1.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Cube);
//
//    //Draw soda
//    scale_matrix = scale(soda_width, soda_height, soda_width);
//    trans_matrix = translate(0.0f, soda_loc_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[RedPlastic]);
//    draw_object(Cylinder);
//
//    //Draw Fan
//    scale_matrix = scale(fan_width, fan_height, fan_depth);
//    trans_matrix = translate(fan_loc);
//    //rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Floor]);
//    draw_object(Fan);
//
//    // Translucent Objects
//    //Draw window
//    scale_matrix = scale(window_width, window_height, window_length);
//    trans_matrix = translate(long_wall_length-1.0f, wall_height, 0.0f);
//    model_matrix = trans_matrix * scale_matrix;
//    normal_matrix = model_matrix.inverse().transpose();
//    glUniformMatrix4fv(light_model_mat_loc, 1, GL_FALSE, model_matrix);
//    glUniformMatrix4fv(light_norm_mat_loc, 1, GL_FALSE, normal_matrix);
//    glUniform1i(material_loc, MaterialIdx[Glass]);
//    glDepthMask(GL_FALSE);
//    draw_object(Cube);
//    glDepthMask(GL_TRUE);





}

void traverse_scene_graph(BaseNode *node, mat4 baseTransform) {
    mat4 model_matrix;

    // Stop when at bottom of branch
    if (node == NULL) {
        return;
    }

    // Apply local transformation and render
    model_matrix = baseTransform*node->ModelTransform;

    node->draw(proj_matrix, camera_matrix, model_matrix);

    // Recurse vertically if possible (depth-first)
    if (node->child != NULL) {
        traverse_scene_graph(node->child, model_matrix);
    }

    // Remove local transformation and recurse horizontal
    if (node->sibling != NULL) {
        traverse_scene_graph(node->sibling, baseTransform);
    }
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

    //Toggle which light is on
    if(key == GLFW_KEY_L && action == GLFW_PRESS){
        lightOn[0] = 1 - lightOn[0];
        lightOn[1] = 1 - lightOn[1];
    }

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

void load_object(GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    loadOBJ(objFiles[obj], vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();
    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_object(GLuint obj){
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(light_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(light_vPos);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(light_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(light_vNorm);
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);

}

void build_scene_graph( ) {

    //Floor node
    floor_node.set_shader(light_program, light_proj_mat_loc, light_camera_mat_loc, light_model_mat_loc);
    floor_node.NormMatPtr = light_norm_mat_loc;
    floor_node.set_buffers(VAOs[Cube], ObjBuffers[Cube][PosBuffer], light_vPos, posCoords, ObjBuffers[Cube][NormBuffer], light_vNorm, normCoords, numVertices[Cube]);
    floor_node.set_materials(MaterialBuffers[MaterialBuffer], materials_block_idx, Materials.size()*sizeof(MaterialProperties), material_loc, MaterialIdx[Floor]);
    floor_node.set_lights(LightBuffers[LightBuffer], lights_block_idx, Lights.size()*sizeof(LightProperties), num_lights_loc, Lights.size());
    floor_node.set_eye(light_eye_loc, eye);
    floor_node.set_base_transform(scale(vec3(long_wall_length, wall_width, short_wall_length)));
    floor_node.sibling = &left_wall;
    floor_node.child = NULL;

    //Left Wall
    left_wall.set_shader(light_program, light_proj_mat_loc, light_camera_mat_loc, light_model_mat_loc);
    left_wall.NormMatPtr = light_norm_mat_loc;
    left_wall.set_buffers(VAOs[Cube], ObjBuffers[Cube][PosBuffer], light_vPos, posCoords, ObjBuffers[Cube][NormBuffer], light_vNorm, normCoords, numVertices[Cube]);
    left_wall.set_materials(MaterialBuffers[MaterialBuffer], materials_block_idx, Materials.size()*sizeof(MaterialProperties), material_loc, MaterialIdx[Wall]);
    left_wall.set_lights(LightBuffers[LightBuffer], lights_block_idx, Lights.size()*sizeof(LightProperties), num_lights_loc, Lights.size());
    left_wall.set_eye(light_eye_loc, eye);
    left_wall.set_base_transform(translate(-long_wall_length, wall_height, 0.0f) * scale(vec3(wall_width, wall_height, short_wall_length)));
    left_wall.sibling = NULL;
    left_wall.child = NULL;
}

//void load_tex_object(GLuint obj) {
//    vector<vec4> vertices;
//    vector<vec2> uvCoords;
//    vector<vec3> normals;
//
//    loadOBJ(objFiles[obj], vertices, uvCoords, normals);
//    numVertices[obj] = vertices.size();
//    // Create and load object buffers
//    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
//    glBindVertexArray(VAOs[obj]);
//    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//}
//
//void draw_tex_object(GLuint obj){
//    glBindVertexArray(VAOs[obj]);
//    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
//    glVertexAttribPointer(tex_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
//    glEnableVertexAttribArray(tex_vPos);
//    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
//    glVertexAttribPointer(tex_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
//    glEnableVertexAttribArray(tex_vTex);
//    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
//}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(GLuint shader, GLuint tex)
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}