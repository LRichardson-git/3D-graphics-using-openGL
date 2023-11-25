#version 330
layout(location = 0) out vec4 color;
out vec4 fragment_colour;

in vec2 UV;

//out vec3 color;

uniform sampler2D Texture;

const float kernel = 10.0f;
const float weight = 1.0f;


void main(void)
{
	


	// our texture

	vec2 u_textureSize;
	u_textureSize.x = 1280;
	u_textureSize.y = 720;


	vec2 textCoord = gl_FragCoord.xy / u_textureSize;
	vec2 onePixel = vec2(1.0, 1.0) / u_textureSize;

	vec4 final = (
		texture2D(Texture, textCoord + onePixel*vec2(-1.0, -1.0)) +
		texture2D(Texture, textCoord + onePixel*vec2(0.0, -1.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(1.0, -1.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(-1.0, 0.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(0.0,  0.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(1.0,  0.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(-1.0, 1.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(0.0,  1.0)) + 
		texture2D(Texture, textCoord + onePixel*vec2(1.0,  1.0))) / 9.0;

	float depth = texture(Texture, UV).x;
	depth = 1.0 - (1.0 - depth) *25;
	color =vec4( depth);
}





