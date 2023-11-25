#version 330

uniform samplerCube skybox;

uniform sampler2D sampler_tex;


layout (std140) uniform FrameUniforms {

		vec3 Light_Pos;
		float light_Range;
		vec3 light_Direction;
		vec3 cam_Pos;
		vec3 light_ambient;
		vec3 Light_Colour;

};



in vec3 Varying_Normal;
in vec2 Varying_Texcord;
in vec3 Varying_Position;

out vec4 fragment_colour;

void main(void)
{

	//I DO NOT THINK THIS TAKES INTO ACCOUNT LIGHT DIRECTION (SUCH AS IF LIGHT IS FACING AWAY WHEN CREATED IT WILL TREAT ALL THE LIGHTS AS GOING EVERYWHERE I THINK)


	vec3 Tex_Colour = texture(sampler_tex, Varying_Texcord).rgb;

	vec3 Position = Varying_Position;

	vec3 Light_Direction2 = Position - Light_Pos;

	vec3 Light_Normal = normalize(-Light_Direction2);
	vec3 Normal_Normal = normalize(Varying_Normal);


	float  diffuse_intensity = max(light_Range,dot(Light_Normal,Normal_Normal));

	vec3 Light_Colour = Light_Colour;

	vec3 final_Colour = Tex_Colour * diffuse_intensity * Light_Colour;

	
	fragment_colour = vec4(final_Colour, 1.0);
	

}



/*


Split the code in the shaders up
each into their own vertex and fragment shaders
e.g: Ambeint vertex shader and ambient fragment shader
in the code draw the ambient light for each model once

light shader
vertex and fragment
in the rendering check each lights effect on each model, so if you have 5 models and 10 lights that 50 checks

look at the lecture slides Table with the different settings and change them in the cpp code for each type of rendering loop
anti alaisingwill be a shader as well
offscreen rendering shader as well

look at the book



*/