#ifndef __TEXNODE_H__
#define __TEXNODE_H__

#include "vgl.h"
#include "vmath.h"
#include "BaseNode.h"

class TexNode : public BaseNode
{
public:
	// Normal buffer
	GLuint TexBuff;
    GLuint TexPtr;
    GLint TexCoords;
    
    // Texture
    GLuint TexID;

	void set_buffers(GLuint vao, GLuint pBuff, GLuint pPtr, GLuint pCoord,
						GLuint tBuff, GLuint tPtr, GLuint tCoord, GLuint nVert) {
		VAO = vao;
		PosBuff = pBuff;
		PosPtr = pPtr;
		PosCoords = pCoord;
		TexBuff = tBuff;
		TexPtr = tPtr;
		TexCoords = tCoord;
		NumVertices = nVert;
	}
	

    void draw(vmath::mat4 proj, vmath::mat4 cam, vmath::mat4 trans){
    	// Select shader program
	    glUseProgram(Shader);
    	// Set projection and camera
    	glUniformMatrix4fv(ProjPtr, 1, GL_FALSE, proj);
    	glUniformMatrix4fv(CamPtr, 1, GL_FALSE, cam);

        // Set model and normal matrices
    	glUniformMatrix4fv(ModPtr, 1, GL_FALSE, trans*BaseTransform);

    	// Set vertex buffers and draw
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, PosBuff);
        glVertexAttribPointer(PosPtr, PosCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(PosPtr);
        glBindBuffer(GL_ARRAY_BUFFER, TexBuff);
        glVertexAttribPointer(TexPtr, TexCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(TexPtr);
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TexID);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }

};

#endif /* __NODE_H__ */