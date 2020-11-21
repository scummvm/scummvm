
float GetValue(float4 color)
{
	float colorMax = max (color[0], color[1]);
	colorMax = max (colorMax, color[2]);
	
	return colorMax;
}

float4 hsv_to_rgb(float3 HSV)
{
    float4 RGB = HSV.z;
    if ( HSV.y != 0 ) {
       float var_h = HSV.x * 6;
       float var_i = floor(var_h);   // Or ... var_i = floor( var_h )
       float var_1 = HSV.z * (1.0 - HSV.y);
       float var_2 = HSV.z * (1.0 - HSV.y * (var_h-var_i));
       float var_3 = HSV.z * (1.0 - HSV.y * (1-(var_h-var_i)));
       if      (var_i < 1.0) { RGB = float4(HSV.z, var_3, var_1, 1.0); }
       else if (var_i < 2.0) { RGB = float4(var_2, HSV.z, var_1, 1.0); }
       else if (var_i < 3.0) { RGB = float4(var_1, HSV.z, var_3, 1.0); }
       else if (var_i < 4.0) { RGB = float4(var_1, var_2, HSV.z, 1.0); }
       else if (var_i < 5.0) { RGB = float4(var_3, var_1, HSV.z, 1.0); }
       else                 { RGB = float4(HSV.z, var_1, var_2, 1.0); }
   }
   return (RGB);
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
// [0] = tint colour H
// [1] = tint colour S
// [2] = tint colour V
// [3] = tint saturation
const float4 tintHSV: register( c0 );
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

	float amount = tintHSV[3];
	float4 pixel = tex2D(Tex0, In.Texture); 
	float lum = GetValue(pixel);
	lum = max(lum - (1.0 - transparency[1]), 0.0);
	
	// scale the colour by the luma, alpha blend it with the tint
	Out.Color = (hsv_to_rgb(float3(tintHSV[0],tintHSV[1],lum)) * amount + pixel*(1-amount)); 
	Out.Color[3] = pixel[3] * transparency[0];

    return Out;                                //return output pixel
}
