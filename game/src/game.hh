#pragma once

#include "libs.hh"

#include "common.hh"

#include "ecs/ecs.hh"
#include "renderer/transform.hh"
#include "input_control.hh"

#include "physics/quickhull.hh"
#include "renderer/skinned_mesh.hh"

#define GLEW_EXPERIMENTAL GL_TRUE

#define _DEBUG 1

enum ShaderEnum { SHADER_CORE_PROGRAM = 0 };
enum TextureEnum { TEX_ROCK32 = 0, TEX_ROCK32_SPEC };
enum MaterialEnum { MAT1 = 0 };


struct TransformComponent : public ECSComponent<TransformComponent>
{
	Transform transform;
};

struct MovementControlComponent : public ECSComponent<MovementControlComponent>
{
	std::vector<std::pair<glm::vec3, InputControl*>> movementControls;
};

class MovementControlSystem : public BaseECSSystem
{
private:

public:
	MovementControlSystem() : BaseECSSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(MovementControlComponent::ID);
	}

	virtual void UpdateComponents(float delta, BaseECSComponent** components)
	{
		TransformComponent* transform = (TransformComponent*)components[0];
		MovementControlComponent* movementControl = (MovementControlComponent*)components[1];

		for (uint32_t i = 0; i < movementControl->movementControls.size(); i++)
		{
			glm::vec3 movement = movementControl->movementControls[i].first;
			InputControl* input = movementControl->movementControls[i].second;
			glm::vec3 newPos = (transform->transform.GetPosition()) + (movement * input->GetAmt() * delta);
			transform->transform.SetPosition(newPos);
		}
	}
};

enum SOUND_EVENT
{
	ZERO,
	ONLY_PLAY_ON_FUNCTION_CALL,
	UI_CLICK,
	PLAYER_WALKS,
	PLAYER_SPRINTS,
	PLAYER_JUMPS,
};

// How to get res dir?
// Should I associate one with each SOUND_EVENT enum? <-- Actually no. Sound events are only for detecting WHEN sounds should play.
//															Now that I think about it, perhaps have two enums. One for sound EVENTS (WHEN) and one for sound TYPES (WHAT)
//															Then the sound TYPES can have an association!!
//		so like std::string resDir = "res/sounds/" + SoundRegistry::GetFilePath(SOUND_TYPE);
//		and then a function somewhere else 

struct SoundComponent : public ECSComponent<SoundComponent>
{
	uint16_t soundEvent; // WHEN the sound should play
	uint16_t soundType; // WHAT sound to play
	std::string resourcePath; // directory of soundType -- determined by functions on initialization of component?
	// todo changesound functions, playsound, etc
};

class SoundEventSystem : public BaseECSSystem
{
private:
public:
	SoundEventSystem() : BaseECSSystem()
	{
		AddComponentType(SoundComponent::ID);
	}

	virtual void UpdateComponents(float delta, BaseECSComponent** components)
	{
		SoundComponent* sound = (SoundComponent*)components[0];

		switch (sound->soundEvent)
		{
			case SOUND_EVENT::ZERO:
				break;
		//	case SOUND_EVENT::UI_BUTTON_0:
				break;
		//	case SOUND_EVENT::UI_BUTTON_1:
				break;
		}

	}

	virtual void PlaySound(BaseECSComponent** components)
	{

	}
};







class Game
{
private:
	SkinnedMesh kotlin;

	// GLFW
	GLFWwindow* _window;
		const int _WINDOW_WIDTH, _WINDOW_HEIGHT;
		int _framebufferWidth, _framebufferHeight;
		const int _GL_VERSION_MAJOR, _GL_VERSION_MINOR;

	// Delta time
	float _deltaTime;
	float _currentTime;
	float _previousTime;
	const float _TICK_RATE = 1.0f/60.0f;

	// FPS
	double _lastTime;
	int _numFrames;

	// Mouse input
	double _lastMouseX;
	double _lastMouseY;
	double _mouseX;
	double _mouseY;
	double _mouseOffsetX;
	double _mouseOffsetY;
	bool _firstMouse;

	// TODO TEST FUNCTION REMOVE LATER
//	unsigned int FBO, framebufferTexture, RBO, rectVAO, rectVBO;
	unsigned int _shadowMapFBO, _shadowMapID;
	unsigned int _shadowMapWidth = 1024, _shadowMapHeight = 1024;
	glm::mat4 _lightProjection;

	// Matrices
	glm::mat4 _viewMatrix;
		glm::vec3 _camPosition; /// Only used for init
		glm::vec3 _worldUp;
		glm::vec3 _camFront;
	glm::mat4 _projectionMatrix;
		float _fov;
		float _nearPlane;
		float _farPlane;


	// Game elements
	Camera _camera;

	std::vector<Shader*> _shaders;
	std::vector<Texture*> _textures;
	std::vector<Material*> _materials;
	std::vector<Model*> _models;
	std::vector<PointLight*> _pointLights;
	
	std::vector<Framebuffer*> _framebuffers;

	ECS _ecs;
	ECSSystemList _ecsMainSystems;
	ECSSystemList _ecsRenderingPipeline;

	InputControl _ic_x;
	InputControl _ic_z;

	EntityHandle _entity;

	// Methods
	void _InitGLFW();
	void _InitWindow(const char* title, bool resizeable);
	/// DO NOT CALL InitGlew() BEFORE glfwMakeContextCurrent(...)
	void _InitGLEW();
	void _InitGLFlags();
	void _InitMatrices();
	void _InitShaders();
	void _InitFramebuffers();
	void _InitTextures();
	void _InitMaterials();
	void _InitModels();
	void _InitPointLights();
	void _InitLights();
	void _InitUniforms();
	
	void _InitECS();


	void _UpdateUniforms();
//	void _UpdateCameraUniforms();

	void _UpdateDeltaTime();
	void _UpdateInputMouse();
	void _UpdateInputKeyboard();

	void _UpdateInput(GLFWwindow* window);
	void _UpdateInput(GLFWwindow* window, Texture* tex);

	void _InitSoundSystem();

	void _TestFunction();

	// Static variables

public:
	// Constructors/deconstructors
	Game(
		const char* title,
		const int WINDOW_WIDTH,
		const int WINDOW_HEIGHT,
		const int GL_VERSION_MAJOR,
		const int GL_VERSION_MINOR
	);
	virtual ~Game();

	// Accessors
	int GetWindowShouldClose();

	// Modifiers
	void SetWindowShouldClose();

	// Functions
	void Update();
	void Render();

	// Static functions
	/// Called whenever GLFW detects that the framebuffer has been resized
	static void framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight);

	/// Called when the user scrolls, whether with a mouse wheel or touchpad gesture
	static void scrollwheel_callback(GLFWwindow* window, double xoffset, double yoffset);

};