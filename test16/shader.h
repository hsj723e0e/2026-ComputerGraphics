#pragma once
#include <GL/glew.h>

extern GLuint shaderProgramID; // 다른 소스 파일(외부 파일)에 정의된 전역 변수나 함수를 현재 파일에서 참조(선언)할 때 사용

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();