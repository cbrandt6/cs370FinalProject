#ifndef __BASENODE_H__
#define __BASENODE_H__

#include "vgl.h"
#include "vmath.h"

class BaseNode
{
public:
    // Object buffers
    GLuint VAO;
    GLuint PosBuff;
    GLuint PosPtr;
    GLint PosCoords;
    GLint NumVertices;
    
	// Shader references
	GLuint Shader;
	GLuint ProjPtr;
	GLuint CamPtr;
	GLuint ModPtr;

	// Object properties
    vmath::mat4 BaseTransform;
	vmath::mat4 ModelTransform;
	BaseNode *sibling;
	BaseNode *child;


    BaseNode() {
        BaseTransform = vmath::mat4().identity();
        ModelTransform = vmath::mat4().identity();
        Shader = 0;
        sibling = NULL;
        child = NULL;
    }

    void set_shader(GLuint shader, GLuint pPtr, GLuint cPtr, GLuint mPtr) {
        Shader = shader;
        ProjPtr = pPtr;
        CamPtr = cPtr;
        ModPtr = mPtr;
    }

    virtual void draw(vmath::mat4 proj, vmath::mat4 cam, vmath::mat4 trans) = 0;

    void set_base_transform(vmath::mat4 transform) {
        BaseTransform = transform;
    }

    void update_transform(vmath::mat4 transform) {
    	ModelTransform = transform;
    };

};

#endif /* __BASENODE_H__ */