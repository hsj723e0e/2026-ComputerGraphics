#pragma once
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "shape.h"
#include <vector>
void drawSelectedOutline(const Shape& shape, GLuint vbo_pos, GLuint vbo_color); // 테두리는 그리는 함수 
void mouseClickedTheShape(GLFWwindow* window, int button, int action,  const std::vector<Shape>& shapes,
    int& selectedIndex, double& prevMouseX, double& prevMouseY);// 마우스가 도형을 클릭했는지 판단하는 함수