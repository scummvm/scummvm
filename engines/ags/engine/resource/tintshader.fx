
float GetLuminance(float4 color)
{
	float colorMax = max (color[0], color[1]);
	colorMax = max (colorMax, color[2]);
	
	return colorMax;
}


// Pixel shader input structure
struct PS_INPUT
{
    float2 Texture    : TEXCOORD0;
};

// Pixel shader output structure
struct PS_OUTPUT
{
    float4 Color   : COLOR0;
};

// Global variables
sampler2D Tex0;

// tintHSV: parameters from AGS engine
// [0] = tint colour R
// [1] = tint colour G
// [2] = tint colour B
// [3] = tint saturation
const float4 tintRGB: register( c0 );
// [0] = object transparency
// [1] = light level
const float4 transparency: register( c1 );

// Name: Simple Pixel Shader
// Type: Pixel shader
// Desc: Fetch texture and blend with constant color
//
PS_OUTPUT main( in PS_INPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel

	float amount = tintRGB[3];
	float4 pixel = tex2D(Tex0, In.Texture);  // tint should be the RGB conversion of the HSL value, with 100% L
	float lum = GetLuminance(pixel);
	
	// scale the colour by the luma, alpha blend it with the tint
	Out.Color = (tintRGB * lum * amount + pixel*(1-amount)) * transparency[1]; 
	Out.Color[3] = pixel[3] * transparency[0];

    return Out;                                //return output pixel
}
