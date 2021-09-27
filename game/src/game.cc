#include "game.hh"

#define DR_WAV_IMPLEMENTATION
#include "sound/dr_wav.h"

void Game::_InitGLFW()
{
	if (glfwInit() == GLFW_FALSE)
	{
		DEBUG_LOG("Renderer", LOG_FATAL, "GLFW initialization failed!");
		glfwTerminate();
	}
}

void Game::_InitWindow(const char* title, bool resizeable)
{
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _GL_VERSION_MINOR);
	glfwWindowHint(GLFW_RESIZABLE, resizeable);
	glfwWindowHint(GLFW_SAMPLES, 4); // MSAA
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	_window = glfwCreateWindow(_WINDOW_WIDTH, _WINDOW_HEIGHT, title, NULL, NULL);

	if (_window == nullptr)
	{
		DEBUG_LOG("Renderer", LOG_FATAL, "GLFW window initialization failed!");
		glfwTerminate();
	}

	glfwGetFramebufferSize(_window, &_framebufferWidth, &_framebufferHeight);
	glfwSetFramebufferSizeCallback(_window, Game::framebuffer_resize_callback);
	glfwSetScrollCallback(_window, Game::scrollwheel_callback);

	// All OpenGL functions, including GLEW, need this before they're usable
	glfwMakeContextCurrent(_window);
}

/// DO NOT CALL InitGLEW() BEFORE glfwMakeContextCurrent(...) -- which is called in InitWindow()
void Game::_InitGLEW()
{
	glewExperimental = GLEW_EXPERIMENTAL;

	if (glewInit() != GLEW_OK)
	{
		DEBUG_LOG("Renderer", LOG_FATAL, "GLEW initialization failed!");
		glfwTerminate();
	}
}

void Game::_InitGLFlags()
{
	glEnable(GL_MULTISAMPLE);

	// OpenGL flags
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glCullFace(GL_FRONT); // TODO: Only do frontface culling when calculating lighting pass in deferred shading model
	glFrontFace(GL_CW);
	//glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Input flags
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Disable VSync
//	glfwSwapInterval(0);
}

void Game::_InitMatrices()
{
	_viewMatrix = glm::mat4(1.0f);
	_viewMatrix = glm::lookAt(_camPosition, _camPosition + _camFront, _worldUp);

	_projectionMatrix = glm::mat4(1.0f);
	_projectionMatrix = glm::perspective(
		glm::radians(_fov),
		static_cast<float>(_framebufferWidth) / _framebufferHeight,
		_nearPlane,
		_farPlane
	);
}

void Game::_InitShaders()
{
	_shaders.push_back(new Shader(
		_GL_VERSION_MAJOR,
		_GL_VERSION_MINOR,
		"res/shaders/vertex_core.glsl",
		"res/shaders/fragment_core.glsl"
	));

	_shaders.push_back(new Shader(_GL_VERSION_MAJOR, _GL_VERSION_MINOR, "res/shaders/shadow.vert", "res/shaders/shadow.frag"));

//	_shaders.push_back(new Shader(
//		_GL_VERSION_MAJOR,
//		_GL_VERSION_MINOR,
//		"shaders/vertex_shadow.glsl",
//		"shaders/fragment_shadow.glsl"
//	));
}

void Game::_InitFramebuffers()
{
/*	FramebufferType type;
	type.width = 2048;
	type.height = 2048;
	type.samples = 4;
	_framebuffers.push_back(new Framebuffer(
		type,
		glm::vec3(0.0f, 4.0f, 4.0f)
	));*/
	

	
}

void Game::_InitTextures()
{
	_textures.push_back(new Texture("res/textures/models/matoshi.png", GL_TEXTURE_2D));
	_textures.push_back(new Texture("res/textures/models/matoshi_emissive.png", GL_TEXTURE_2D));
}

void Game::_InitMaterials()
{
	_materials.push_back(new Material(
		glm::vec3(0.1f),
		glm::vec3(1.0f),
		glm::vec3(1.0f),
		0,
		1,
		1
	));
}

