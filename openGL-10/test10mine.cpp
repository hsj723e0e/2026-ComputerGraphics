#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <math.h>
//--- 필요한 헤더파일 선언
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "shader.h"
#include "shape.h"
#include "drawSelectedOutline.h"
using namespace std;
//--- 기존 사용자 정의 함수
void InitBuffer();
GLvoid drawScene(); // 출력함수 
GLvoid Reshape(int w, int h);
//--- 기존 변수 선언
GLint width, height;
GLuint vao, vbo_pos, vbo_color; // 버퍼 생성 
//--- 개인 변수 선언
std::random_device dre;
std::mt19937 gen(dre());
std::uniform_real_distribution<float> pos(-0.8f, 0.8f);
std::uniform_real_distribution<float> color(0.0f, 1.0f);
std::uniform_real_distribution<float> randomSize(0.05f, 0.2f);
int selectedIndex = -1;
double prevMouseX = 0.0;
double prevMouseY = 0.0;


//--- 개인 함수 선언
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double x, double y);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
float Timer();
void Move(float dt);
vector<Shape> shapes;
vector<Shape> initialShapes;
//--- 메인 함수
int main(int argc, char** argv)
{
	width = 800;
	height = 800;
	//--- GLFW 초기화
	if (!glfwInit())
		return -1;
	//--- OpenGL 버전 및 프로파일 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//--- 윈도우 생성
	GLFWwindow* window =
		glfwCreateWindow(width, height, "Example1", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	//--- 생성한 윈도우의 OpenGL Context를 현재 Context로 설정
	glfwMakeContextCurrent(window);
	//--- GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	//--- VBO, VAO 생성하고 설정하기
	InitBuffer();

	glfwSetKeyCallback(window, keyCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);



	// 위쪽 삼각형: 아래를 향하도록 180도 회전
	shapes.push_back(Shape(LINE_TRIANGLE, 0.7f, 0.2f, 0.0f, 0.0f, 0.7f));
	Shape& tri_top = shapes.back();
	//tri_top.size = 0.05f;
	tri_top.rotate90();
	tri_top.rotate90();

	// 아래쪽 삼각형: 기본 방향이 위쪽이므로 회전 없음
	shapes.push_back(Shape(LINE_TRIANGLE, 0.7f, 0.0f, 0.0f, 0.0f, 0.7f));
	Shape& tri_bottom = shapes.back();
	//tri_bottom.size = 0.05f;

	// 왼쪽 삼각형: 오른쪽을 향하도록 270도 회전
	shapes.push_back(Shape(LINE_TRIANGLE, 0.6f, 0.1f, 0.0f, 0.0f, 0.7f));
	Shape& tri_left = shapes.back();
	//tri_left.size = 0.05f;
	tri_left.rotate90();
	tri_left.rotate90();
	tri_left.rotate90();

	// 오른쪽 삼각형: 왼쪽을 향하도록 90도 회전
	shapes.push_back(Shape(LINE_TRIANGLE, 0.8f, 0.1f, 0.0f, 0.0f, 0.7f));
	Shape& tri_right = shapes.back();
	//tri_right.size = 0.05f;
	tri_right.rotate90();

	// 직각 삼각형 위에 
	shapes.push_back(Shape(LINE_RIGHT_TRIANGLE, 0.725f, -0.605f, 0.0f, 0.0f, 0.7f));
	Shape& right_triangle_top = shapes.back();

	// 직각 삼각형 아래에 
	shapes.push_back(Shape(LINE_RIGHT_TRIANGLE, 0.725f, -0.605f, 0.0f, 0.0f, 0.7f));
	Shape& right_triangle_bottom = shapes.back();
	right_triangle_bottom.x += 0.05f;  // 오른쪽으로 이동
	right_triangle_bottom.y += 0.1f;  // 위쪽으로 이동
	right_triangle_bottom.rotate90();
	right_triangle_bottom.rotate90();


	// 왼쪽 위
	shapes.push_back(Shape(RECTANGLE_LIKEWINDOW, 0.555f, 0.69f, 0.0f, 0.0f, 0.7f));
	Shape& rect1 = shapes.back();
	rect1.outline = true;
	rect1.size = 0.025f;

	// 오른쪽 위
	shapes.push_back(Shape(RECTANGLE_LIKEWINDOW, 0.62f, 0.69f, 0.0f, 0.0f, 0.7f));
	Shape& rect2 = shapes.back();
	rect2.outline = true;
	rect2.size = 0.025f;

	// 왼쪽 아래
	shapes.push_back(Shape(RECTANGLE_LIKEWINDOW, 0.555f, 0.575f, 0.0f, 0.0f, 0.7f));
	Shape& rect3 = shapes.back();
	rect3.outline = true;
	rect3.size = 0.025f;

	// 오른쪽 아래
	shapes.push_back(Shape(RECTANGLE_LIKEWINDOW, 0.62f, 0.575f, 0.0f, 0.0f, 0.7f));
	Shape& rect4 = shapes.back();
	rect4.outline = true;
	rect4.size = 0.025f;

	//---------------------------------------------------------------------------------------- 이 밑에는 색칠된 도형입니다
	for (int i = 0; i < 4; i++) { // 작은 삼각형 
		std::uniform_real_distribution<float> pos(-0.9f, 0.3f);

		float r = color(dre);
		float g = color(dre);
		float b = color(dre);

		float x = pos(dre);
		float y = pos(dre);

		shapes.push_back(Shape(TRIANGLE, x, y, r, g, b));
		shapes.back().size = 0.1f;
		shapes.back().draggable = true;
		for (int j = 0; j < i; j++) {
			shapes.back().rotate90();
		}
	}

	for (int i = 0; i < 2; i++) { // 직각삼각형 
		std::uniform_real_distribution<float> pos(-0.9f, 0.2f);

		float r = color(dre);
		float g = color(dre);
		float b = color(dre);

		float x = pos(dre);
		float y = pos(dre);

		shapes.push_back(Shape(RIGHT_TRIANGLE, x, y, r, g, b));
		shapes.back().size = 0.1f;
		shapes.back().draggable = true;

		for (int j = 0; j < i; j++) {
			shapes.back().rotate90();
			shapes.back().rotate90();

		}
	}

	for (int i = 0; i < 4; i++) { // 사각형 
		std::uniform_real_distribution<float> pos(-0.9f, 0.2f);

		float r = color(dre);
		float g = color(dre);
		float b = color(dre);

		float x = pos(dre);
		float y = pos(dre);

		shapes.push_back(Shape(RECTANGLE_LIKEWINDOW, x, y, r, g, b));
		shapes.back().size = 0.025f;
		shapes.back().draggable = true;

	}
	//----------------------------------------------------------------------------------------
	//--- 렌더링 루프
	initialShapes = shapes;
	while (!glfwWindowShouldClose(window))
	{
		float dt = Timer();
		Move(dt);
		drawScene();

		//--- 윈도우에 렌더링 결과 출력
		glfwSwapBuffers(window);
		//--- 이벤트 처리
		glfwPollEvents();
	}
	//--- 종료
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

GLvoid drawScene()
{

	//--- 변경된 배경색 설정
	//glClearColor(rColor, gColor, bColor, 1.0f);
	glClearColor(1.0, 1.0, 1.0, 1.0f); // 배경을 흰색으로 설정
	glClear(GL_COLOR_BUFFER_BIT); // 이 두줄은 openGL 함수 그대로 만들면 됨 
	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);
	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	GLfloat Border_position[2][3] = { // 경계 나누기 
		// 첫 번째 삼각형
		{0.3f, 1.0f, 0.0f}, // 왼쪽
		{0.3f, -1.0f, 0.0f}, // 오른쪽 

	};

	GLfloat Border_color[2][3];
	for (int i = 0; i < 2; i++) {
		Border_color[i][0] = 0.0f;
		Border_color[i][1] = 0.0f;
		Border_color[i][2] = 0.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Border_position), Border_position, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Border_color), Border_color, GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2);


	//--- 도형을 그리자 
	for (Shape& shape : shapes)//도형들을 하나씩 그리는 역할
	{
		if (shape.type == NONE) continue;

		if (shape.type == POINT) {
			const float size = shape.size;

			GLfloat position[1][3] = {
			   {shape.x , shape.y, 0.0f},
			};

			GLfloat color[1][3] = {
				{shape.r, shape.g, shape.b},
			};

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, sizeof(position),
				position, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(color),
				color, GL_DYNAMIC_DRAW);

			glPointSize(5.0f);

			glDrawArrays(GL_POINTS, 0, 1);
		}
		else if (shape.type == TRIANGLE || shape.type == RIGHT_TRIANGLE) {
			const float size = shape.size;

			shape.updateVertices();
			shape.updateColors();

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, shape.vertexCount * sizeof(shape.vertices[0]),
				shape.vertices, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, shape.vertexCount * sizeof(shape.colors[0]),
				shape.colors, GL_DYNAMIC_DRAW);


			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else if (shape.type == RECTANGLE || shape.type == RECTANGLE_LIKEWINDOW) {
			const float size = shape.size;

			shape.updateVertices();
			shape.updateColors();

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, shape.vertexCount * sizeof(shape.vertices[0]), shape.vertices, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, shape.vertexCount * sizeof(shape.colors[0]), shape.colors, GL_STATIC_DRAW);

			glPolygonMode(GL_FRONT_AND_BACK,
				shape.outline ? GL_LINE : GL_FILL); // outline이 참이면 line을 그리고 false이면 색칠된 도형을 그린다 

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 원레대로 되돌린다 
		}
		else if (shape.type == LINE || shape.type == LINE_RIGHT_TRIANGLE || shape.type == LINE_TRIANGLE) {
			const float size = shape.size;

			shape.updateVertices();
			shape.updateColors();

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(
				GL_ARRAY_BUFFER,
				shape.vertexCount * sizeof(shape.vertices[0]),
				shape.vertices,
				GL_DYNAMIC_DRAW
			);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(
				GL_ARRAY_BUFFER,
				shape.vertexCount * sizeof(shape.colors[0]),
				shape.colors,
				GL_DYNAMIC_DRAW
			);

			glDrawArrays(GL_LINES, 0, shape.vertexCount);
		}
	}
	// 선택된 도형 표시되면 테두리를 그리자
	if (selectedIndex >= 0 &&
		selectedIndex < static_cast<int>(shapes.size()))
	{
		//drawSelectedOutline(shapes[selectedIndex], vbo_pos, vbo_color);
	}
}
//--- 버퍼 생성하고 데이터 받아오기
void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 생성하고 ID 얻기 -> vao를 만들어주는 함수 
	glBindVertexArray(vao); //--- VAO를 현재 사용할 대상으로 지정


	glGenBuffers(1, &vbo_pos); // 버퍼 id 생성 
	glGenBuffers(1, &vbo_color); // 버퍼 id 생성 

	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 버퍼객체에 저장할 데이터 타입 지정, bind후에는 모든 버퍼는 바인딩 된 버퍼를 사용한다 
	//--- triShape 배열의 사이즈: sizeof(triShape) = 9 * float
	//glBufferData(GL_ARRAY_BUFFER, sizeof(triShape), triShape, GL_STATIC_DRAW); // bind된 버퍼에 데이터를 넣는 것이다 
		//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // bind된 버퍼에서 속성에 번호를 어떻게 설정햇는가
	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0); // 속성의 번호를 활성화 


	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	//--- colors 배열의 사이즈: sizeof(colors) = 9 *float
	//glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);
}
void DeleteShape() {
	for (Shape& shape : shapes) {
		shape.type = NONE;
	}
	/*shapes.clear();
	shapes.resize(4);*/
	selectedIndex = -1;
}
static double lastTime = glfwGetTime();

