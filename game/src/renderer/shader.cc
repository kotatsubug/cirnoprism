#include "shader.hh"

Shader::Shader(
	const int glVersionMajor,
	const int glVersionMinor,
	const std::string& vertexFile,
	const std::string& fragmentFile,
	const std::string& geometryFile)
	: _glVersionMajor(glVersionMajor), _glVersionMinor(glVersionMinor)
{
	GLuint vertexShader = 0;
	GLuint geometryShader = 0;
	GLuint fragmentShader = 0;

	vertexShader = _LoadShader(GL_VERTEX_SHADER, vertexFile);
	if (geometryFile != "") geometryShader = _LoadShader(GL_GEOMETRY_SHADER, geometryFile);
	fragmentShader = _LoadShader(GL_FRAGMENT_SHADER, fragmentFile);

	_LinkProgram(vertexShader, geometryShader, fragmentShader);

	// End
	glDeleteShader(vertexShader);
	glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(_id);
}

std::string Shader::_LoadShaderFile(const std::string& filename)
{
	std::string temp = "";
	std::string src = "";
	std::ifstream in_file;

	in_file.open(filename);

	if (in_file.is_open())
	{
		while (std::getline(in_file, temp))
		{
			src += temp + "\n";
		}
	}
	else
	{
		DEBUG_LOG("Shader", LOG_ERROR, "Could not open file: %s", filename.c_str());
	}

	in_file.close();

	std::string version = std::to_string(_glVersionMajor) + std::to_string(_glVersionMinor) + "0";
	src.replace(src.find("#version"), 12, ("#version " + version));

	return src;
}

GLuint Shader::_LoadShader(GLenum shaderType, const std::string& filename)
{
	char infoLog[512];
	GLint success;

	GLuint shader = glCreateShader(shaderType);
	std::string shaderCode = _LoadShaderFile(filename);
	const GLchar* shaderSrc = shaderCode.c_str();
	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);

	// Compilation error check
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		DEBUG_LOG("Shader", LOG_ERROR, "Could not compile shader [%s], GetShaderInfoLog returns [%s]", filename.c_str(), infoLog);
	}

	return shader;
}

void Shader::_LinkProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader)
{
	char infoLog[512];
	GLint success;

	_id = glCreateProgram();

	glAttachShader(_id, vertexShader);
	if (geometryShader)
		glAttachShader(_id, geometryShader);
	glAttachShader(_id, fragmentShader);

	glLinkProgram(_id);

	glGetProgramiv(_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(_id, 512, NULL, infoLog);
		DEBUG_LOG("Shader", LOG_ERROR, "Could not link program, GetShaderInfoLog returns [%s]", infoLog);
	}

	glUseProgram(0);
}

void Shader::Use()
{
	glUseProgram(_id);
}

void Shader::UnUse()
{
	glUseProgram(0);
}

void Shader::Set1i(const int val, const std::string& name)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniform1i(location, val);
}

void Shader::Set1f(const float val, const std::string& name)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniform1f(location, val);
}

void Shader::SetVec2f(const glm::fvec2& val, const std::string& name)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniform2f(location, val.x, val.y);
}

void Shader::SetVec3f(const glm::fvec3& val, const std::string& name)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniform3f(location, val.x, val.y, val.z);
}

void Shader::SetVec4f(const glm::fvec4& val, const std::string& name)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniform4f(location, val.x, val.y, val.z, val.w);
}

void Shader::SetMat3fv(const glm::mat3& matrix, const std::string& name, GLboolean transpose)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniformMatrix3fv(location, 1, transpose, glm::value_ptr(matrix));
}

void Shader::SetMat4fv(const glm::mat4& matrix, const std::string& name, GLboolean transpose)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniformMatrix4fv(location, 1, transpose, glm::value_ptr(matrix));
	
}

void Shader::SetArrMat4fv(const std::vector<glm::mat4>& matrices, const std::string& name, GLboolean transpose)
{
	GLint location = glGetUniformLocation(_id, name.c_str());
	glUniformMatrix4fv(location, (GLsizei)(matrices.size()), transpose, glm::value_ptr(matrices[0]));
}