void Game::_InitModels()
{
//	std::vector<Mesh*> meshes;
//	std::vector<Mesh*> meshes2;

//	std::vector<Vertex> matoshi = ImportOBJ("res/models/matoshi.obj");
//	meshes.push_back(new Mesh(
//		matoshi.data(), matoshi.size(),
//		NULL, 0,
//		glm::vec3(0.0f, -1.0f, -5.0f),
//		glm::vec3(0.0f),
//		glm::vec3(0.0f),
//		glm::vec3(1.0f)
//	));

//	Primitive* quad = new Primitive();

//	Vertex planeVertices[] = {
//		// Position | Color | TexCoords | Normals
//		glm::vec3(-25.0f, 0.0f, -25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
//		glm::vec3(-25.0f, 0.0f,  25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f,  0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
//		glm::vec3(25.0f, 0.0f,  25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f,  0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
//		
//		glm::vec3(25.0f, 0.0f, -25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
//		glm::vec3(-25.0f, 0.0f, -25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
//		glm::vec3(25.0f, 0.0f,  25.0f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f,  0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
//	};

//	quad->Set(planeVertices, (sizeof(planeVertices) / sizeof(Vertex)), 0, NULL);

//	meshes2.push_back(new Mesh(
//		quad,
//		glm::vec3(0.0f, -1.0f, 0.0f),
//		glm::vec3(0.0f, -1.0f, 0.0f),
//		glm::vec3(0.0f, 0.0f, 0.0f),
//		glm::vec3(1.0f)
//	));

//	delete quad;

	// Convex hull experimentation
	//		QuickHull qh;
	//		std::vector<glm::vec3> pointCloud = ImportOBJVertices("res/models/matoshi.obj");
	//		ConvexHull hull = qh.GetConvexHull(pointCloud, false);
	//		hull.WriteWaveformOBJ("res/models/convex_hull.obj");
	//		/// Next step: make VertexDataSource return a vec3 array or something so you can feed it into Mesh constructor!!
		
		


	// Loading skinned meshes
	//	kotlin.loadMesh("res/models/bob_lamp.md5mesh");
	//	DEBUG_LOG("KOTLIN", LOG_WARN, "NUM BONES: %i", kotlin.numBones());






	//	std::vector<Vertex> PCVB2;
	//	for (size_t x = 0; x < PCVB.size(); x++)
	//	{
	//		Vertex v;
	//		v.position = PCVB[x];
	//		v.color = glm::vec3(1.0f);
	//		v.normal = glm::vec3(0.0f); // Could be a problem...?
	//		v.texcoord = glm::vec2(0.0f);
	//		DEBUG_LOG("TEMP", LOG_WARNING, "Logged vertex position for iteration %i as [%f, %f, %f]", x, PCVB[x].x, PCVB[x].y, PCVB[x].z);
	//		PCVB2.push_back(v);
	//	}
	//
	//	meshes.push_back(new Mesh(
	//		PCVB2.data(),
	//		PCVB2.size(),
	//		NULL,
	//		0,
	//		glm::vec3(0.0f, 0.0f, 0.0f),
	//		glm::vec3(0.0f),
	//		glm::vec3(0.0f),
	//		glm::vec3(1.0f)
	//	));



//	_models.push_back(new Model(
//		glm::vec3(0.0f),
//		_materials[0],
//		_textures[0],
//		_textures[1],
//		meshes
//	));
//
//	_models.push_back(new Model(
//		glm::vec3(0.0f),
//		_materials[0],
//		_textures[1],
//		_textures[1],
//		meshes2
//	));
//
//	for (auto*& i : meshes)
//	{
//		delete i;
//	}
//	for (auto*& i : meshes2)
//	{
//		delete i;
//	}
}

void Game::_InitPointLights()
{
	_pointLights.push_back(new PointLight(glm::vec3(0.0f, 4.0f, 4.0), glm::vec3(1.0f, 1.0f, 1.0f)));
}

void Game::_InitLights()
{
	_InitPointLights();

	
}