float Timer() {

	double now = glfwGetTime();
	float dt = static_cast<float>(now - lastTime);
	lastTime = now;

	return dt;
}
void Move(float dt) {
	/*if (조건) {

		for (Shape& shape : shapes) {
			if (shape.minX() <= -1.0f || shape.maxX() >= 1.0f) {
				shape.speedX *= -1.0f;
				shape.rotate90();
			}

			if (shape.minY() <= -1.0f || shape.maxY() >= 1.0f) {
				shape.speedY *= -1.0f;
				shape.rotate90();
			}

			float dx = shape.speedX * dt;
			float dy = shape.speedY * dt;

			shape.move(dx, dy);
		}
	}*/



}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		shapes = initialShapes;
		selectedIndex = -1;
		prevMouseX = 0.0;
		prevMouseY = 0.0;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

// 조각의 꼭짓점들이 목표 꼭짓점들과 일치하는지 검사
bool fitsTarget(Shape piece, Shape target)
{
	piece.updateVertices();
	target.updateVertices();

	if (piece.vertexCount == 0 || target.vertexCount == 0)
		return false;

	const float tolerance = 0.015f;

	for (int i = 0; i < piece.vertexCount; i++) {
		bool found = false;

		for (int j = 0; j < target.vertexCount; j++) {
			float dx = piece.vertices[i][0] - target.vertices[j][0];
			float dy = piece.vertices[i][1] - target.vertices[j][1];

			if (dx * dx + dy * dy <= tolerance * tolerance) {
				found = true; // 조각이 맞았다 
				break;
			}
		}

		if (!found)
			return false;
	}

	return true;
}

