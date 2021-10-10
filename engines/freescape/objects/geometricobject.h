//
//  GeometricObject.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__GeometricObject__
#define __Phantasma__GeometricObject__

#include "common/array.h"
#include "freescape/language/instruction.h"
#include "freescape/objects/object.h"

class GeometricObject : public Object {
public:
	static int numberOfColoursForObjectOfType(Type type);
	static int numberOfOrdinatesForType(Type type);

	/*static void setupOpenGL();
		static void setProjectionMatrix(const GLfloat *projectionMatrix);
		static void setViewMatrix(const GLfloat *projectionMatrix);

		static VertexBuffer *newVertexBuffer();
		static DrawElementsBuffer *newDrawElementsBuffer();*/

	GeometricObject(
		Type type,
		uint16 objectID,
		uint16 flags,
		const Vector3d &origin,
		const Vector3d &size,
		Common::Array<uint8> *colours,
		Common::Array<uint16> *ordinates,
		FCLInstructionVector condition);
	virtual ~GeometricObject();

	/*void setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer);*/
	void draw(Freescape::Renderer *gfx) override;
	bool isDrawable();
	bool isPlanar();

private:
	/*static GLuint openGLProgram;
		static GLuint compileShader(const GLchar *source, GLenum shaderType);
		static GLint viewMatrixUniform, projectionMatrixUniform;*/

	FCLInstructionVector condition;
	Common::Array<uint8> *colours;
	Common::Array<uint16> *ordinates;

	size_t drawElementsStartIndex;
	//GLsizei drawElementsCount;
	//GLenum drawElementsMode;
};

#endif /* defined(__Phantasma__GeometricObject__) */