void Game::_InitUniforms()
{
	_shaders[SHADER_CORE_PROGRAM]->Use();
	
	// Send matrices to shader files
	// *Model matrix is handled by an individual Mesh class
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_viewMatrix, "viewMatrix");
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_projectionMatrix, "projectionMatrix");

	for (auto* pl : _pointLights)
	{
		pl->SendToShader(*(_shaders[SHADER_CORE_PROGRAM]));
	}
	
}

void Game::_InitECS()
{
	// ECS must be initialized in this order: first components, then entities, then systems
	

	// Components
	
//	TransformComponent transformComponent;
//	transformComponent.transform.SetPosition(glm::vec3(0.0f, 0.0f, 7.0f));

//	MovementControlComponent movementControl;
//	movementControl.movementControls.push_back(std::make_pair(glm::vec3(1.0f, 0.0f, 0.0f) * 10.0f, &_ic_x));
//	movementControl.movementControls.push_back(std::make_pair(glm::vec3(0.0f, 0.0f, 1.0f) * 10.0f, &_ic_z));

	// Entities
	
//	_entity = _ecs.MakeEntity(transformComponent, movementControl);

	// Systems
	
//	MovementControlSystem movementControlSystem;
//	_ecsMainSystems.AddSystem(movementControlSystem);

	//	RenderableMeshSystem renderableMeshSystem(gameRenderContext);
	//	_ecsRenderingPipeline.AddSystem(renderableMeshSystem);

}
/*
/// Updates VP matrices as rendered from Camera and sends their data to SHADER_CORE.
/// Call Shader::Use() on SHADER_CORE first.
void Game::_UpdateCameraUniforms()
{
	_ecs.UpdateSystems(_ecsMainSystems, _TICK_RATE);

	Transform& workingTransform = _ecs.GetComponent<TransformComponent>(_entity)->transform;
//	workingTransform.SetRotation(Quaternion(glm::vec3(1.0f, 1.0f, 1.0f).Normalized(), _amt * 10.0f / 11.0f));
	workingTransform.SetPosition(_camera.GetPosition());
#ifdef _DEBUG
		std::cout << "Memory addr of mat4:viewMatrix OUTSIDE func: " << &_viewMatrix << "\n";
#endif
		workingTransform.GetViewMatrix(&_viewMatrix, &_camera);
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_viewMatrix, "viewMatrix");
									_shaders[SHADER_CORE_PROGRAM]->SetVec3f(_camera.GetPosition(), "camPosition");
	///
		glfwGetFramebufferSize(_window, &_framebufferWidth, &_framebufferHeight);
		for (auto* pl : _pointLights)
		{
			pl->SendToShader(*(_shaders[SHADER_CORE_PROGRAM]));
		}
	///
#ifdef _DEBUG
		std::cout << "Memory addr of mat4:projectionMatrix OUTSIDE func: " << &_projectionMatrix << "\n";
#endif
		workingTransform.GetPerspectiveMatrix(
			&_projectionMatrix, 
			_fov,
			static_cast<float>(_framebufferWidth)/_framebufferHeight, 
			_nearPlane, 
			_farPlane
		);
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_projectionMatrix, "projectionMatrix");

	


	
}*/

/// Updates VP matrices as rendered from Camera and sends their data to SHADER_CORE.
/// Call Shader::Use() first.
void Game::_UpdateUniforms()
{
	// Update view matrix
	_viewMatrix = _camera.GetViewMatrix();
// 	_viewMatrix = _workingTransform
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_viewMatrix, "viewMatrix"); // Send
	_shaders[SHADER_CORE_PROGRAM]->SetVec3f(_camera.GetPosition(), "camPosition"); // Send cam pos to shader files (for specular light)
	

	// TODO: THIS IS REALTIME LIGHTING LOCATION UPDATING.
	// In the future, only send this information if a light pos has been updated
	for (auto* pl : _pointLights)
	{
		pl->SendToShader(*(_shaders[SHADER_CORE_PROGRAM]));
	}

	// Update framebuffer...
	glfwGetFramebufferSize(_window, &_framebufferWidth, &_framebufferHeight);

	// ...THEN update perspective matrix!
	_projectionMatrix = glm::mat4(1.0f); // Reset
	_projectionMatrix = glm::perspective( /// This keeps fucking crashing when I minimize the window
		glm::radians(_fov),
		static_cast<float>(_framebufferWidth) / _framebufferHeight,
		_nearPlane,
		_farPlane
	);
	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_projectionMatrix, "projectionMatrix"); // Send

	
}

