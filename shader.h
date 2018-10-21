#pragma once

#include <GL/gl.h>
#include <fstream>

class Shader {
public:
    Shader(const char* vertPath, const char* fragPath);

    void use() const;
    GLuint id() const;

private: 
    GLuint _program;

    static GLuint loadShaders(const char* vertPath, const char* fragPath);
};

// -------------
// implementation here
// -------------

Shader::Shader(const char* vertPath, const char* fragPath) {
    _program = loadShaders(vertPath, fragPath);
}

void Shader::use() const {
    glUseProgram(_program); 
}

GLuint Shader::id() const {
    return _program;
}

GLuint Shader::loadShaders(const char* vertPath, const char* fragPath) {
	GLuint vertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vertPath, std::ios::in);
	if (vertexShaderStream.is_open()){
		std::stringstream ss;
		ss << vertexShaderStream.rdbuf();
		vertexShaderCode = ss.str();
		vertexShaderStream.close();
	} else {
        throw std::runtime_error("vshader file path doesn't exist.");
	}

	// Read the Fragment Shader code from the file
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderStream(fragPath, std::ios::in);
	if(fragmentShaderStream.is_open()){
		std::stringstream ss;
		ss << fragmentShaderStream.rdbuf();
		fragmentShaderCode = ss.str();
		fragmentShaderStream.close();
	} else {
        throw std::runtime_error("fshader file path doesn't exist.");
    }

	GLint result = GL_FALSE;
	int infoLogLength;

    std::cout << "compiling shader: " << vertPath << std::endl;
	char const * vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer , nullptr);
	glCompileShader(vertexShaderID);

    auto printVectorChar = [](const auto& v) {
        for (auto c : v)
            std::cout << c;
        std::cout << std::endl;
    };

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> vertexShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(vertexShaderID, infoLogLength, nullptr, &vertexShaderErrorMessage[0]);
        printVectorChar(vertexShaderErrorMessage);
	}

    std::cout << "compiling shader: " << fragPath << std::endl;
	char const * fragmentSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer , nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> fragmentShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(fragmentShaderID, infoLogLength, nullptr, &fragmentShaderErrorMessage[0]);
        printVectorChar(fragmentShaderErrorMessage);
	}

	// Link the program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> programErrorMessage(infoLogLength+1);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        printVectorChar(programErrorMessage);
	}

	
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}
