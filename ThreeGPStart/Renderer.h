#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"




struct ModelMesh {
	//vertex array object
	GLuint VAO{ 0 };
	//number of elemets to use when rendering
	GLuint numElements{ 0 };
	//texture id
	GLuint textureId{ 0 };

	

};

struct Light {

	glm::vec3 Light_Position{ 0 };
	glm::float32_t Light_Range{ 0 };
	glm::vec3 Light_Colour{ 1,1,1 };


};


struct Model {
	std::string name;
	glm::mat4 TranslationMatrix = glm::mat4(1.0f);
	std::vector<ModelMesh> Model_Mesh;
};


class Renderer
{
private:
	// Program object - to host shaders
	GLuint FXAA_Program{ 0 };
	GLuint m_Ambient{ 0 };
	GLuint m_Lights{ 0 };
	GLuint m_Gbuffer{ 0 };
	GLuint m_LGlobalbuffer{ 0 };
	GLuint VAO2;
	GLuint globalLightMesh;
	GLuint m_BlurFrameBuffer;
	GLuint m_DepthOfFieldBuffer;
	GLuint m_BlurProgram;
	GLuint m_DepthOffieldProgram;
	GLuint BlurredImage;
	GLuint DepthOfFieldImage;
	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };
	GLuint quad_VertexArrayID;
	// Number of elments to use when rendering
	GLuint m_numElements{ 0 };
	GLuint GrassTexture;
	GLuint GrassVao;
	GLuint texture;
	GLuint LrenderBuffer;
	GLuint m_DepthMapProg;

	bool FXAA{ false };

	GLuint DepthFramebuffer_FBO;
	GLuint DepthMap_texture;
	int width = 1280;
	int height = 720;

	int lol1 =  1;
	int  lol2 = 2;
	
	int maxPosition = 300;
	int trackPostion = 0;

	bool depthoffield = false;
	bool Gbuffer = false;
	bool shadows = false;

	glm::mat4 depthMVP;
	glm::mat4 lightSpaceMatrix;
	std::vector<Light> m_Lights_Scene;
	std::vector<Model> m_Models;

	bool m_wireframe{ false };

	bool CreateProgram();

	struct FrameUniforms {

		glm::vec3 Light_Pos;
		glm::float32_t light_Range;
		glm::vec3 light_Direction; //gonna use in future but not atm
		glm::float32_t pad1 = 0.0;
		glm::vec3 cam_Pos; //not using atm but might be useful in future
		glm::float32_t pad2 = 0.0;
		glm::vec3 light_ambient;
		glm::float32_t pad3 = 0.0;
		glm::vec3 Light_Colour;
		glm::float32_t pad4 = 0.0;

	};

	GLuint Per_Frame_UBO;

	struct ModelUniforms {

		glm::mat4 projection_xform;
		glm::mat4 model_xform;
		glm::mat4 view_xform;
		//no padding neccaset atm
	
		

	};

	GLuint per_Model_UBO;

	
	//deffered 
	GLuint GBuffer_FBO;
	GLuint GBuffer_Pos_Tex;
	GLuint GBuffer_Normal_Tex;
	GLuint GBuffer_Depth_Tex;
	GLuint GBuffer_Colour_Tex;



	
	/// fxaa
	GLuint FXAA_FBO;
	GLuint Rendered_Texture_FXAA;
	

	GLuint Lbuffer_FBO;
	GLuint Lbuffer_Colour_tex;
	GLuint Lbuffer_colour_rbo;


public:
	Renderer();
	~Renderer();
	void DefineGUI();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	bool Terrain();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);

	void CreateModel(Helpers::ModelLoader& Model_C, GLuint& TextureM, glm::vec3& PositionMat, glm::vec3& ScaleMat);

};

