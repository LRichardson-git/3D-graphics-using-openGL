#version 330



layout (std140) uniform FrameUniforms {

		vec3 Light_Pos;
		float light_Range;
		vec3 light_Direction;
		vec3 cam_Pos;
		vec3 light_ambient;
		vec3 Light_Colour;

};



in vec3 Position;
in vec3 Varying_Normal;
in vec2 Varying_Texcord;




out vec4 Frag_Position;
out vec3 Frag_Normal;
out vec2 Frag_TexCoord;
out vec4 fragment_colour;
float distance;

void main(void)
{
	
	

	distance = length(cam_Pos - distance);

	Frag_Position = vec4(Position, distance);
	Frag_Normal = Varying_Normal;
	Frag_TexCoord = Varying_Texcord;
	fragment_colour = vec4(light_ambient, 1.0);

};



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