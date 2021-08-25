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

#include "objloader.hh"
