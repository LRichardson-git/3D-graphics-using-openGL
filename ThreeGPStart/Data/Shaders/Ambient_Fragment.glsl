#version 330




layout (std140) uniform FrameUniforms {

		vec3 Light_Pos;
		float light_Range;
		vec3 light_Direction;
		vec3 cam_Pos;
		vec3 light_ambient;
		vec3 Light_Colour;

};








out vec4 fragment_colour;



void main(void)
{
	
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