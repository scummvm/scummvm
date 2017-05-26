attribute vec4 myVertex;
attribute vec2 myUV0;
attribute vec2 myUV1;

uniform mat4 myPMVMatrix;
uniform vec4 myColor;
uniform vec4 mySecondaryColor;

varying vec2 varCoord0;
varying vec2 varCoord1;

varying vec4 color;
varying vec4 secondaryColor;

void main()
{
	gl_Position = myPMVMatrix * myVertex;
	varCoord0 = myUV0.st;

	// Light
	varCoord1 = myUV1.st;

	color = myColor;
	secondaryColor = mySecondaryColor;
}
