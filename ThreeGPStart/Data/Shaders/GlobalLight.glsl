#version 330

uniform sampler2DRect sampler_world_position;
uniform sampler2DRect sampler_world_normal;
uniform sampler2DRect sampler_tex_Col;




in vec4 gl_FragCoord;

vec4 world_position;
vec4 world_normal;
vec4 masterial_colour;
ivec2 icoord;

vec3 L;
vec3 N;

vec3 colour;
vec3 test;
out vec4 fragment_colour;


void main(void)
{
	icoord = ivec2 (int(gl_FragCoord.x), int(gl_FragCoord.y));

	

	world_position = texelFetch(sampler_world_position, icoord);
	world_normal = texelFetch(sampler_world_normal, icoord);
	masterial_colour = texelFetch(sampler_tex_Col, icoord);


	test = vec3(10,1008,1070);
	L = normalize(test);
	N = normalize(vec3(world_normal));
	
	
	colour = vec3(masterial_colour) * test * clamp(dot(L,N), 0, 1);

	fragment_colour = vec4(colour, 0.0);
	
	

}



























/*
vec2 test = sampler_tex_Col;

	vec3 Tex_Colour = texture(sampler_tex, test).rgb;

	vec4 Position = sampler_world_position;

	vec3 Light_Direction2 = Position - Light_Pos;

	vec3 Light_Normal = normalize(-Light_Direction2);
	vec3 newnewnormal = sampler_world_normal;
	vec3 Normal_Normal = normalize(newnewnormal);

	
	float  diffuse_intensity = max(light_Range,dot(Light_Normal,Normal_Normal));

	vec3 Light_Colour = Light_Colour;

	vec3 final_Colour = Tex_Colour * diffuse_intensity * Light_Colour;

	
	fragment_colour = vec4(final_Colour, 1.0);
	*/