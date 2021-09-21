#pragma once

#include <iostream>
#include <string>

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <SOIL2.h>

class Texture
{
private:
	GLuint _id;
	int _width;
	int _height;
	unsigned int _type;
public:

	Texture(const char* filename, GLenum type)
	{
		_type = type;
		SetTexture(filename);
	}

	~Texture()
	{
		glDeleteTextures(1, &_id);
	}

	/// Trivial -- set replace=true if dynamic texture replacement needed.
	/// This does NOT replace individual textures associated with meshes, but the entire texture data in memory
	void SetTexture(const char* filename, bool replace = false)
	{ 
		if (replace && _id)
		{
			glDeleteTextures(1, &_id);
		}

		glGenTextures(1, &_id);
		glBindTexture(_type, _id);

		// Below: repeat texture in the ST-plane when surface is larger than texture
		glTexParameteri(_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Below: how to handle mipmaps (dynamic antialiasing depending on how close the camera is)
		//		GL_NEAREST | GL_NEAREST_MIPMAP_LINEAR just takes the nearest texel, will look more pixel-y
		//		GL_LINEAR | GL_LINEAR_MIPMAP_LINEAR takes an average of surrounding texels
		//		GL_**_MIPMAP_** applies the effect to all mipmaps of the texture.
		glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// Below: make the texture N I C E and C R I S P Y
		glTexParameteri(_type, GL_TEXTURE_MIN_LOD, static_cast<GLint>(1.0f));

		// Load image
		unsigned char* image = SOIL_load_image(filename, &_width, &_height, NULL, SOIL_LOAD_RGBA);

		// Check if image was loaded correctly
		if (image)
		{
			// 2nd argument is starting mipmap level (0)
			glTexImage2D(_type, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(_type);
		}
		else
		{
			DEBUG_LOG("Texture", LOG_ERROR, "SetTexture failed for [%s]", filename);
		}

		glActiveTexture(0);
		glBindTexture(_type, 0);
		SOIL_free_image_data(image);
	}

	/// Dynamically bind the texture to some spot(s) called the textureUnit.
	void Bind(const GLint textureUnit)
	{
		// Dynamic texture activation. If texture ID is 0, GL_TEXTURE0 + textureUnit = ID 0 and so on.
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(_type, _id);
	}

	void UnBind()
	{
		glActiveTexture(0);
		glBindTexture(_type, 0);
	}

	inline GLuint GetID() const { return _id; }

};