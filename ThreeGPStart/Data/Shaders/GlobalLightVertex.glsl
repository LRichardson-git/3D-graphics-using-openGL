#version 330

in vec2 Vertex_Position;




void main(void)
{

	gl_Position = vec4 (Vertex_Position, 0.0, 1.0);



}