#version 330

 layout(location = 0) in vec3 Vertex_Position;
 //layout(location = 1) in vec3 colour;
 layout(location = 1) in vec3 TexCoord;

 out vec2 UV;
 //out vec3 color;


void main(void)
{
	UV = vec2(TexCoord);
	gl_Position = vec4 (Vertex_Position,1.0);
	}