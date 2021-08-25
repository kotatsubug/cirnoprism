#pragma once

#include "libs.hh"
#include "common.hh"

#include "ecs/ecs.hh"
#include "renderer/transform.hh"
#include "input_control.hh"

#define GLEW_EXPERIMENTAL GL_TRUE

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

struct CameraComponent : public ECSComponent<CameraComponent>
{
	Camera camera;
	int x;
};

class MovementControlSystem : public BaseECSSystem
{
private:
public:
	MovementControlSystem() : BaseECSSystem()
	{
		AddComponentType(TransformComponent::ID);
		AddComponentType(MovementControlComponent::ID);
		AddComponentType(CameraComponent::ID);
	}

	virtual void UpdateComponents(float delta, BaseECSComponent** components)
	{
		TransformComponent* transform = (TransformComponent*)components[0];
		MovementControlComponent* movementControl = (MovementControlComponent*)components[1];
		CameraComponent* cameraComponent = (CameraComponent*)components[2];

		for (uint32_t i = 0; i < movementControl->movementControls.size(); i++)
		{
			glm::vec3 movement = movementControl->movementControls[i].first;
			InputControl* input = movementControl->movementControls[i].second;
			glm::vec3 newPos = (transform->transform.GetPosition()) + (movement * input->GetAmt() * delta);
			transform->transform.SetPosition(newPos);
		}

		cameraComponent->camera.SetAbsolutePosition(transform->transform.GetPosition());
	}
};

class Game
{
private:
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
//	ECSSystemList _ecsRenderingPipeline;

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


//	void _UpdateUniforms();
	void _UpdateCameraUniforms();

	void _UpdateDeltaTime();
	void _UpdateInputMouse();
	void _UpdateInputKeyboard();

	void _UpdateInput(GLFWwindow* window);
	void _UpdateInput(GLFWwindow* window, Texture* tex);

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