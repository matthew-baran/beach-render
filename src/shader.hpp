#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	unsigned int id;

	Shader(std::string vertex_file, std::string fragment_file)
	{
		auto vs = getShaderSource(vertex_file);
		auto fs = getShaderSource(fragment_file);
		auto vert_source = vs.c_str();
		auto frag_source = fs.c_str();

		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vert_source, NULL);
		glCompileShader(vertexShader);

		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &frag_source, NULL);
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		id = glCreateProgram();
		glAttachShader(id, vertexShader);
		glAttachShader(id, fragmentShader);
		glLinkProgram(id);

		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	std::string getShaderSource(std::string filename) const
	{
		std::ifstream f(filename);
		std::ostringstream buffer;
		buffer << f.rdbuf();
		auto str(buffer.str());

		return buffer.str();
	}

	Shader() = delete;
	~Shader() = default;
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = default;
	Shader& operator=(Shader&&) = default;

	// Use/activate the shader
	void use() const
	{
		glUseProgram(id);
	}

	// set uniform types
	void setBool(const std::string& name, bool value) const
	{
		use();
		glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
	}

	void setInt(const std::string& name, int value) const
	{
		use();
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const
	{
		use();
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void setMat3(const std::string& name, glm::mat3 value) const
	{
		use();
		auto value_loc = glGetUniformLocation(id, name.c_str());
		glUniformMatrix3fv(value_loc, 1, GL_FALSE, glm::value_ptr(value));
	}

	void setMat4(const std::string& name, glm::mat4 value) const
	{
		use();
		auto value_loc = glGetUniformLocation(id, name.c_str());
		glUniformMatrix4fv(value_loc, 1, GL_FALSE, glm::value_ptr(value));
	}

	void setVec2(const std::string& name, glm::vec2 value)
	{
		use();
		auto value_loc = glGetUniformLocation(id, name.c_str());
		glUniform2f(value_loc, value.x, value.y);
	}

	void setVec3(const std::string& name, float v0, float v1, float v2) const
	{
		use();
		auto value_loc = glGetUniformLocation(id, name.c_str());
		glUniform3f(value_loc, v0, v1, v2);
	}

	void setVec3(const std::string& name, glm::vec3 value) const
	{
		use();
		auto value_loc = glGetUniformLocation(id, name.c_str());
		glUniform3f(value_loc, value.x, value.y, value.z);
	}
};