void Game::_UpdateDeltaTime()
{
	_currentTime = static_cast<float>(glfwGetTime());
	_deltaTime = _currentTime - _previousTime;
	_previousTime = _currentTime;
}

void Game::_UpdateInputMouse()
{
	glfwGetCursorPos(_window, &(_mouseX), &(_mouseY));

	if (_firstMouse)
	{
		_lastMouseX = _mouseX;
		_lastMouseY = _mouseY;
		_firstMouse = false;
	}

	// Calculate offset
	_mouseOffsetX = _mouseX - _lastMouseX;
	_mouseOffsetY = _lastMouseY - _mouseY;

	// Set the last X and Y
	_lastMouseX = _mouseX;
	_lastMouseY = _mouseY;
}

void Game::_UpdateInputKeyboard()
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(_window, GLFW_TRUE);
	}


	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		_camera.Translate(_deltaTime, DIRECTION::FORWARD);
	}
	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		_camera.Translate(_deltaTime, DIRECTION::LEFT);
	}
	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		_camera.Translate(_deltaTime, DIRECTION::BACK);
	}
	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		_camera.Translate(_deltaTime, DIRECTION::RIGHT);
	}


	if (glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		_models[0]->Rotate(glm::vec3(0.0f, 8.0f, 0.0f));
	}
	if (glfwGetKey(_window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		_models[0]->Scale(glm::vec3(0.1f));
	}
	if (glfwGetKey(_window, GLFW_KEY_U) == GLFW_PRESS)
	{
		_models[0]->Scale(glm::vec3(-0.1f));
	}


	if (glfwGetKey(_window, GLFW_KEY_2) == GLFW_PRESS)
	{
		_fov -= 1.0f;
	}
	if (glfwGetKey(_window, GLFW_KEY_3) == GLFW_PRESS)
	{
		_fov += 1.0f;
	}

	if (glfwGetKey(_window, GLFW_KEY_EQUAL) == GLFW_PRESS)
	{
		_camera.SetMovementSpeed(_camera.GetMovementSpeed() + 0.01f);
	}
	if (glfwGetKey(_window, GLFW_KEY_MINUS) == GLFW_PRESS)
	{
		_camera.SetMovementSpeed(_camera.GetMovementSpeed() - 0.01f);
	}
}

void Game::_UpdateInput(GLFWwindow* window)
{
	_UpdateInputMouse();
	_UpdateInputKeyboard();
	_camera.UpdateInput(_deltaTime, -1, _mouseOffsetX, _mouseOffsetY);
}

void Game::_UpdateInput(GLFWwindow* window, Texture* tex)
{
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		tex->SetTexture("res/edit32.png", true);
	}
}

