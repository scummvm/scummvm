attribute vec4 myVertex;
attribute vec2 myUV0;

uniform mat4 myPMVMatrix;

varying vec2 varCoord;

void main()
{
	gl_Position = myPMVMatrix * myVertex;
	varCoord = myUV0.st;
}
