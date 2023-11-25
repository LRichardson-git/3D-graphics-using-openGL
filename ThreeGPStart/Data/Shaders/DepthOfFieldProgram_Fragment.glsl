#version 330

layout(location = 0) out vec4 color;



in vec2 UV;

//out vec3 color;
uniform sampler2D Texture;
uniform sampler2DRect sampler_world_position;
uniform sampler2D TextureBlur;
uniform vec2 cam;

  float minDistance = 20.0;
  float maxDistance = 350.0;

void main(void)
{
	
	vec4 focusColor = texture(Texture, UV);
	// vec4 outOfFocusColor = texture(TextureBlur, UV);

	vec2 u_textureSize;
	u_textureSize.x = 1280;
	u_textureSize.y = 720;


	vec2 textCoord = gl_FragCoord.xy / u_textureSize;
	vec2 onePixel = vec2(1.0, 1.0) / u_textureSize;



	ivec2 icoord;




	icoord = ivec2 (int(gl_FragCoord.x), int(gl_FragCoord.y));

	vec4 outOfFocusColor = (
		texture2D(TextureBlur, textCoord + onePixel*vec2(-2.0, -2.0)) +
		texture2D(TextureBlur, textCoord + onePixel*vec2(1-.0, -2.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(2.0, -2.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(-2.0, -1.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(-1.0,  -1.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(2.0,  -1.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(-2.0, 2.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(-1.0,  2.0)) + 
		texture2D(TextureBlur, textCoord + onePixel*vec2(2.0,  2.0))) / 9.0;

	 vec4 position = texelFetch(sampler_world_position, icoord);
	// our texture
	
	vec4 focusPoint = texture(sampler_world_position,ivec2(0.4,0.7));
	//vec4 focusPoint = texture(sampler_world_position, vec2(0.5,0.5));

	float blur = smoothstep(minDistance, maxDistance , abs(position.y - focusPoint.y));

	//color = focusPoint;
	color = mix(focusColor, outOfFocusColor, blur);

	}

