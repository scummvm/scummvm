attribute vec4 myVertex;
attribute vec2 myUV0;
attribute vec2 myUV1;
attribute vec2 myUV2;

uniform mat4 myPMVMatrix;
uniform vec4 myColor;
uniform vec4 mySecondaryColor;

varying vec2 varCoord0;
varying vec2 varCoord1;
varying vec2 varCoord2;

varying vec4 color;
varying vec4 secondaryColor;

void main() {
	varCoord0 = myUV0.st;
	varCoord1 = myUV1.st;
	varCoord2 = myUV2.st;
	gl_Position = myPMVMatrix * myVertex;

	color = myColor;
	secondaryColor = mySecondaryColor;
}