// 한 무늬를 구성하는 조각이 전부 맞았을 때만 잠금
void checkPuzzleGroup(int targetStart, int pieceStart, int count)
{
	if (targetStart + count > static_cast<int>(shapes.size()) ||
		pieceStart + count > static_cast<int>(shapes.size()))
		return;

	bool used[4] = {};
	int matchedPieces[4] = {};

	for (int t = 0; t < count; t++) {
		int matched = -1;

		for (int p = 0; p < count; p++) {
			if (used[p])
				continue;

			if (fitsTarget(shapes[pieceStart + p],
				shapes[targetStart + t])) {
				matched = p;
				break;
			}
		}

		// 목표 하나라도 비어 있으면 아직 완성 아님
		if (matched == -1)
			return;

		used[matched] = true;
		matchedPieces[t] = pieceStart + matched;
	}

	// 모든 목표가 서로 다른 조각으로 채워졌을 때 실행
	for (int t = 0; t < count; t++) {
		Shape& piece = shapes[matchedPieces[t]];
		const Shape& target = shapes[targetStart + t];

		piece.x = target.x;
		piece.y = target.y;
		piece.draggable = false;
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		if (width <= 0 || height <= 0) return;

		// 마우스 좌표 → 사각형을 그리는 -1~1 좌표
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		if (mx >= 0.0f && my >= 0.0f) {
			// 1사분면
			shapes[0].size += 0.05;
			if (shapes[0].size > 0.2) shapes[0].size = 0.03;
		}

	}
	mouseClickedTheShape(window, button, action, shapes, selectedIndex, prevMouseX, prevMouseY);
	//-------------------------------------------------------------------
	if (button == GLFW_MOUSE_BUTTON_LEFT &&
		action == GLFW_RELEASE)
	{
		checkPuzzleGroup(0, 10, 4); // 가운데 삼각형 무늬
		checkPuzzleGroup(4, 14, 2); // 아래 직각삼각형 무늬
		checkPuzzleGroup(6, 16, 4); // 위쪽 창문 무늬

		selectedIndex = -1;
	}
	//-------------------------------------------------------------------
}
void CursorPosCallback(GLFWwindow* window, double x, double y)
{
	// 왼쪽 버튼을 누르고 있을 때만 이동
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)
		!= GLFW_PRESS)
		return;

	// 선택된 도형이 없으면 종료
	if (selectedIndex < 0 ||
		selectedIndex >= static_cast<int>(shapes.size()))
		return;

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (width <= 0 || height <= 0) return;

	// 마우스 좌표를 OpenGL 좌표로 변환
	double mx = 2.0 * x / width - 1.0;
	double my = 1.0 - 2.0 * y / height;

	// 마우스가 움직인 거리만큼 도형 이동
	shapes[selectedIndex].x += static_cast<float>(mx - prevMouseX);
	shapes[selectedIndex].y += static_cast<float>(my - prevMouseY);

	prevMouseX = mx;
	prevMouseY = my;
}

