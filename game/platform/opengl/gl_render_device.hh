#pragma once

//#include "color.hh"
#include <string>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <glew.h>

#include "../../common.hh"
#include "../sdl/sdl_window.hh"

#define USE_GLEW_EXPERIMENTAL GL_TRUE

#define MAKEFOURCC(a, b, c, d) \
	((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | \
		((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))

#define MAKEFOURCCDXT(a) MAKEFOURCC('D', 'X', 'T', a)

#define FOURCC_DXT1 MAKEFOURCCDXT('1')
#define FOURCC_DXT2 MAKEFOURCCDXT('2')
#define FOURCC_DXT3 MAKEFOURCCDXT('3')
#define FOURCC_DXT4 MAKEFOURCCDXT('4')
#define FOURCC_DXT5 MAKEFOURCCDXT('5')

typedef SDL_GLContext DeviceContext;

class OpenGLRenderDevice
{
public:
	enum BufferUsage
	{
		USAGE_STATIC_DRAW = GL_STATIC_DRAW,
		USAGE_STREAM_DRAW = GL_STREAM_DRAW,
		USAGE_DYNAMIC_DRAW = GL_DYNAMIC_DRAW,

		USAGE_STATIC_COPY = GL_STATIC_COPY,
		USAGE_STREAM_COPY = GL_STREAM_COPY,
		USAGE_DYNAMIC_COPY = GL_DYNAMIC_COPY,

		USAGE_STATIC_READ = GL_STATIC_READ,
		USAGE_STREAM_READ = GL_STREAM_READ,
		USAGE_DYNAMIC_READ = GL_DYNAMIC_READ,
	};

	enum SamplerFilter
	{
		FILTER_NEAREST = GL_NEAREST,
		FILTER_LINEAR = GL_LINEAR,
		FILTER_NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
		FILTER_NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
		FILTER_LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
	};

	enum SamplerWrapMode
	{
		WRAP_CLAMP = GL_CLAMP_TO_EDGE,
		WRAP_REPEAT = GL_REPEAT,
		WRAP_CLAMP_MIRROR = GL_MIRROR_CLAMP_TO_EDGE,
		WRAP_REPEAT_MIRROR = GL_MIRRORED_REPEAT,
	};

	enum PixelFormat
	{
		FORMAT_R,
		FORMAT_RG,
		FORMAT_RGB,
		FORMAT_RGBA,
		FORMAT_DEPTH,
		FORMAT_DEPTH_AND_STENCIL,
	};

	enum PrimitiveType
	{
		PRIMITIVE_TRIANGLES = GL_TRIANGLES,
		PRIMITIVE_POINTS = GL_POINTS,
		PRIMITIVE_LINE_STRIP = GL_LINE_STRIP,
		PRIMITIVE_LINE_LOOP = GL_LINE_LOOP,
		PRIMITIVE_LINES = GL_LINES,
		PRIMITIVE_LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,
		PRIMITIVE_LINES_ADJACENCY = GL_LINES_ADJACENCY,
		PRIMITIVE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		PRIMITIVE_TRIANGLE_FAN = GL_TRIANGLE_FAN,
		PRIMITIVE_TRAINGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
		PRIMITIVE_TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
		PRIMITIVE_PATCHES = GL_PATCHES,
	};

	enum FaceCulling
	{
		FACE_CULL_NONE,
		FACE_CULL_BACK = GL_BACK,
		FACE_CULL_FRONT = GL_FRONT,
		FACE_CULL_FRONT_AND_BACK = GL_FRONT_AND_BACK,
	};

	enum DrawFunc
	{
		DRAW_FUNC_NEVER = GL_NEVER,
		DRAW_FUNC_ALWAYS = GL_ALWAYS,
		DRAW_FUNC_LESS = GL_LESS,
		DRAW_FUNC_GREATER = GL_GREATER,
		DRAW_FUNC_LEQUAL = GL_LEQUAL,
		DRAW_FUNC_GEQUAL = GL_GEQUAL,
		DRAW_FUNC_EQUAL = GL_EQUAL,
		DRAW_FUNC_NOT_EQUAL = GL_NOTEQUAL,
	};

	enum FramebufferAttachment
	{
		ATTACHMENT_COLOR = GL_COLOR_ATTACHMENT0,
		ATTACHMENT_DEPTH = GL_DEPTH_ATTACHMENT,
		ATTACHMENT_STENCIL = GL_STENCIL_ATTACHMENT,
	};

	enum BlendFunc
	{
		BLEND_FUNC_NONE,
		BLEND_FUNC_ONE = GL_ONE,
		BLEND_FUNC_SRC_ALPHA = GL_SRC_ALPHA,
		BLEND_FUNC_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
		BLEND_FUNC_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
		BLEND_FUNC_DST_ALPHA = GL_DST_ALPHA,
	};

	enum StencilOp
	{
		STENCIL_KEEP = GL_KEEP,
		STENCIL_ZERO = GL_ZERO,
		STENCIL_REPLACE = GL_REPLACE,
		STENCIL_INCR = GL_INCR,
		STENCIL_INCR_WRAP = GL_INCR_WRAP,
		STENCIL_DECR_WRAP = GL_DECR_WRAP,
		STENCIL_DECR = GL_DECR,
		STENCIL_INVERT = GL_INVERT,
	};

	struct DrawParams
	{
		enum PrimitiveType primitiveType = PRIMITIVE_TRIANGLES;
		enum FaceCulling faceCulling = FACE_CULL_NONE;
		enum DrawFunc depthFunc = DRAW_FUNC_ALWAYS;
		bool shouldWriteDepth = true;
		bool useStencilTest = false;
		enum DrawFunc stencilFunc = DRAW_FUNC_ALWAYS;
		uint32_t stencilTestMask = 0;
		uint32_t stencilWriteMask = 0;
		int32_t stencilComparisonVal = 0;
		enum StencilOp stencilFail = STENCIL_KEEP;
		enum StencilOp stencilPassButDepthFail = STENCIL_KEEP;
		enum StencilOp stencilPass = STENCIL_KEEP;
		bool useScissorTest = false;
		uint32_t scissorStartX = 0;
		uint32_t scissorStartY = 0;
		uint32_t scissorWidth = 0;
		uint32_t scissorHeight = 0;
		enum BlendFunc sourceBlend = BLEND_FUNC_NONE;
		enum BlendFunc destBlend = BLEND_FUNC_NONE;
	};
private:
	struct VertexArray
	{
		uint32_t* buffers;
		uintptr_t* bufferSizes;
		uint32_t numBuffers;
		uint32_t numElements;
		uint32_t instanceComponentsStartIndex;
		enum BufferUsage usage;
	};

	struct ShaderProgram
	{
		std::vector<uint32_t> shaders;
		std::map<std::string, int32_t> uniformMap;
		std::map<std::string, int32_t> samplerMap;
	};

	struct FBOData
	{
		int32_t width;
		int32_t height;
	};

	static bool _isInitialized;

	DeviceContext _context;
	std::string _shaderVersion;
	uint32_t _version;
	std::map<uint32_t, VertexArray> _vaoMap;
	std::map<uint32_t, FBOData> _fboMap;
	std::map<uint32_t, ShaderProgram> _shaderProgramMap;

	uint32_t _boundFBO;
	uint32_t _viewportFBO;
	uint32_t _boundVAO;
	uint32_t _boundShader;

	enum FaceCulling _currentFaceCulling;
	enum DrawFunc _currentDepthFunc;
	enum BlendFunc _currentSourceBlend;
	enum BlendFunc _currentDestBlend;
	enum DrawFunc _currentStencilFunc;
	uint32_t _currentStencilTestMask;
	uint32_t _currentStencilWriteMask;
	int32_t _currentStencilComparisonVal;
	enum StencilOp _currentStencilFail;
	enum StencilOp _currentStencilPassButDepthFail;
	enum StencilOp _currentStencilPass;

	bool _blendingEnabled;
	bool _shouldWriteDepth;
	bool _stencilTestEnabled;
	bool _scissorTestEnabled;

	void _SetFBO(uint32_t fbo);
	void _SetViewport(uint32_t fbo);
	void _SetVAO(uint32_t vao);
	void _SetShader(uint32_t shader);
	void _SetFaceCulling(enum FaceCulling faceCulling);
	void _SetDepthTest(bool shouldWrite, enum DrawFunc depthFunc);
	void _SetBlending(enum BlendFunc sourceBlend, enum BlendFunc destBlend);

	void _SetStencilTest(
		bool enable, enum DrawFunc stencilFunc, uint32_t stencilTestMask,
		uint32_t stencilWriteMask, int32_t stencilComparisonVal, enum StencilOp stencilFail,
		enum StencilOp stencilPassButDepthFail, enum StencilOp stencilPass);

	void _SetStencilWriteMask(uint32_t mask);

	void _SetScissorTest(bool enable, uint32_t startX = 0, uint32_t startY = 0,
		uint32_t width = 0, uint32_t height = 0);

	uint32_t _GetVersion();
	std::string _GetShaderVersion();
	//	NULL_COPY_AND_ASSIGN(OpenGLRenderDevice)
public:
	static bool GlobalInit();

	OpenGLRenderDevice(SDLWindow& window);
	virtual ~OpenGLRenderDevice();

	uint32_t CreateRenderTarget(
		uint32_t texture, int32_t width, int32_t height,
		enum FramebufferAttachment attachment, uint32_t attachmentNumber, uint32_t mipLevel);

	uint32_t ReleaseRenderTarget(uint32_t fbo);

	uint32_t CreateVertexArray(const float** vertexData, const uint32_t* vertexElementSizes,
		uint32_t numVertexComponents, uint32_t numInstanceComponents,
		uint32_t numVertices, const uint32_t* indices,
		uint32_t numIndices, enum BufferUsage usage);

	void UpdateVertexArrayBuffer(
		uint32_t vao, uint32_t bufferIndex,
		const void* data, uintptr_t dataSize);

	uint32_t ReleaseVertexArray(uint32_t vao);

	uint32_t CreateSampler(
		enum SamplerFilter minFilter, enum SamplerFilter magFilter,
		enum SamplerWrapMode wrapU, enum SamplerWrapMode wrapV, float anisotropy);

	uint32_t ReleaseSampler(uint32_t sampler);

	uint32_t CreateTexture2D(
		int32_t width, int32_t height, const void* data,
		enum PixelFormat dataFormat, enum PixelFormat internalFormat,
		bool generateMipmaps, bool compress);

	uint32_t CreateDDSTexture2D(
		uint32_t width, uint32_t height, const unsigned char* buffer,
		uint32_t fourCC, uint32_t mipMapCount);

	uint32_t ReleaseTexture2D(uint32_t texture2D);

	uint32_t CreateUniformBuffer(const void* data, uintptr_t dataSize, enum BufferUsage usage);
	void UpdateUniformBuffer(uint32_t buffer, const void* data, uintptr_t dataSize);
	uint32_t ReleaseUniformBuffer(uint32_t buffer);

	uint32_t CreateShaderProgram(const std::string& shaderText);
	void SetShaderUniformBuffer(uint32_t shader, const std::string& uniformBufferName,
		uint32_t buffer);
	void SetShaderSampler(uint32_t shader, const std::string& samplerName,
		uint32_t texture, uint32_t sampler, uint32_t unit);
	uint32_t ReleaseShaderProgram(uint32_t shader);

	void Clear(
		uint32_t fbo,
		bool shouldClearColor, bool shouldClearDepth, bool shouldClearStencil,
		const Color& color, uint32_t stencil);

	void Draw(uint32_t fbo, uint32_t shader, uint32_t vao, const DrawParams& drawParams,
		uint32_t numInstances, uint32_t numElements);

};