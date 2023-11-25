#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"
#include "View.h"
#include "stb_image.h"

Renderer::Renderer() 
{
	
}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls

	//Programs
	glDeleteProgram(FXAA_Program);
	glDeleteProgram(m_Ambient);
	glDeleteProgram(m_Lights);
	glDeleteProgram(m_Gbuffer);
	glDeleteProgram(m_LGlobalbuffer);
	glDeleteProgram(m_DepthMapProg);
	glDeleteProgram(m_DepthOffieldProgram);
	glDeleteProgram(m_BlurProgram);
	
	//Textures
	glDeleteTextures(1, &GBuffer_Pos_Tex);
	glDeleteTextures(1, &GBuffer_Normal_Tex);
	glDeleteTextures(1, &GBuffer_Colour_Tex);
	glDeleteTextures(1, &GBuffer_Depth_Tex);
	glDeleteTextures(1, &Lbuffer_Colour_tex);
	glDeleteTextures(1, &Rendered_Texture_FXAA);
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &GrassTexture);
	glDeleteTextures(1, &DepthMap_texture);
	glDeleteTextures(1, &BlurredImage);
	glDeleteTextures(1, &DepthOfFieldImage);

	//Buffers
	glDeleteBuffers(1, &m_VAO);
	glDeleteBuffers(1, &GrassVao);
	glDeleteFramebuffers(1, &GBuffer_FBO);
	glDeleteFramebuffers(1, &DepthFramebuffer_FBO);
	glDeleteFramebuffers(1, &Lbuffer_FBO);
	glDeleteFramebuffers(1, &FXAA_FBO);
	glDeleteRenderbuffers(1, &Lbuffer_colour_rbo);
	glDeleteRenderbuffers(1, &Lbuffer_colour_rbo);
	glDeleteRenderbuffers(1, &m_BlurFrameBuffer);
	glDeleteRenderbuffers(1, &m_DepthOfFieldBuffer);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		ImGui::Begin("RTG");                    // Create a window called "RTG" and append into it.

		ImGui::Text("Visibility.");             // Display some text (you can use a format strings too)	

		ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

		ImGui::Checkbox("FXAA", &FXAA);	// A checkbox linked to a member variable

		ImGui::Checkbox("DepthOfField", &depthoffield);	// A checkbox linked to a member variable
		ImGui::Checkbox("Shadows", &shadows);
		ImGui::Checkbox("Gbuffer", &Gbuffer);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		


		ImGui::End();
	}
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram()
{
	// Create a new program (returns a unqiue id)
	
	

	// Load and create vertex and fragment shaders
	GLuint Ambient_Vertex_Shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/Ambient_Vertex.glsl") };
	GLuint Ambient_Fragment_Shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/Ambient_Fragment.glsl") };
	GLuint Light_Vertex_Shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/Light_Vertex.glsl") };
	GLuint Light_Fragment_Vertex_Shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/Light_Fragment.glsl") };
	GLuint GBuffer_Vertex_Shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/Gbuffer_Vertex.glsl") };
	GLuint GBuffer_Fragment_Vertex_Shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/Gbuffer_Fragment.glsl") };
	GLuint GlobalLightShaderVertex{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/GlobalLightVertex.glsl") };
	GLuint GlobalLightShader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/GlobalLight.glsl") };
	GLuint FXAAVertex{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/FXAA_Vertex.glsl") };
	GLuint FXAAFragment{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/FXAA_Fragment.glsl") };
	GLuint DepthVertex{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/DepthMap_Vertex.glsl") };
	GLuint DepthFragment{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/DepthMap_Fragment.glsl") };
	//Depth field
	GLuint DepthOfFieldVertex{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/DepthOfFieldProgram_Vertex.glsl") };
	GLuint DepthOfFieldFragment{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/DepthOfFieldProgram_Fragment.glsl") };

	GLuint BlurVertex{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/BlurProgram_Vertex.glsl") };
	GLuint BlurFragment{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/BlurProgram_Fragment.glsl") };

	if (Ambient_Vertex_Shader == 0 || Ambient_Fragment_Shader == 0 || Light_Vertex_Shader == 0 || Light_Fragment_Vertex_Shader == 0 || GBuffer_Fragment_Vertex_Shader == 0 || GBuffer_Vertex_Shader == 0 || FXAAVertex == 0 || FXAAFragment == 0)
		return false;




	//depth of field program

	m_DepthOffieldProgram = glCreateProgram();

	glAttachShader(m_DepthOffieldProgram, DepthOfFieldVertex);
	glBindAttribLocation(m_DepthOffieldProgram, 0, "Vertex_Position");

	glAttachShader(m_DepthOffieldProgram, DepthOfFieldFragment);
	glBindAttribLocation(m_DepthOffieldProgram, 0, "fragment_colour");

	if (!Helpers::LinkProgramShaders(m_DepthOffieldProgram))
		return false;

	//blur program


	m_BlurProgram = glCreateProgram();

	glAttachShader(m_BlurProgram, BlurVertex);
	glBindAttribLocation(m_BlurProgram, 0, "Vertex_Position");

	glAttachShader(m_BlurProgram, BlurFragment);
	glBindAttribLocation(m_BlurProgram, 0, "fragment_colour");

	if (!Helpers::LinkProgramShaders(m_BlurProgram))
		return false;





	//FXAA Program

	FXAA_Program = glCreateProgram();

	glAttachShader(FXAA_Program, FXAAVertex);
	glBindAttribLocation(FXAA_Program, 0, "Vertex_Position");

	glAttachShader(FXAA_Program, FXAAFragment);
	glBindAttribLocation(FXAA_Program, 0, "fragment_colour");

	if (!Helpers::LinkProgramShaders(FXAA_Program))
		return false;


	//DepthMap Program



	m_DepthMapProg = glCreateProgram();

	glAttachShader(m_DepthMapProg, DepthVertex);
	glBindAttribLocation(m_DepthMapProg, 0, "Vertex_Position");

	glAttachShader(m_DepthMapProg, DepthFragment);
	glBindAttribLocation(m_DepthMapProg, 0, "fragment_colour");

	if (!Helpers::LinkProgramShaders(m_DepthMapProg))
		return false;



	
	//Create Uniform buffers for use
	glGenBuffers(1, &Per_Frame_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, Per_Frame_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameUniforms), nullptr, GL_STREAM_DRAW);

	glGenBuffers(1, &per_Model_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, per_Model_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ModelUniforms), nullptr, GL_STREAM_DRAW);


	

	


	//ambient program -----------------------------------------------------------------------------------------------------

	//Create a new program for my ambient shaders
	m_Ambient = glCreateProgram();

	glAttachShader(m_Ambient, Ambient_Vertex_Shader);
	glBindAttribLocation(m_Ambient, 0, "Vertex_Position");

	glAttachShader(m_Ambient, Ambient_Fragment_Shader);
	glBindAttribLocation(m_Ambient, 0, "fragment_colour");

	

	//WE check if the programs linked correctly
	if (!Helpers::LinkProgramShaders(m_Ambient))
		return false;


	//bind Uniform buffer objects to our program
	//I am also telling the shaders that it can expect a uniform called "FrameUnifroms" which is a struct I have created
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, Per_Frame_UBO);
	glUniformBlockBinding(m_Ambient, glGetUniformBlockIndex(m_Ambient, "FrameUniforms"), 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, per_Model_UBO);
	glUniformBlockBinding(m_Ambient, glGetUniformBlockIndex(m_Ambient, "ModelUniforms"), 1);


	//Light program -------------------------------------------------------------------------------------------------------

	m_Lights = glCreateProgram();


	glAttachShader(m_Lights, Light_Vertex_Shader);
	glBindAttribLocation(m_Lights, 0, "Vertex_Position");
	glBindAttribLocation(m_Lights, 2, "Vertex_Normal");

	glAttachShader(m_Lights, Light_Fragment_Vertex_Shader);
	glBindAttribLocation(m_Lights, 0, "fragment_colour");

	

	if (!Helpers::LinkProgramShaders(m_Lights))
		return false;

	//Light UBO

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, Per_Frame_UBO);
	glUniformBlockBinding(m_Lights, glGetUniformBlockIndex(m_Lights, "FrameUniforms"), 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, per_Model_UBO);
	glUniformBlockBinding(m_Lights, glGetUniformBlockIndex(m_Lights, "ModelUniforms"), 1);


	//mburffer program

	m_Gbuffer = glCreateProgram();

	glAttachShader(m_Gbuffer, GBuffer_Vertex_Shader);
	glBindAttribLocation(m_Gbuffer, 0, "Vertex_Position");
	glBindAttribLocation(m_Gbuffer, 2, "Vertex_Normal");
	
	glAttachShader(m_Gbuffer, GBuffer_Fragment_Vertex_Shader);
	glBindFragDataLocation(m_Gbuffer, 0, "Frag_Position");
	glBindFragDataLocation(m_Gbuffer, 1, "Frag_Normal");
	glBindFragDataLocation(m_Gbuffer, 2, "Frag_TexCoord");



	if (!Helpers::LinkProgramShaders(m_Gbuffer))
		return false;


	

	//Light UBO

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, Per_Frame_UBO);
	glUniformBlockBinding(m_Gbuffer, glGetUniformBlockIndex(m_Gbuffer, "FrameUniforms"), 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, per_Model_UBO);
	glUniformBlockBinding(m_Gbuffer, glGetUniformBlockIndex(m_Gbuffer, "ModelUniforms"), 1);


	return !Helpers::CheckForGLError();
	m_LGlobalbuffer = glCreateProgram();


	glAttachShader(m_LGlobalbuffer, GlobalLightShaderVertex);
	glBindAttribLocation(m_LGlobalbuffer, 0, "Vertex_Position");
	

	glAttachShader(m_LGlobalbuffer, GlobalLightShader);
	glBindFragDataLocation(m_LGlobalbuffer, 0, "fragment_colour");
	

	if (!Helpers::LinkProgramShaders(m_LGlobalbuffer))
		return false;

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, Per_Frame_UBO);
	glUniformBlockBinding(m_LGlobalbuffer, glGetUniformBlockIndex(m_LGlobalbuffer, "FrameUniforms"), 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, per_Model_UBO);
	glUniformBlockBinding(m_LGlobalbuffer, glGetUniformBlockIndex(m_LGlobalbuffer, "ModelUniforms"), 1);



	//CleanUp --------------------------------------------------------------------------------------------------------------



	// Done with the originals of these as we have made copies
	glDeleteShader(Ambient_Vertex_Shader);
	glDeleteShader(Ambient_Fragment_Shader);
	glDeleteShader(Light_Vertex_Shader);
	glDeleteShader(Light_Fragment_Vertex_Shader);
	glDeleteShader(GBuffer_Vertex_Shader);
	glDeleteShader(GBuffer_Fragment_Vertex_Shader);
	glDeleteShader(GlobalLightShader);
	glDeleteShader(GlobalLightShaderVertex);
	glDeleteShader(FXAAVertex);
	glDeleteShader(FXAAFragment);
	glDeleteShader(DepthVertex);
	glDeleteShader(DepthFragment);
	glDeleteShader(BlurFragment);
	glDeleteShader(BlurVertex);
	glDeleteShader(DepthOfFieldVertex);
	glDeleteShader(DepthOfFieldFragment);
	//Set textures to flip
	stbi_set_flip_vertically_on_load(true);

	

	Helpers::CheckForGLError();
	//Check for errors, If they are any then end the program
	return !Helpers::CheckForGLError();

	
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into FXAA_Program
	if (!CreateProgram())
		return false;

	
	//elemets
	GLuint indices[] =
	{
		0, 2, 1, // Left traingle
		3, 1, 2 // right traignle
	};

	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	//Position data
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,//1
		1.0f, -1.0f, 0.0f,//2
		-1.0f,  1.0f, 0.0f,//3
		1.0f,  1.0f, 0.0f,//4

		
	};

	//Uv / texcoord data
	static const GLfloat UVs[] = {
		0.0f, 0.0f, 1.0f,//0
		1.0f, 0.0f, 1.0f,//1
		0.0f,  1.0f, 1.0f,//2
		1.0f,  1.0f, 1.0f,//3

		
	};

	//Setup Vertex data, UBO, VAO, EBO , ETC

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint UBuffer;
	glGenBuffers(1, &UBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UVs), UVs, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	GLuint ElementEBO2;
	glGenBuffers(1, &ElementEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//Models -------------------------------------------------------------------------

	// Load in the jeep
	Helpers::ModelLoader loader_Jeep;
	if (!loader_Jeep.LoadFromFile("Data\\Models\\Jeep\\jeep.obj"))
		return false;

	Helpers::ImageLoader Jeep_Image;
	if (!Jeep_Image.Load("Data\\Models\\Jeep\\jeep_army.jpg"))
		return false;

	//bind the textures
	GLuint jeepImage;
	glGenTextures(1, &jeepImage);
	glBindTexture(GL_TEXTURE_2D, jeepImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Jeep_Image.Width(), Jeep_Image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Jeep_Image.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);
	
	CreateModel(loader_Jeep, jeepImage, glm::vec3(1000,10,500), glm::vec3(1,1,1));

	



	//Load in
	Helpers::ModelLoader Spp;
	if (!Spp.LoadFromFile("Data\\Models\\ship\\source\\spaceship.obj"))
		return false;

	Helpers::ImageLoader Sppimg;
	if (!Sppimg.Load("Data\\Models\\ship\\textures\\sp_1_color.png"))
		return false;

		
	//bind the textures
	GLuint Dp;
	glGenTextures(1, &Dp);
	glBindTexture(GL_TEXTURE_2D, Dp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Sppimg.Width(), Sppimg.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Sppimg.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);


	//CREATE AND ADD MODEL TO Pile
	CreateModel(Spp, Dp, glm::vec3(1150, 1000, -500), glm::vec3(100, 100, 100)); // model data, tex data, position matrix, scale matrix

	//rock
	Helpers::ModelLoader Rockl;
	if (!Rockl.LoadFromFile("Data\\Models\\Rock\\Rock.obj"))
		return false;

	Helpers::ImageLoader RockI;
	if (!RockI.Load("Data\\Models\\Rock\\Rocktex.jpeg"))
		return false;


	//bind the textures
	GLuint Dprock;
	glGenTextures(1, &Dprock);
	glBindTexture(GL_TEXTURE_2D, Dprock);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RockI.Width(), RockI.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, RockI.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	CreateModel(Rockl, Dprock, glm::vec3(-1000, 100, 0), glm::vec3(10, 10, 10));

	//orthogod 

	Helpers::ModelLoader oockl;
	if (!oockl.LoadFromFile("Data\\Models\\OrhtoFOCKM\\obj_1.obj"))
		return false;

	Helpers::ImageLoader oockI;
	if (!oockI.Load("Data\\Models\\OrhtoFOCKM\\Obj_1_in_uv.png"))
		return false;


	//bind the textures
	GLuint Dprock1;
	glGenTextures(1, &Dprock1);
	glBindTexture(GL_TEXTURE_2D, Dprock1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, oockI.Width(), oockI.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, oockI.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	CreateModel(oockl, Dprock1, glm::vec3(400, 200, 0), glm::vec3(10, 10, 10)); //4th model



	//5
	Helpers::ModelLoader mod;
	if (!mod.LoadFromFile("Data\\Models\\Apple\\apple.obj"))
		return false;

	Helpers::ImageLoader poockI;
	if (!poockI.Load("Data\\Models\\Apple\\2.jpg"))
		return false;


	//bind the textures
	GLuint Dprock12;
	glGenTextures(1, &Dprock12);
	glBindTexture(GL_TEXTURE_2D, Dprock12);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, poockI.Width(), poockI.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, poockI.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	CreateModel(mod, Dprock12, glm::vec3(60, 220, 50), glm::vec3(0.3, 0.3, 0.3)); //4th model

	
	//6 
	Helpers::ModelLoader odockl;
	if (!odockl.LoadFromFile("Data\\Models\\Apple\\Octotable.obj"))
		return false;

	Helpers::ImageLoader odockI;
	if (!odockI.Load("Data\\Models\\Ship\\textures\\sp_1_emit.png"))
		return false;


	//bind the textures
	GLuint Dpdrock1;
	glGenTextures(1, &Dpdrock1);
	glBindTexture(GL_TEXTURE_2D, Dpdrock1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, odockI.Width(), odockI.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, odockI.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	CreateModel(odockl, Dpdrock1, glm::vec3(60, 100, 50), glm::vec3(0.2, 0.2, 0.2)); //4th model

	glBindTexture(GL_TEXTURE_2D, 0);


	//22
	Helpers::ImageLoader odockI2;
	if (!odockI2.Load("Data\\Models\\Ship\\textures\\SPBLUE.png"))
		return false;


	//bind the textures
	GLuint Dpdrock12;
	glGenTextures(1, &Dpdrock12);
	glBindTexture(GL_TEXTURE_2D, Dpdrock12);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, odockI2.Width(), odockI2.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, odockI2.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	CreateModel(oockl, Dpdrock12, glm::vec3(100.0f, 1700.0f, -1.f), glm::vec3(10, 10, 10)); //4th model
	
	

	glBindTexture(GL_TEXTURE_2D, 0);

	//Cube










	//Create the terrain in the scene
	Terrain();


	//Create Texture Data
	GLuint JeepTex;

	//Create a Model(Structure that holds all the data necassry to render a model) 
	//details to the model struct can be found in renderer.h
	

	glBindTexture(GL_TEXTURE_2D, 0);
	//Lights

	Light Light1;
	Light Light2;

	Light1.Light_Position = glm::vec3(0, 1700, 0);
	Light1.Light_Range = 1;
	Light1.Light_Colour = glm::vec3(1, 1, 1);

	Light2.Light_Position = glm::vec3(500, 1000, 0);
	Light2.Light_Range = 0.2;
	Light2.Light_Colour = glm::vec3(0.6, 0.0, 0.0);

	m_Lights_Scene.push_back(Light1);
	m_Lights_Scene.push_back(Light2);


	
	
	//ETC
	
	//BUFFERS-----------------------------------------------



	glGenFramebuffers(1, &m_DepthOfFieldBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthOfFieldBuffer);




	glGenTextures(1, &DepthOfFieldImage);
	glBindTexture(GL_TEXTURE_2D, DepthOfFieldImage);

	//give empty timage to openGL


	//poor filtering 

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	//poor filtering 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint RenderBlur2;

	glGenRenderbuffers(1, &RenderBlur2);
	glBindRenderbuffer(GL_RENDERBUFFER, RenderBlur2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBlur2);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, DepthOfFieldImage, 0);



	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, Lbuffer_Colour_tex, 0);


	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, BlurredImage, 0);

	GLenum DrawBuffer11[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffer11);
	//Helpers::CheckForGLError();



	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);












	
	//DEPTH OF FIELD FRAME BUFFER (THAT OUTPUTS THE A BLUURED TEXTURE


	
	glGenFramebuffers(1, &m_BlurFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFrameBuffer);

	GLuint RenderBlur;

	
	

	glGenTextures(1, &BlurredImage);
	glBindTexture(GL_TEXTURE_2D, BlurredImage);

	//give empty timage to openGL


	//poor filtering 

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	//poor filtering 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	


	glGenRenderbuffers(1, &RenderBlur);
	glBindRenderbuffer(GL_RENDERBUFFER, RenderBlur);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBlur);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, BlurredImage, 0);
	
	

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, Lbuffer_Colour_tex, 0);


	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, BlurredImage, 0);

	GLenum DrawBuffer[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffer);
	//Helpers::CheckForGLError();



	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);






	
	
	//DepthMap FBO










	glGenFramebuffers(1, &DepthFramebuffer_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, DepthFramebuffer_FBO);

	

	glGenTextures(1, &DepthMap_texture);
	glBindTexture(GL_TEXTURE_2D, DepthMap_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float ClamColourp[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ClamColourp);


	glBindFramebuffer(GL_FRAMEBUFFER, DepthFramebuffer_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap_texture, 0);
	glDrawBuffer(GL_NONE); // Telling OPENGL we are not going to be rendering any colour data (only depth data) 
	glReadBuffer(GL_NONE); // Do this by setting these settings to none


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);



	glGenFramebuffers(1, &FXAA_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FXAA_FBO);


	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	//glCheckFramebufferStatus(FXAA_FBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	


	//renderbuffer
	glGenFramebuffers(1, &GBuffer_FBO);
	

	glGenTextures(1, &GBuffer_Pos_Tex);
	glGenTextures(1, &GBuffer_Normal_Tex);
	glGenTextures(1, &GBuffer_Depth_Tex);
	glGenTextures(1, &GBuffer_Colour_Tex);


	glGenFramebuffers(1, &Lbuffer_FBO);
	

	glGenTextures(1, &Lbuffer_Colour_tex);
	glGenRenderbuffers(1, &Lbuffer_colour_rbo); //15 19
	glBindRenderbuffer(GL_RENDERBUFFER, (GLuint)Lbuffer_colour_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);
	
	 //MIGHT HAVE TO DO WITH width and hieght not being what i think it is
	
	
	//textures

	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Depth_Tex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);


	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Pos_Tex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Normal_Tex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Colour_Tex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);


	// sahdoes, focul length and stuff go here (part 16 minutes week 5video ) look earlier for setup as well (incl anti aliastin

	glBindTexture(GL_TEXTURE_RECTANGLE, Lbuffer_Colour_tex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);


	//AATTACH STUFF TO THE TEXTURES for the programs

	GLenum framebuffer_Status = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer_FBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, GBuffer_Depth_Tex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, GBuffer_Pos_Tex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, GBuffer_Normal_Tex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, GBuffer_Colour_Tex, 0);

	//set draw buffers

	GLenum  bufs[] = { GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, bufs);

	//test framebuff

	glBindTexture(GL_TEXTURE_2D, 0);



	//light buffer

	glGenTextures(1, &Rendered_Texture_FXAA);

	glBindTexture(GL_TEXTURE_2D, Rendered_Texture_FXAA);

	//give empty timage to openGL

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	//poor filtering 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	//lbuffer 

	glBindFramebuffer(GL_FRAMEBUFFER, Lbuffer_FBO);

	glGenRenderbuffers(1, &LrenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, LrenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height); // used to be 1024, 768
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, LrenderBuffer);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, Lbuffer_Colour_tex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, GBuffer_Depth_Tex, 0); 

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Rendered_Texture_FXAA, 0);

	GLenum DrawBufferBlur[1] = { GL_COLOR_ATTACHMENT0 };
	//Helpers::CheckForGLError();

	
	
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);



	//Shadow mapping // Depth buffer FBO // getting depth data only from this FBO

	

	//Setup for Depth info

	//light space transformation matrixm, each worl space vector into space visible from light
	//lightSpaceMatrix = ProjectionMatrixLight * lightView;


	

	//std::cout << m_Models.size();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	
