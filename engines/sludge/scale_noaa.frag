uniform sampler2D Texture;
uniform sampler2D lightTexture;
uniform bool antialias;
uniform bool useLightTexture;
uniform float scale;

varying vec2 varCoord0;
varying vec2 varCoord1;

varying vec4 color;
varying vec4 secondaryColor;

void main()
{
	vec4 c11 = texture2D(Texture, varCoord0.xy);

	//gl_FragColor = c11;

	//if (gl_FragColor.a<0.001) discard;
	
	vec3 col;
	if (useLightTexture) {
		vec4 texture1 = texture2D (lightTexture, varCoord1.xy);
		col = texture1.rgb * c11.rgb;
	} else {
		col = color.rgb * c11.rgb;
	}
	col += vec3(secondaryColor);
	gl_FragColor = vec4 (col, color.a * c11.a);
}

