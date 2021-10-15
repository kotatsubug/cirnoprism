#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include <SOIL2.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/vertex.hh"
#include "renderer/primitives.hh"
#include "renderer/mesh.hh"
#include "renderer/shader.hh"
#include "renderer/texture.hh"
#include "renderer/material.hh"
#include "renderer/model.hh"
#include "renderer/light.hh"
#include "renderer/framebuffer.hh"
#include "renderer/camera.hh"


/// Cirnoprism numerical 16-bit flags
#define RENDER_BACKFACE_CULLING 0x0000
#define RENDER_FRONTFACE_CULLING 0x0001
#define RENDER_SINGLE_SIDED_FACES 0x0000
#define RENDER_DOUBLE_SIDED_FACES 0x0002
// #define CATEGORY_TYPE 0x0001 (01)
// #define CATEGORY_TYPE 0x0002 (10)
// #define CATEGORY_TYPE 0x0004 (100)
// #define CATEGORY_TYPE 0x0008 (1000)
// #define CATEGORY_TYPE 0x0010 (10000)
// #define CATEGORY_TYPE 0x0020 (100000)
// #define CATEGORY_TYPE 0x0040 (1000000)
// #define CATEGORY_TYPE 0x0080 (10000000)
// #define CATEGORY_TYPE 0x0100 (100000000)