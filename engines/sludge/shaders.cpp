/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stringy.h"
#include "shaders.h"
#include "graphics.h"

extern char *bundleFolder;

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we need to pass it as a string to the GLSL driver
char *shaderFileRead(const char *name) {
	FILE *fp;
	char *content = NULL;
	char *fn = joinStrings(bundleFolder, name);

	int count = 0;

	if (fn != NULL) {

		fp = fopen(fn, "rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);

		}
	}

	delete fn;

	return content;
}

static void
printShaderInfoLog(GLuint shader) {
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	char *infoLog;

#if 0
	printOpenGLError();   // Check for OpenGL errors
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError();   // Check for OpenGL errors
#endif

	if (infologLength > 0) {
		infoLog = new char [infologLength];
		if (infoLog == NULL) {
			debugOut("ERROR: Could not allocate InfoLog buffer");
			return;
		}
#if 0
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		debugOut("Shader InfoLog:\n%s\n\n", infoLog);
#endif
		delete[] infoLog;
	}
	printOpenGLError();  // Check for OpenGL errors
}

/* Print out the information log for a program object */
static void
printProgramInfoLog(GLuint program) {
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	char *infoLog;
#if 0
	printOpenGLError();   // Check for OpenGL errors
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError();   // Check for OpenGL errors
#endif
	if (infologLength > 0) {
		infoLog = new char [infologLength];
		if (infoLog == NULL) {
			debugOut("ERROR: Could not allocate InfoLog buffer");
			return;
		}
#if 0
		glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
		debugOut("Program InfoLog:\n%s\n\n", infoLog);
#endif
		delete[] infoLog;
	}
	printOpenGLError();   // Check for OpenGL errors
}

int buildShaders(const char *vertexShader, const char *fragmentShader) {
	GLuint VS, FS, prog;
	GLint vertCompiled, fragCompiled;
	GLint linked;
#if 0
	// Create Shader Objects
	VS = glCreateShader(GL_VERTEX_SHADER);
	FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Load source code strings into shaders
	glShaderSource(VS, 1, &vertexShader, NULL);
	glShaderSource(FS, 1, &fragmentShader, NULL);
#endif
	debugOut("Compiling vertex shader... \n");
#if 0
	// Compile vertex shader and print log
	glCompileShader(VS);
	printOpenGLError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &vertCompiled);
	printShaderInfoLog(VS);
#endif
	debugOut("\nCompiling fragment shader... \n");
#if 0
	// Compile fragment shader and print log
	glCompileShader(FS);
	printOpenGLError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &fragCompiled);
	printShaderInfoLog(FS);
#endif
	if (!vertCompiled || !fragCompiled)
		return 0;

	debugOut("\nShaders compiled. \n");

#if 0
	// Create a program object and attach the two compiled shaders
	prog = glCreateProgram();
	glAttachShader(prog, VS);
	glAttachShader(prog, FS);

	// Clean up
	glDeleteShader(VS);
	glDeleteShader(FS);

	// Link the program and print log
	glLinkProgram(prog);
	printOpenGLError();
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);
#endif
	if (!linked)
		return 0;

	debugOut("Shader program linked. \n");

	return prog;
}