//	Helpers::CheckForGLError();
	return true;
}



// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{
	//
	//Helpers::CheckForGLError();c

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);










	// Clear buffers from previous frame



	// Structs of uniform data

	FrameUniforms PerFrameUniforms;
	ModelUniforms PerModelUniforms;









	//shadow mapping 

	//Depth texture buffer
	glViewport(0, 0, 1024, 1024);
	GLint viewportSize2[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize2);
	const float aspect_ratio2 = viewportSize2[2] / (float)viewportSize2[3];

	const static float far_plane_dist2 = 4000.0f;
	const static float near_plane_dist2 = 0.1f;

	glm::mat4 view_xform2 = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 projection_xform2 = glm::perspective(glm::radians(45.0f), aspect_ratio2, near_plane_dist2, far_plane_dist2);


	glBindFramebuffer(GL_FRAMEBUFFER, DepthFramebuffer_FBO);
	glUseProgram(m_DepthMapProg);



	
	//projection matrix
	glm::mat4 orthProjection = glm::perspective(glm::radians(45.0f), aspect_ratio2, near_plane_dist2, far_plane_dist2);

	float near_plane = 0.3f, far_plane = 4000.f;
	



	glm::mat4 lightView = glm::lookAt(glm::vec3(100.0f, 2700.0f, -1.f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));



	glm::mat4 lightSpaceMatrix;


	
	std::cout << lol1;
	lol1++;
	Helpers::CheckForGLError();

	//enable depth 
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	//if (shadows == true) {


		//std::cout << std::endl << "shadows true";


		for (int i = 0; i < m_Models.size(); i++) {
			for (int j = 0; j < m_Models[i].Model_Mesh.size(); j++) {
				//Draw Triangles

				float planeDiff = 2620.f;
				switch (i) {

				case 0:
					planeDiff = 2620.f;
					break;
				case 1:
					planeDiff = 2600.f;
					break;
				case 2:
					planeDiff = 2625.f;
					break;
				case 3:
					planeDiff = 2620.f;
					break;
				case 4:
					planeDiff = 2920.f;
					break;
				case 5:
					planeDiff = 2700.f;
					break;
				case 6:
					planeDiff = 0.f;
					break;

				default:
					planeDiff = 2720.f;
					break;



				}
				//std::cout << planeDiff;
				glm::mat4 lightProjection = glm::ortho(-2375.0f, 3575.0f, -2875.0f, 2375.0f, near_plane, planeDiff);


				lightSpaceMatrix = lightProjection * lightView;

				glUniformMatrix4fv(glGetUniformLocation(m_DepthMapProg, "LightProj"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

				glUniformMatrix4fv(glGetUniformLocation(m_DepthMapProg, "model"), 1, GL_FALSE, glm::value_ptr(m_Models[i].TranslationMatrix));
				//int f = 3;
				//int p = 0;
					//	glUniformMatrix4fv(glGetUniformLocation(m_DepthMapProg, "model"), 1, GL_FALSE, glm::value_ptr(m_Models[f].TranslationMatrix));
				glBindVertexArray(m_Models[i].Model_Mesh[j].VAO);
				glDrawElements(GL_TRIANGLES, m_Models[i].Model_Mesh[j].numElements, GL_UNSIGNED_INT, (void*)0);
			}
		}
	//}
	

	//ambient --------------------------------------------------------------------------------------------------------------------

	glViewport(0, 0, width, height);
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];

	//for projection_xform
	const static float far_plane_dist = 4000.0f;
	const static float near_plane_dist = 0.1f;

	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, near_plane_dist, far_plane_dist);

	PerModelUniforms.projection_xform = projection_xform;
	PerModelUniforms.view_xform = view_xform;





	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer_FBO); //bind to the framebuffer we created earlier called gbuffer

	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0f, .0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Helpers::CheckForGLError();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);

	//Define The camera Position and Ambient Light
	PerFrameUniforms.cam_Pos = camera.GetPosition();
	PerFrameUniforms.light_ambient = glm::vec3(0.0, 0.0, 0.0); //Change very gradually

	glUseProgram(m_Gbuffer);
	//Helpers::CheckForGLError();
	//loop throuhg all the models in m_models
	for (int i = 0; i < m_Models.size(); i++) {
		for (int j = 0; j < m_Models[i].Model_Mesh.size(); j++) {

			//Send information to the shader
			
			PerModelUniforms.model_xform = m_Models[i].TranslationMatrix;

			//std::cout << "RENDERING"  << std::endl << i << "NUMBER"  << std::endl;

			//bind unifrom buffers
			glBindBuffer(GL_UNIFORM_BUFFER, Per_Frame_UBO);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), &PerFrameUniforms, GL_STREAM_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, per_Model_UBO);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), &PerModelUniforms, GL_STREAM_DRAW);

			//Draw Triangles
			glBindVertexArray(m_Models[i].Model_Mesh[j].VAO);
			glDrawElements(GL_TRIANGLES, m_Models[i].Model_Mesh[j].numElements, GL_UNSIGNED_INT, (void*)0);
		}
	}

	//global light 

	//Helpers::CheckForGLError();
	glBindFramebuffer(GL_FRAMEBUFFER, Lbuffer_FBO);


	glBindTexture(GL_TEXTURE_2D, 0);
	
	//glClear(GL_DEPTH_BUFFER_BIT); attempt at global light, decided to disable

	/*
	glUseProgram(m_LGlobalbuffer);


	//Helpers::CheckForGLError();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Pos_Tex);
	Helpers::CheckForGLError();
	GLuint sampler_world_position_id = glGetUniformLocation(m_LGlobalbuffer, "sampler_world_position");
	Helpers::CheckForGLError();
	glUniform1i(sampler_world_position_id, 0);

	//Helpers::CheckForGLError();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Normal_Tex);
	GLuint sampler_world_normal_id = glGetUniformLocation(m_LGlobalbuffer, "sampler_world_normal");
	glUniform1i(sampler_world_normal_id, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Colour_Tex);
	GLuint sampler_world_tex_id = glGetUniformLocation(m_LGlobalbuffer, "sampler_tex_Col");
	glUniform1i(sampler_world_tex_id, 2);

	//PerFrameUniforms.Light_Pos = glm::vec3(100, 100, 100);
	//PerFrameUniforms.Light_Colour = glm::vec3(100, 100, 100);
	//PerFrameUniforms.light_Direction = glm::vec3(0, 0, 0);
	//PerFrameUniforms.light_Range = 100;

	//Helpers::CheckForGLError();
	//glBindBuffer(GL_UNIFORM_BUFFER, Per_Frame_UBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), &PerFrameUniforms, GL_STREAM_DRAW);

	//glBindBuffer(GL_UNIFORM_BUFFER, per_Model_UBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), &PerModelUniforms, GL_STREAM_DRAW);


	glDisable(GL_DEPTH_TEST);

	//Helpers::CheckForGLError();

	//unsure on what to render exactly
	glBindVertexArray(globalLightMesh.VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//glbind


	*/



	//Lights ---------------------------------------------------------------------------------------------------------------------


	//Change settings for Lighting
	glUseProgram(m_Lights);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_BLEND); //blends colours
	glBlendFunc(GL_ONE, GL_ONE);

	glClearColor(0.f, 0.f, 0.f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);




	PerFrameUniforms.cam_Pos = camera.GetPosition();
	PerFrameUniforms.light_ambient = glm::vec3(0.0, 0.0, 0.0); //Unused right now

	
	glActiveTexture(GL_TEXTURE0);
	GLuint texIDImage22 = glGetUniformLocation(m_Lights, "ShadowMap"); // pass in sahdow map from depth buffer
	glBindTexture(GL_TEXTURE_2D, DepthMap_texture);
	//glGenerateMipmap(GL_TEXTURE_2D);
	texIDImage22 = DepthMap_texture;


	for (int i = 0; i < m_Models.size(); i++) {
		for (int j = 0; j < m_Models[i].Model_Mesh.size(); j++) {


	
			
			
		

			//custom hard coded cause i cba
			float planeDiff = 2620.f;
			switch (i) {

			case 0:
				planeDiff = 2380.f;
				break;
			case 1:
				planeDiff = 2600.f;
				break;
			case 2:
				planeDiff = 2625.f;
				break;
			case 3:
				planeDiff = 2620.f;
				break;
			case 4:
				planeDiff = 2920.f;
				break;
			case 5:
				planeDiff = 700.f;
				break;
			case 6:
				planeDiff = 0.f;
				break;
			default:
				planeDiff = 2720.f;
				break;



			}

			if (shadows == false)
				planeDiff = 0;
			//std::cout << planeDiff;
			glm::mat4 lightProjection = glm::ortho(-2375.0f, 3575.0f, -2875.0f, 2375.0f, near_plane, planeDiff);


			lightSpaceMatrix = lightProjection * lightView;


			glUniformMatrix4fv(glGetUniformLocation(m_Lights, "LightProj"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));









			PerModelUniforms.model_xform = m_Models[i].TranslationMatrix;
			//Bind frame buffer
			glBindBuffer(GL_UNIFORM_BUFFER, per_Model_UBO);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(PerModelUniforms), &PerModelUniforms, GL_STREAM_DRAW);

			for (int k = 0; k < m_Lights_Scene.size(); k++) {                  //Lights

				//Send information about lights to shader
				PerFrameUniforms.Light_Pos = m_Lights_Scene[k].Light_Position;
				PerFrameUniforms.light_Range = m_Lights_Scene[k].Light_Range;
				PerFrameUniforms.Light_Colour = m_Lights_Scene[k].Light_Colour; //Change this to something called light colour, but keep light direction value for future

				//If the Model has a texture apply it 
				if (m_Models[i].Model_Mesh[j].textureId) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, m_Models[i].Model_Mesh[j].textureId);
					glUniform1i(glGetUniformLocation(m_Lights, "sampler_tex"), 1);

				}
				else {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, 0);
					glUniform1i(glGetUniformLocation(m_Lights, "sampler_tex"), 1); //Send a empty value
					std::cout << "texture load error" << std::endl;
				}



				//bind frame buffer 
				glBindBuffer(GL_UNIFORM_BUFFER, Per_Frame_UBO);
				glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameUniforms), &PerFrameUniforms, GL_STREAM_DRAW);

				//Render the triangles
				glBindVertexArray(m_Models[i].Model_Mesh[j].VAO);
				glDrawElements(GL_TRIANGLES, m_Models[i].Model_Mesh[j].numElements, GL_UNSIGNED_INT, (void*)0);


			}


		}


	};


	//BLur image FBO



	glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFrameBuffer);
	glUseProgram(m_BlurProgram);




	GLuint texIDImage = glGetUniformLocation(m_BlurProgram, "Texture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthMap_texture);
	//glGenerateMipmap(GL_TEXTURE_2D);
	texIDImage = DepthMap_texture;


	//render 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.0f, 0.f);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//Helpers::CheckForGLError();

	//Draw two triangles that cover screen, with texture of rendered scene on it

	glBindVertexArray(quad_VertexArrayID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);




	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);




	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthOfFieldBuffer);
	glUseProgram(m_DepthOffieldProgram);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.0f, 0.f);



	//defered apporach for depth of field
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Rendered_Texture_FXAA);
	GLuint texIDImageC = glGetUniformLocation(m_DepthOffieldProgram, "Texture");
	texIDImageC = Rendered_Texture_FXAA;
	glUniform1i(texIDImageC, 0);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, GBuffer_Pos_Tex);
	GLuint sampler_world_position_id = glGetUniformLocation(m_DepthOffieldProgram, "sampler_world_position");
	glUniform1i(sampler_world_position_id, 1);


	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, BlurredImage);
	GLuint texIDImageB = glGetUniformLocation(m_DepthOffieldProgram, "TextureBlur"); // pass in blurred image
	texIDImageB = BlurredImage;
	glUniform1i(texIDImageB, 2);
	
	




	glBindVertexArray(quad_VertexArrayID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);


	glBindVertexArray(0);




	//FXAA ------------------------------------------------------


	if (depthoffield == false) {
		GLuint texID = glGetUniformLocation(FXAA_Program, "Texture");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Rendered_Texture_FXAA);
		//glGenerateMipmap(GL_TEXTURE_2D);
		texID = Rendered_Texture_FXAA;
	}

	else {

		GLuint texID = glGetUniformLocation(FXAA_Program, "Texture");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DepthOfFieldImage);
		//glGenerateMipmap(GL_TEXTURE_2D);
		texID = DepthMap_texture;



	}




	glBindFramebuffer(GL_FRAMEBUFFER, FXAA_FBO);


	glUseProgram(FXAA_Program);



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.0f, 0.f);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	Helpers::CheckForGLError();

	//Draw two triangles that cover screen, with texture of rendered scene on it

	glBindVertexArray(quad_VertexArrayID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//}


	
	//use correct frame buffer

	if (FXAA == true) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FXAA_FBO);
}
	else
	{
		if (depthoffield == false)
			glBindFramebuffer(GL_READ_FRAMEBUFFER, Lbuffer_FBO);
		else
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_DepthOfFieldBuffer);
		}
	}


	if (Gbuffer == true)
		glBindFramebuffer(GL_READ_FRAMEBUFFER, GBuffer_FBO);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		

	glBlitFramebuffer(0, 0, viewportSize[2], viewportSize[3], 0, 0, viewportSize[2], viewportSize[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	//Helpers::CheckForGLError();
}




bool Renderer::Terrain()
{
	
	//size of the terrain
	float size{ 4000 };


	//numver of cells in terrain (more cells = higher quality but less performance)
	int numCellsXZ{ 64 };

	//calculate the size of each cell
	float cellSize = size / numCellsXZ;

	//calculate the numbers of verticies for each side
	int numVertsX = numCellsXZ + 1;
	int numVertsZ = numCellsXZ + 1;

	glm::vec3 start(-size / 2, 0, size / 2);

	//vectores to store vertices and UVCoordinates
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvCoords;

	//Load heightMap image
	Helpers::ImageLoader HeightMap;
	if (!HeightMap.Load("Data\\HeightMaps\\curvy.gif"))
		return false;

	//create unsinged char of heightmap data to be used for Y position
	unsigned char* texels = (unsigned char*)HeightMap.GetData();
	float tiles{ 10.0f };

	//loop throuhg the x and z of Terrain
	for (int z = 0; z < numVertsZ; z++) {
		for (int x = 0; x < numVertsX; x++)
		{
			glm::vec3 pos{ start };

			//get x and y position
			pos.x += x * cellSize;
			pos.z -= z * cellSize;


			//UV
			float u = (float)x / (numVertsX - 1);
			float V = (float)z / (numVertsX - 1);

			//Calcaute x and y of Heightmap
			int heightMapX = (int)(u * (HeightMap.Width() - 1));
			int heightMapY = (int)(V * (HeightMap.Height() - 1));

			//Create offset
			int offset = (heightMapX + heightMapY * HeightMap.Width()) * 4;
			pos.y = texels[offset];

			//add pos (position) to the verticies vector
			vertices.push_back(pos);

			//addd to uvcoordinates vector
			u *= tiles;
			V *= tiles;
			uvCoords.push_back(glm::vec2(u, V));

			//debug positions
			//std::cout << "vert at x,z " << x << "," << z << "position x,y,z" << pos.x << "," << pos.y << "," << pos.z << std::endl;
		}
	}

	//create Element vector and a toggle function so that the traingles can be in a diamond pattern
	std::vector<glm::uint> elements;
	bool toggle{ true };

	//loop throuhg z and x of terrain
	for (int cellZ = 0; cellZ < numCellsXZ; cellZ++) {
		for (int cellX = 0; cellX < numCellsXZ; cellX++) {
			int startVertIndex = cellZ * numVertsX + cellX;

			//create elements but toggle between them for diamond pattern
			if (toggle) {
				//first traingle
				elements.push_back(startVertIndex);
				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertsX);


				//second traiagnle
				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertsX + 1);
				elements.push_back(startVertIndex + numVertsX);
			}

			else {
				//first traingle
				elements.push_back(startVertIndex);
				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertsX + 1);


				//second traiagnle
				elements.push_back(startVertIndex);
				elements.push_back(startVertIndex + numVertsX + 1);
				elements.push_back(startVertIndex + numVertsX);

			}
			//switch
			toggle = !toggle;
		}
		//switch after full side done
		toggle = !toggle;
	}

	//create normal vector and fill normals with 0,0,0 as defualt
	std::vector<glm::vec3> normals(vertices.size());
	std::fill(normals.begin(), normals.end(), glm::vec3(0, 0, 0));


	//add to noramls for every element
	//+3 since 3 elements per traingle
	for (size_t index = 0; index < elements.size(); index += 3) {

		//first,second,third vertex in triangle
		int index1 = elements[index + 0];
		int index2 = elements[index + 1];
		int index3 = elements[index + 2];

		//get verticies
		glm::vec3 v0 = vertices[index1];
		glm::vec3 v1 = vertices[index2];
		glm::vec3 v2 = vertices[index3];

		glm::vec3 edg1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		//normalize
		glm::vec3 normal = glm::normalize(glm::cross(edg1, edge2));

		//add to normal vector
		normals[index1] += normal;
		normals[index2] += normal;
		normals[index3] += normal;
	}

	//normalize normals
	for (glm::vec3& n : normals)
		n = glm::normalize(n);

	//create VBOs

	//verts 
	GLuint PositionVBO;
	glGenBuffers(1, &PositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint NormalsVBO;
	glGenBuffers(1, &NormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, NormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint CoordsVBO;
	glGenBuffers(1, &CoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, CoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvCoords.size(), uvCoords.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ElementVBO;
	//elements
	glGenBuffers(1, &ElementVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elements.size(), elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//create VAO
	
	glGenVertexArrays(1, &GrassVao);
	glBindVertexArray(GrassVao);

	glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, //attribute
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, NormalsVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, //attribute
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);


	glBindBuffer(GL_ARRAY_BUFFER, CoordsVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementVBO);

	glBindVertexArray(0);

	//create Mesh for terrain
	ModelMesh TerrainMesh;
	TerrainMesh.VAO = GrassVao;
	TerrainMesh.numElements = (GLuint)elements.size();

	//Load an image to be used as terrain
	Helpers::ImageLoader image;
	if (image.Load("Data\\Textures\\grass11.bmp")) {
		glGenTextures(1, &GrassTexture);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);
		//add image to terrain textureID
		TerrainMesh.textureId = GrassTexture;
	}
	else
		std::cout << "texture load error" << std::endl;


	//create terrain model and add Terrain mesh to it
	Model TerrainModel;
	TerrainModel.name = "Terrain";
	TerrainModel.Model_Mesh.push_back(TerrainMesh);
	//add terrain model to m_model vector
	m_Models.push_back(TerrainModel);
	return true;
	
return false;
}




void Renderer::CreateModel(Helpers::ModelLoader &Model_C , GLuint &TextureM, glm::vec3 &PositionMat, glm::vec3 &ScaleMat){



	Model Model_Temp;
	Model_Temp.name = "Model";

	//load image of jeep
	//so that the textures can be wrapped correctly
	




	// Now we can loop through all the meshs in the loaded model:s
	// This is for the Jeep model
	for (const Helpers::Mesh& mesh : Model_C.GetMeshVector())
	{

		//go thourgh verticies of mode
		GLuint PositionVBO;
		glGenBuffers(1, &PositionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffer

		//elements vertex array
		GLuint ElementEBO;
		glGenBuffers(1, &ElementEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//normals
		GLuint NormalVBO;
		glGenBuffers(1, &NormalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, NormalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Textures 
		GLuint TextureVBO;
		glGenBuffers(1, &TextureVBO);
		glBindBuffer(GL_ARRAY_BUFFER, TextureVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Create a VAO
		GLuint VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 		//Set attribute pointer so can be used in The shaders

		//normals
		glBindBuffer(GL_ARRAY_BUFFER, NormalVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//textures
		glBindBuffer(GL_ARRAY_BUFFER, TextureVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//IDK why i did this
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementEBO);
		glBindVertexArray(0);


		//Create a ModelMesh(Struct) Which holds the VAO, Elemts and Texture ID
		ModelMesh newMesh;
		newMesh.VAO = VAO;
		newMesh.numElements = (GLuint)mesh.elements.size();
		newMesh.textureId = TextureM;

		//put the meshes into one vector
		Model_Temp.Model_Mesh.push_back(newMesh);

	}

	//Model positions

	glm::mat4 scaler = glm::scale(glm::mat4(1.0f), ScaleMat);
	Model_Temp.TranslationMatrix = glm::translate(glm::mat4(1.0f), PositionMat);
	Model_Temp.TranslationMatrix = Model_Temp.TranslationMatrix * scaler;

	glBindTexture(GL_TEXTURE_2D, 0);
	//Put the meshes into 
	
	m_Models.push_back(Model_Temp);


	//std::cout << "test";
}




