/*
   Inspired by a shader by guest(r) - guest.r@gmail.com - that was found at
   http://www.razyboard.com/system/morethread-smart-texture-mag-filter-for-ogl2-and-dosbox-pete_bernert-266904-5689051-0.html
*/

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

	if (antialias) {
		vec2 fw = fwidth(varCoord0.xy)*scale;

		vec2 sd1 = vec2( fw.x,fw.y);
		vec2 sd2 = vec2(-fw.x,fw.y);

		vec4 s00 = texture2D(Texture, varCoord0.xy-sd1);
		vec4 s20 = texture2D(Texture, varCoord0.xy-sd2);
		vec4 s22 = texture2D(Texture, varCoord0.xy+sd1);
		vec4 s02 = texture2D(Texture, varCoord0.xy+sd2);   

		vec4 dt = vec4(1.0,1.0,1.0,1.0);
		
		float m1=dot(abs(s00-s22),dt)+0.0001;
		float m2=dot(abs(s02-s20),dt)+0.0001;

		vec4 temp1 = m2*(s00 + s22) + m1*(s02 + s20);

	//	gl_FragColor = (temp1/(m1+m2)) * 0.5;
		c11 = c11*0.333333 + (temp1/(m1+m2)) * 0.333333;
	} 
	/*
	else {
		gl_FragColor = c11;
	}*/

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

