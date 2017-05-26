attribute vec4 myVertex;

uniform mat4 myPMVMatrix;
uniform vec4 myColor;

varying vec4 color;

void main(void)
{
	gl_Position = myPMVMatrix * myVertex;
	color = myColor;
}

