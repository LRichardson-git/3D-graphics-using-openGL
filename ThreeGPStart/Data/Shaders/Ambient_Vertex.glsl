#version 330

layout(location = 0) in vec3 Vertex_Position;
layout(location = 1) in vec3 Vertex_Normal;
layout(location = 2) in vec2 Vertex_TexCoord;



layout (std140) uniform ModelUniforms {

		mat4 projection_xform;
		mat4 model_xform;
		mat4 view_xform;

	};









out vec3 Position;
out vec3 Varying_Normal;
out vec2 Varying_Texcord;


void main(void)
{

	Position = mat4x3(model_xform) * vec4(Vertex_Position,1.0);

	Varying_Normal = Vertex_Normal;
	Varying_Texcord = Vertex_TexCoord;


	mat4 combined_xform = projection_xform * view_xform * model_xform;
	gl_Position = combined_xform * vec4(Vertex_Position, 1.0);



}