void Game::_InitSoundSystem()
{
	// Find the default audio device
	const ALCchar* defaultDeviceStr = alcGetString(/*device*/nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceStr);
	if (!device)
	{
		DEBUG_LOG("SoundSystem", LOG_FATAL, "Failed to get the default audio device for OpenAL");
		return;
	}
	DEBUG_LOG("SoundSystem", LOG_SUCCESS, "OpenAL device: %s", alcGetString(device, ALC_DEVICE_SPECIFIER));

	// Create an OpenAL audio context for the device
	ALCcontext* context = alcCreateContext(device, /*attribute list*/nullptr);
	OpenAL_ErrorCheck(device);
	OpenAL_ErrorCheck(context);

	// Activate the context so OpenAL state modifications can are applied to it
	if (!alcMakeContextCurrent(context))
	{
		DEBUG_LOG("SoundSystem", LOG_FATAL, "Failed to make the OpenAL context the current context");
		return;
	}
	OpenAL_ErrorCheck("Make context current");

	// Create listener
	alec(alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f));
	alec(alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f));
	ALfloat forwardAndUpVectors[] =
	{
		/* FORWARD */ 1.0f, 0.0f, 0.0f,
		/* UP */	  0.0f, 1.0f, 0.0f
	};
	alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

	// Create buffers that hold the sound data
	// These are shared between contexts and are defined at device level

	struct ReadWavData
	{
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
		drwav_uint64 totalPCMFrameCount = 0;
		std::vector<uint16_t> pcmData;

		inline constexpr drwav_uint64 GetTotalSamples()
		{
			return totalPCMFrameCount * channels;
		}
	};

	ReadWavData monoData;

	{
		drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(
			"res/sounds/music/the_other_side.wav",
			&monoData.channels,
			&monoData.sampleRate,
			&monoData.totalPCMFrameCount,
			nullptr
		);

		if (pSampleData == NULL)
		{
			DEBUG_LOG("SoundSystem", LOG_ERROR, "Failed to load audio file!");
		}

		if (monoData.GetTotalSamples() > drwav_uint64(std::numeric_limits<size_t>::max()))
		{
			DEBUG_LOG("SoundSystem", LOG_ERROR, "Too much data in file for 32-bit addressed vector");
			drwav_free(pSampleData, nullptr);

			return; // TODO
		}

		monoData.pcmData.resize(size_t(monoData.GetTotalSamples()));

		// Copy the sample data into the vector data
		// memcpy works on bytes so multiply size by 2 (two bytes in short16 format)
		std::memcpy(monoData.pcmData.data(), pSampleData, monoData.pcmData.size() * 2);
		
		drwav_free(pSampleData, nullptr);
	}

	ALuint monoSoundBuffer;
	alec(alGenBuffers(1, &monoSoundBuffer));
	alec(alBufferData(
		monoSoundBuffer,
		monoData.channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
		monoData.pcmData.data(),
		monoData.pcmData.size() * 2, // Two bytes per sample!
		monoData.sampleRate
	));

	// Create a sound source that plays the wav from the sound buffer
	ALuint monoSource;
	alec(alGenSources(1, &monoSource));
	alec(alSource3f(monoSource, AL_POSITION, 0.0f, 0.0f, 0.0f));
	alec(alSource3f(monoSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
	alec(alSourcef(monoSource, AL_PITCH, 1.0f));
	alec(alSourcef(monoSource, AL_GAIN, 1.0f));
	alec(alSourcei(monoSource, AL_LOOPING, AL_FALSE));
	alec(alSourcei(monoSource, AL_BUFFER, monoSoundBuffer));

	// Play the sound!
	alec(alSourcePlay(monoSource));
	ALint sourceState;
	alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState));
	while (sourceState == AL_PLAYING)
	{
		// Basically loop until we're doing playing the monoSource
		alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState));
	}

	// OpenAL cleanup
	alec(alDeleteSources(1, &monoSource));
	alDeleteSources(1, &monoSoundBuffer);
	alcMakeContextCurrent(nullptr);
	alec(alcCloseDevice(device));










}

