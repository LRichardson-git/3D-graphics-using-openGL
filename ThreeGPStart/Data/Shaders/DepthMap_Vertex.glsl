#version 330

layout(location = 0) in vec3 Vertex_Position;



// Values that stay constant for the whole mesh.
uniform mat4 LightProj;
uniform mat4 model;

void main(){



 gl_Position =  LightProj *  model * vec4(Vertex_Position, 1.0);
}

//uniform mat4 LightProj;
//uniform mat4 model;

//void main(){
// gl_Position =  LightProj * model * vec4(Vertex_Position, 1.0);
//}