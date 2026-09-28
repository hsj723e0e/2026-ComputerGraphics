#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw3.h>

#include "shader.h"
GLuint shaderProgramID; // 쉐이더 프로그램 만들기
GLuint vertexShader;// 얘는 버텍스 세이더 객체
GLuint fragmentShader; // 얘는 프래그먼트 세이터 객체 


std::string filetobuf(const char* file)
{
	std::ifstream shaderFile(file);

	if (!shaderFile.is_open())
	{
		return "";
	}

	std::string source(
		(std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	shaderFile.close();
	return source;
}
//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	std::string vertexSource = filetobuf("vertex.glsl"); // filetobuf: 사용자 정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	const char* source = vertexSource.c_str();
	//--- 셰이더 생성하고 코드 연결한 후 컴파일 하기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);  // vertex shader 생성
	glShaderSource(vertexShader, 1, &source, NULL);  // vertex shader 소스 지정
	glCompileShader(vertexShader); // vertex shader 컴파일 
	GLint result; // 여기서부터 1234
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	} // 여기까지 1234 에러체크 하는 부분이므로 꼭 넣을 것 
}
//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	std::string fragmentSource = filetobuf("fragment.glsl"); // filetobuf: 사용자 정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	const char* source = fragmentSource.c_str();
	//--- 셰이더 생성하고 코드 연결한 후 컴파일 하기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // fragment shader 생성
	glShaderSource(fragmentShader, 1, &source, NULL); // fragment shader 소스 시정
	glCompileShader(fragmentShader); // fragment shader 컴파일
	GLint result;  // 여기서부터 1234
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}  // 여기까지 1234 에러체크 하는 부분이므로 꼭 넣을 것 
}
//--- 세이더 프로그램 생성하기
GLuint make_shaderProgram()
{
	//-- shader Program
	shaderProgramID = glCreateProgram(); // shader program 생성
	glAttachShader(shaderProgramID, vertexShader); // 두 shader(vertex부분)을 program에 연결
	glAttachShader(shaderProgramID, fragmentShader); // 두 shader(fragment부분)을 program에 연결
	glLinkProgram(shaderProgramID); // 세이터 프로그램 링크
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader); // 링크가 끝났으므로 셰이더 객체 삭제 
	glDeleteShader(fragmentShader); // 링크가 끝났으므로 셰이더 객체 삭제
	//--- 링크 성공 여부 확인
	GLint result; // 여기서부터 1234
	GLchar errorLog[512];
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog
		(shaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return 0;
	} // 여기까지 1234 에러 체크하는 부분이므로 꼭 넣을 것 
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID); // shader program 사용
	return shaderProgramID;
}