// Constructors
Game::Game(
	const char* title,
	const int WINDOW_WIDTH,
	const int WINDOW_HEIGHT,
	const int GL_VERSION_MAJOR,
	const int GL_VERSION_MINOR)
	:
	_WINDOW_WIDTH(WINDOW_WIDTH),
	_WINDOW_HEIGHT(WINDOW_HEIGHT),
	_GL_VERSION_MAJOR(GL_VERSION_MAJOR),
	_GL_VERSION_MINOR(GL_VERSION_MINOR),
	_camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
{
	_window = nullptr;
	_framebufferWidth = _WINDOW_WIDTH;
	_framebufferHeight = _WINDOW_HEIGHT;


	_camPosition = glm::vec3(0.0f, 0.0f, 1.0f);
	_worldUp = glm::vec3(0.0f, 1.0f, 0.0);
	_camFront = glm::vec3(0.0f, 0.0f, -1.0f);
	_fov = 90.0f;
	_nearPlane = 0.1f;
	_farPlane = 10000000.0f;

	_deltaTime = 0.0f;
	_currentTime = 0.0f;
	_previousTime = 0.0f;

	_lastTime = glfwGetTime();
	_numFrames = 0;

	_lastMouseX = 0.0;
	_lastMouseY = 0.0;
	_mouseX = 0.0;
	_mouseY = 0.0;
	_mouseOffsetX = 0.0;
	_mouseOffsetY = 0.0;
	_firstMouse = true;

	_InitGLFW();
	_InitWindow(title, true);
	_InitGLEW();
	_InitGLFlags();
	_InitMatrices();
	_InitShaders(); // Framebuffers must be after shaders.
	_InitFramebuffers();

	_InitTextures();
	_InitMaterials();
	_InitModels();
	_InitLights(); // Init lights first, THEN send to uniforms!
	_InitUniforms(); // This activates shaders and sends values to core, should be called near the end

	_InitECS();

	_InitSoundSystem();

	_TestFunction();
}

Game::~Game()
{
	glfwDestroyWindow(_window);
	glfwTerminate();

	for (size_t i = 0; i < _shaders.size(); i++)
		delete _shaders[i];

	for (size_t i = 0; i < _textures.size(); i++)
		delete _textures[i];

	for (size_t i = 0; i < _materials.size(); i++)
		delete _materials[i];

	for (auto*& i : _models)
		delete i;

	for (size_t i = 0; i < _pointLights.size(); i++)
		delete _pointLights[i];

	for (size_t i = 0; i < _framebuffers.size(); i++)
		delete _framebuffers[i];
}

// Accessors
int Game::GetWindowShouldClose()
{
	return glfwWindowShouldClose(_window);
}

// Modifiers
void Game::SetWindowShouldClose()
{
	glfwSetWindowShouldClose(_window, GL_TRUE);
}

// Public functions
/// Should be called every frame when possible as it polls events and input
void Game::Update()
{
	// FPS counter
	double currentTime = glfwGetTime(); // TODO: TEST FOR OVERFLOW
	_numFrames++;
	if (currentTime - _lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
		printf("%f ms/frame (%i FPS)\n", 1000.0f / double(_numFrames), _numFrames);
		_numFrames = 0;
		_lastTime += 1.0;
	}

	glfwPollEvents();

	_UpdateDeltaTime();
	// Update input
	if (currentTime - _lastTime >= (1.0f / 30.0f))
	{	
		_UpdateInput(_window);
		_UpdateInput(_window, _textures[0]);
		//	_pointLights[0]->SetPosition(glm::vec3(8.0f * std::cos(_currentTime), 1.0f, 8.0f * std::sin(_currentTime)));
	}
}

