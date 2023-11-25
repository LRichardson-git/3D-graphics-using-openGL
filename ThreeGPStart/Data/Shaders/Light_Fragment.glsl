#version 330
layout(location = 0) out vec3 color;

uniform samplerCube skybox;

uniform sampler2D sampler_tex;

uniform sampler2D sampler_tex_Col;

layout (std140) uniform FrameUniforms {

		vec3 Light_Pos;
		float light_Range;
		vec3 light_Direction;
		vec3 cam_Pos;
		vec3 light_ambient;
		vec3 Light_Colour;

};

//shadows
in vec4	FragPosLight;
uniform sampler2D ShadowMap;



in vec3 Varying_Normal;
in vec2 Varying_Texcord;
in vec3 Varying_Position;




out vec4 fragment_colour;

void main(void)
{

	//I DO NOT THINK THIS TAKES INTO ACCOUNT LIGHT DIRECTION (SUCH AS IF LIGHT IS FACING AWAY WHEN CREATED IT WILL TREAT ALL THE LIGHTS AS GOING EVERYWHERE I THINK)


	//DEFFERED WAS ATTEMPTED AND Failed




	vec3 Tex_Colour = texture(sampler_tex, Varying_Texcord).rgb;

	vec3 Position = Varying_Position;

	vec3 Light_Direction2 = Position - Light_Pos;

	vec3 Light_Normal = normalize(-Light_Direction2);
	vec3 Normal_Normal = normalize(Varying_Normal);


	float  diffuse_intensity = max(light_Range,dot(Light_Normal,Normal_Normal));

	vec3 Light_Colour = Light_Colour;

	
	
	//shadows

	float shadow = 0.0f;
	// Sets lightCoords to cull space
	vec3 lightCoords = FragPosLight.xyz / FragPosLight.w;
	if(lightCoords.z <= 1.0f)
	{
		// Get from [-1, 1] range to [0, 1] range just like the shadow map
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		// Prevents shadow acne
		float bias = max(0.025f * (1.0f - dot(Light_Normal, Normal_Normal)), 0.0005f);

		// Smoothens out the shadows
		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(ShadowMap, 0);
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(ShadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;     
		    }    
		}
		// Get average shadow
		shadow /= pow((sampleRadius * 2 + 1), 2);

	}





	//diffuse_intensity = diffuse_intensity * (1.0f - Shadow);


	
	vec3 final_Colour = Tex_Colour * (diffuse_intensity * (1.0 - shadow))  * Light_Colour ;
	
	//color = vec3(final_Colour);
	fragment_colour = vec4(final_Colour, 1.0);
	

}



