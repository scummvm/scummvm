uniform sampler2D Ytex;
uniform sampler2D Utex;
uniform sampler2D Vtex;

varying vec2 varCoord;

void main()
{
	float y, u, v, r, g, b;

	y=texture2D(Ytex, varCoord).a;
	u=texture2D(Utex, varCoord).a;
	v=texture2D(Vtex, varCoord).a;
	
	y=1.1643*(y-0.0625);
	u=u-0.5;
	v=v-0.5;

	r=y+1.5958*v;
	g=y-0.39173*u-0.81290*v;
	b=y+2.017*u;

	gl_FragColor=vec4(r,g,b,1.0);
}