void Game::_TestFunction()
{
	glGenFramebuffers(1, &_shadowMapFBO);

	glGenTextures(1, &_shadowMapID);
	glBindTexture(GL_TEXTURE_2D, _shadowMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _shadowMapWidth, _shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	float clampColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, _shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMapID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// Light matrices orthographic projection
	glm::mat4 orthogonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
	glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 8.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_lightProjection = orthogonalProjection * lightView;


	// Init uniforms (move later)
	_shaders[1]->SetMat4fv(_lightProjection, "lightProjection");















/**	POST PROCESSING, USE LATER

	float rectangleVertices[] =
	{
		// Coords		// TexCoords
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	// frame buffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// frame buffer texture
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _WINDOW_WIDTH, _WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	// render buffer object
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _WINDOW_WIDTH, _WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << "\n";
	// 36054: Not all framebuffer attachment points are framebuffer-attachment-complete.
	//			At least one attachment point with renderbuffer or texture attached has its attached object
	// 			-	No longer in existence
	// 			-	Has an attached image with width or height = 0
	// 			-	Color attachment point has a non-color-renderable image attached
	// 			-	Depth attachment point has a non-depth-renderable image attached
	// 			-	Stencil attachment point has a non-stencil-renderable image attached
	// 36057: Not all attached images have same width & height
	// 36055: No images are attached to framebuffer
	// 36061: Combination of internal formats of attached images violates an implementation-dependent set of restrictions



	_shaders.push_back(new Shader(_GL_VERSION_MAJOR, _GL_VERSION_MINOR, "shaders/framebuffer.vert", "shaders/framebuffer.frag"));

	_shaders[1]->Use();
	_shaders[1]->Set1i(1, "screenTexture");**/
}

void Game::Render()
{
//	for (auto& i : _textures)
//	{
//		std::cout << "Screaming in all dimensions: " << i->GetID() << "\n";
//	}
//	std::cout << "and the framebuffer " << framebufferTexture << "\n";

	// First, make sure to bind the framebuffer before ANYTHING is drawn, including a ClearColor background
//			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, _shadowMapWidth, _shadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, _shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (auto& x : _models)
			x->Draw(_shaders[1]); // Draw into shadow shader!
	glBindFramebuffer(GL_FRAMEBUFFER, 0);





	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Reset viewport
	glViewport(0, 0, _framebufferWidth, _framebufferHeight); /// Should be removed later
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

	_shaders[SHADER_CORE_PROGRAM]->Use();


	_UpdateUniforms(); // Update matrices related to drawing from the camera -- this is done before drawing models for obvious reasons


	_materials[MAT1]->SendToShader(*(_shaders[SHADER_CORE_PROGRAM]));




	

	_shaders[SHADER_CORE_PROGRAM]->SetMat4fv(_lightProjection, "lightProjection");
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, _shadowMapID);
//d	std::cout << "shadowMapID: " << shadowMapID << "\n";
	_shaders[SHADER_CORE_PROGRAM]->Set1i(2, "shadowMap");



	_textures[TEX_ROCK32]->Bind(0);
	_textures[TEX_ROCK32_SPEC]->Bind(1);

	for (auto& x : _models)
		x->Draw(_shaders[SHADER_CORE_PROGRAM]); // Draw into core shader!

	// REMOVE
	kotlin.Draw(_shaders[SHADER_CORE_PROGRAM]);


	_shaders[SHADER_CORE_PROGRAM]->UnUse();

//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//			_shaders[1]->Use();
//			glBindVertexArray(rectVAO);
//			glDisable(GL_DEPTH_TEST); // Make sure the post-processing rectangle doesn't fill the depth test
//			glBindTexture(GL_TEXTURE_2D, framebufferTexture);
//				glDisable(GL_CULL_FACE);
//			glDrawArrays(GL_TRIANGLES, 0, 6);
//				glEnable(GL_CULL_FACE);


	// End Draw -- cleanup on aisle 6
	glfwSwapBuffers(_window);
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_TEXTURE_2D, 0);

/*
	// Update

	// Clear
	glClearColor(0.4862745f, 0.7647058f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Update uniforms
	_UpdateUniforms();
	_materials[MAT1]->SendToShader(*(_shaders[SHADER_CORE_PROGRAM]));

	// Use the program -- should be AFTER calling Set functions from the shader class, since they Use and UnUse the program!!
	_shaders[SHADER_CORE_PROGRAM]->Use();
	

	// Activate a texture
	_textures[TEX_ROCK32]->Bind(0);
	_textures[TEX_ROCK32_SPEC]->Bind(1);

	// Draw
	for (auto& x : _models)
	{
		x->Render(_shaders[SHADER_CORE_PROGRAM]);
	}

	// End Draw -- cleanup on aisle 6
	glfwSwapBuffers(window);
	glFlush();

	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_TEXTURE_2D, 0);*/
}


// Static functions
void Game::framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight)
{
	std::cout << "Framebuffer resize callback called.\n";
	glViewport(0, 0, fbWidth, fbHeight);
}

void Game::scrollwheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}