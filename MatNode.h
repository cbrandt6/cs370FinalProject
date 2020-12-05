#ifndef __MATNODE_H__
#define __MATNODE_H__

#include "vgl.h"
#include "vmath.h"
#include "BaseNode.h"

class MatNode : public BaseNode
{
public:
    // Shader reference
    GLuint NormMatPtr;

	// Normal buffer
	GLuint NormBuff;
    GLuint NormPtr;
    GLint NormCoords;
    
    // Material buffer
    GLuint MatBuff;
	GLuint MatBlockIdx;
	GLuint MatSize;
	GLuint MatIDPtr;
    GLuint MatID;

	// Light buffer
	GLuint LightBuff;
	GLuint LightBlockIdx;
	GLuint LightSize;
	GLuint NumLightsPtr;
	GLuint NumLights;

	// Eye ptr
	vmath::vec3 EyeVec;
	GLuint EyePtr;

	void set_buffers(GLuint vao, GLuint pBuff, GLuint pPtr, GLuint pCoord,
						GLuint nBuff, GLuint nPtr, GLuint nCoord, GLuint nVert) {
		VAO = vao;
		PosBuff = pBuff;
		PosPtr = pPtr;
		PosCoords = pCoord;
		NormBuff = nBuff;
		NormPtr = nPtr;
		NormCoords = nCoord;
		NumVertices = nVert;
	}
	
	void set_materials(GLuint mBuff, GLuint mBlk, GLuint mSize, GLuint mIDPtr, GLuint mID) {
        MatBuff = mBuff;
        MatBlockIdx = mBlk;
        MatSize = mSize;
        MatIDPtr = mIDPtr;
        MatID = mID;
	}
    void set_lights(GLuint lBuff, GLuint lBlk, GLuint lSize, GLuint numLightPtr, GLuint numLights) {
        LightBuff = lBuff;
        LightBlockIdx = lBlk;
        LightSize = lSize;
        NumLightsPtr = numLightPtr;
        NumLights = numLights;
    }
    void set_eye(GLuint eyePtr, vmath::vec3 eyeVec) {
        EyePtr = eyePtr;
        EyeVec = eyeVec;
    }

    void draw(vmath::mat4 proj, vmath::mat4 cam, vmath::mat4 trans){
    	// Select shader program
	    glUseProgram(Shader);
    	// Set projection and camera
    	glUniformMatrix4fv(ProjPtr, 1, GL_FALSE, proj);
    	glUniformMatrix4fv(CamPtr, 1, GL_FALSE, cam);
    	// Bind lights
    	glUniformBlockBinding(Shader, LightBlockIdx, 0);
    	glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuff, 0, LightSize);
    	glUniform1i(NumLightsPtr, NumLights);
    	// Bind materials
    	glUniformBlockBinding(Shader, MatBlockIdx, 1);
    	glBindBufferRange(GL_UNIFORM_BUFFER, 1, MatBuff, 0, MatSize);
        glUniform1i(MatIDPtr, MatID);
        // Set eye position
    	glUniform3fv(EyePtr, 1, EyeVec);

        // Set model and normal matrices
    	glUniformMatrix4fv(ModPtr, 1, GL_FALSE, trans*BaseTransform);
        vmath::mat4 normal_matrix = trans.inverse().transpose();
    	glUniformMatrix4fv(NormMatPtr, 1, GL_FALSE, normal_matrix);

    	// Set vertex buffers and draw
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, PosBuff);
        glVertexAttribPointer(PosPtr, PosCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(PosPtr);
        glBindBuffer(GL_ARRAY_BUFFER, NormBuff);
        glVertexAttribPointer(NormPtr, NormCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(NormPtr);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }

};

#endif /* __NODE_H__ */