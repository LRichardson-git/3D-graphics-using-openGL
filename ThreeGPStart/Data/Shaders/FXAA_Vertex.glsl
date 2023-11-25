#version 330


 //vec2 Vertex_Position;

 layout(location = 0) in vec3 Vertex_Position;
 //layout(location = 1) in vec3 colour;
 layout(location = 1) in vec3 TexCoord;

 out vec2 UV;
 //out vec3 color;

 float scale;
void main(void)
{
	scale = 1f;
	UV = vec2(TexCoord);
	gl_Position = vec4 (Vertex_Position,1.0);

	//color = colour;
	

}




