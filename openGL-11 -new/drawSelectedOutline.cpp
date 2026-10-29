#include "shape.h"
#include "drawSelectedOutline.h"
#include <vector>

void drawSelectedOutline(const Shape& shape, GLuint vbo_pos, GLuint vbo_color) { //선택된 테두리 그리는 함수 
    if (!shape.isshow) return;

	const float size = shape.size;
    GLfloat position[4][3] = {};
	GLfloat color[4][3] = { {1.0,0.0,0.0}, {1.0,0.0,0.0}, {1.0,0.0,0.0} }; // 모두 0 → 검정색
    int count = 4;

    if (shape.type == TRIANGLE)
    {
		count = 3;

		for (int i = 0; i < count; i++) {
			position[i][0] = shape.vertices[i][0];
			position[i][1] = shape.vertices[i][1];
			position[i][2] = shape.vertices[i][2];
		}
		
    }
    else
    {
        float halfWidth = size;
        float halfHeight = size;

        if (shape.type == LINE)
        {
			return;
            halfHeight = 0.02f;
        }
        else if (shape.type == POINT)
        {
            halfWidth = 0.02f;
            halfHeight = 0.02f;
        }

        position[0][0] = shape.x - halfWidth;
        position[0][1] = shape.y - halfHeight;

        position[1][0] = shape.x + halfWidth;
        position[1][1] = shape.y - halfHeight;

        position[2][0] = shape.x + halfWidth;
        position[2][1] = shape.y + halfHeight;

        position[3][0] = shape.x - halfWidth;
        position[3][1] = shape.y + halfHeight;
    }

    // VBO에 데이터 전달 및 그리기
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_DYNAMIC_DRAW);

	glLineWidth(3.0f);
    glDrawArrays(GL_LINE_LOOP, 0, count);
	glLineWidth(1.0f);
}
void mouseClickedTheShape(GLFWwindow* window, int button, int action, const std::vector<Shape>& shapes,
	int& selectedIndex, double& prevMouseX, double& prevMouseY) { // 마우스 클릭시 도형이 클릭됬는지 판단하는 함수
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		if (width <= 0 || height <= 0) return;

		// 마우스 좌표 → 사각형을 그리는 -1~1 좌표
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		selectedIndex = -1;
		// 나중에 그린 사각형부터 검사

		for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i)
		{
			//const Shape& shape = shapes[i];				// 마우스 클릭시 안움직이게 하려면...
			//if (!shape.isshow) continue;

			//const float size = shape.size;
			//float dx = mx - shape.x;
			//float dy = my - shape.y;
			//bool hit = false;

			//if (shape.type == RECTANGLE)
			//{
			//	hit = (dx >= -size && dx <= size &&
			//		dy >= -size && dy <= size);
			//}
			//else if (shape.type == TRIANGLE)
			//{
			//	// 아래쪽은 넓고 위로 갈수록 좁아지는 현재 삼각형
			//	if (dy >= -size && dy <= size)
			//	{
			//		float halfWidth = (size - dy) / 2.0f;
			//		hit = (dx >= -halfWidth && dx <= halfWidth);
			//	}
			//}
			//else if (shape.type == LINE)
			//{
			//	// 가로선 주변에 클릭 여유를 줌
			//	const float tolerance = 0.02f;

			//	hit = (dx >= -size && dx <= size &&
			//		dy >= -tolerance && dy <= tolerance);
			//}
			//else if (shape.type == POINT)
			//{
			//	// 작은 점 주변에 클릭 여유를 줌
			//	const float tolerance = 0.02f;

			//	hit = (dx >= -tolerance && dx <= tolerance &&
			//		dy >= -tolerance && dy <= tolerance);
			//}

			//if (hit)
			//{
			//	selectedIndex = i;
			//	prevMouseX = mx;
			//	prevMouseY = my;
			//	return;
			//}
			// 원본을 바꾸지 않고 정점과 경계를 계산하기 위해 복사
			Shape shape = shapes[i];

			if (!shape.isshow || !shape.draggable || shape.type == NONE)
				continue;

			shape.updateVertices();
			if (shape.vertexCount == 0)
				continue;

			float left = shape.minX();
			float right = shape.maxX();
			float bottom = shape.minY();
			float top = shape.maxY();

			if (mx >= left && mx <= right &&
				my >= bottom && my <= top)
			{
				selectedIndex = i;
				prevMouseX = mx;
				prevMouseY = my;
				return;
			}
		}

	}
}
