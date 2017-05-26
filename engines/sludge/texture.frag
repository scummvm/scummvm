uniform sampler2D sampler2d;
uniform bool zBuffer;
uniform float zBufferLayer;
uniform bool modulateColor;

varying vec2 varCoord;
varying vec4 color;

void main(void)
{
	vec4 col = texture2D(sampler2d, varCoord);
	if (zBuffer && col.a < 0.0625*zBufferLayer-0.03)
	{
		discard;
	}
	if (modulateColor)
	{
		col = col * color;
	}
	gl_FragColor = col;
}

