#include "libs.hh"
#include "game.hh"
#include "math.hh"






/// TODO LIST
/// 
/// Sunlight shadow maps, abstract it
/// ECS system
///		-> now you can abstract lights and stuff. Make all sorts of components! Lights, NPCs, ... go wild!
/// Search and scatter or whatever it's called for Physics
/// [ ] Deferred Shading
/// 
/// [ ] Colored lights
/// Blinn-phong and regular phong lighting? Which looks better?
/// Collision boxes
/// Raycast bounce gun thing
/// 
int main()
{
	/// TODO: This should be stored in an .ini file
	const int glVersionMajor = 4;
	const int glVersionMinor = 5;
	const int WINDOW_WIDTH = 1920; // THESE CANNOT BE LEFT INITIALIZED TO ZERO!!
	const int WINDOW_HEIGHT = 1080;
	int framebufferWidth = WINDOW_WIDTH;
	int framebufferHeight = WINDOW_HEIGHT;

	Game game("Spelunking Robot Spanish Inqusition Apostle Simulator v2021.7", WINDOW_WIDTH, WINDOW_HEIGHT, glVersionMajor, glVersionMinor);

	
	// :^|
/*	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);
	
	const unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;
	unsigned int shadowMap;
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		shadowMapWidth,
		shadowMapHeight,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE); // No reason to draw the color data of the shadow buffer
	glReadBuffer(GL_NONE); // ...or read it
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	glm::mat4 orthogonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f); // Larger orthos provide more blurry shadows
	glm::mat4 lightView = glm::lookAt(20.0f * lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = orthogonalProjection * lightView;

	
	Shader* shadows = new Shader(glVersionMajor, glVersionMinor, "vertex_shadow.glsl", "fragment_shadow.glsl");
	shadows->SetMat4fv(lightProjection, "lightProjection");
	shadows->Use();

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight); // THIS WILL SWITCH YOUR VIEWPORT TO THE SHADOW MAP.
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	model.Draw(shadowMapProgram, camera);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	


	while (!game.GetWindowShouldClose())
	{
		game.Update();
		game.Render();
	}

	return 0;
}