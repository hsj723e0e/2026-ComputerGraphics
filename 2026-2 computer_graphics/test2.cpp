#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
using namespace std;
GLFWwindow* window= nullptr;
std::mt19937 dre(std::random_device{}());
GLfloat red, green, blue, rect_x1, rect_y1, rect_x2, rect_y2;
bool showRect = false;
int selectedArea = -1;
int selectedIndex = -1;
struct Rectangle {
	float red, green, blue;
	float x1, y1, x2, y2;
	bool show = false;
};
Rectangle rects[4][5]; //1,2,3,4분면에 사각형 5개 
int rectCount[4] = { 0 }; 
float bg[4][3] = {	// bg[n][0]은 r bg[n][1]은 g bg[n][2]은 b이다 
	{1, 0, 0}, // 좌하 bg[0]
	{0, 1, 0}, // 우하 bg[1]
	{0, 0, 1}, // 좌상 bg[2]
	{1, 1, 0}  // 우상 bg[3]
};

void glPrintRect(GLfloat red, GLfloat green, GLfloat blue,
	GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glColor3f(red, green, blue);
	glRectf(x1, y1, x2, y2);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		// ESC 키를 눌렀을 때 작업
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	//else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
	//	uniform_real_distribution<float> uid(0.0f, 1.0f);
	//	red = uid(dre);	green = uid(dre); blue = uid(dre);
	//	std::uniform_real_distribution<float> uid_1(-1.0f, 0.0f);
	//	std::uniform_real_distribution<float> uid_2(0.0f, 1.0f);
	//	while (1) {
	//		rect_x1 = uid_1(dre);  rect_x2 = uid_1(dre);
	//		if ((abs(rect_x2 - rect_x1)) < 0.5 && abs(rect_x2 - rect_x1) > 0.1) break;
	//	}
	//	while (1) {
	//		rect_y1 = uid_2(dre); rect_y2 = uid_2(dre);
	//		if ((abs(rect_y2 - rect_y1)) < 0.5 && abs(rect_y2 - rect_y1) > 0.1) break;
	//	}
	//	showRect = true;
	//}
	//else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
	//	// 좌하: x 음수, y 음수
	//	uniform_real_distribution<float> uid(0.0f, 1.0f);
	//	red = uid(dre); green = uid(dre); blue = uid(dre);

	//	uniform_real_distribution<float> pos(-1.0f, 0.0f);

	//	while (1) {
	//		rect_x1 = pos(dre); rect_x2 = pos(dre);
	//		if (abs(rect_x2 - rect_x1) < 0.5f &&
	//			abs(rect_x2 - rect_x1) > 0.1f) break;
	//	}
	//	while (1) {
	//		rect_y1 = pos(dre); rect_y2 = pos(dre);
	//		if (abs(rect_y2 - rect_y1) < 0.5f &&
	//			abs(rect_y2 - rect_y1) > 0.1f) break;
	//	}
	//	showRect = true;
	//}
	//else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
	//	// 우상: x 양수, y 양수
	//	uniform_real_distribution<float> uid(0.0f, 1.0f);
	//	red = uid(dre); green = uid(dre); blue = uid(dre);

	//	uniform_real_distribution<float> pos(0.0f, 1.0f);

	//	while (1) {
	//		rect_x1 = pos(dre); rect_x2 = pos(dre);
	//		if (abs(rect_x2 - rect_x1) < 0.5f &&
	//			abs(rect_x2 - rect_x1) > 0.1f) break;
	//	}
	//	while (1) {
	//		rect_y1 = pos(dre); rect_y2 = pos(dre);
	//		if (abs(rect_y2 - rect_y1) < 0.5f &&
	//			abs(rect_y2 - rect_y1) > 0.1f) break;
	//	}
	//	showRect = true;
	//}
	//else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
	//	// 우하: x 양수, y 음수
	//	uniform_real_distribution<float> uid(0.0f, 1.0f);
	//	red = uid(dre); green = uid(dre); blue = uid(dre);

	//	uniform_real_distribution<float> posX(0.0f, 1.0f);
	//	uniform_real_distribution<float> posY(-1.0f, 0.0f);

	//	while (1) {
	//		rect_x1 = posX(dre); rect_x2 = posX(dre);
	//		if (abs(rect_x2 - rect_x1) < 0.5f &&
	//			abs(rect_x2 - rect_x1) > 0.1f) break;
	//	}
	//	while (1) {
	//		rect_y1 = posY(dre); rect_y2 = posY(dre);
	//		if (abs(rect_y2 - rect_y1) < 0.5f &&
	//			abs(rect_y2 - rect_y1) > 0.1f) break;
	//	}
	//	showRect = true;
	//}

	else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4
		&& action == GLFW_PRESS) {
		int area = key - GLFW_KEY_1; // 1~4번 → 인덱스 0~3 area=0이면 1번키, area=3이면 4번키

		if (rectCount[area] >= 5) return;

		Rectangle& rect = rects[area][rectCount[area]];

		uniform_real_distribution<float> color(0.0f, 1.0f);

		// 0: 좌상, 1: 좌하, 2: 우상, 3: 우하
		float minX = (area < 2) ? -1.0f : 0.0f;
		float minY = (area % 2 == 0) ? 0.0f : -1.0f;

		uniform_real_distribution<float> posX(minX, minX + 1.0f);
		uniform_real_distribution<float> posY(minY, minY + 1.0f);

		rect.red = color(dre);
		rect.green = color(dre);
		rect.blue = color(dre);

		do {
			rect.x1 = posX(dre);
			rect.x2 = posX(dre);
		} while (abs(rect.x2 - rect.x1) <= 0.1f ||
			abs(rect.x2 - rect.x1) >= 0.5f);

		do {
			rect.y1 = posY(dre);
			rect.y2 = posY(dre);
		} while (abs(rect.y2 - rect.y1) <= 0.1f ||
			abs(rect.y2 - rect.y1) >= 0.5f);

		rectCount[area]++;
	}
	
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		uniform_real_distribution<float> color(0.0f, 1.0f);
		selectedArea = -1;
		selectedIndex = -1;
		for (int i = 0; i < 4; i++) {
			rectCount[i] = 0;
			bg[i][0] = color(dre);
			bg[i][1] = color(dre);
			bg[i][2] = color(dre);
		}
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		uniform_real_distribution<float> color(0.0f, 1.0f);

		Rectangle& rect = rects[selectedArea][selectedIndex];
		rect.red = color(dre);
		rect.green = color(dre);
		rect.blue = color(dre);
	}
	else if (((key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)) ||
		key == GLFW_KEY_KP_ADD) &&
		action == GLFW_PRESS) {
		Rectangle& rect = rects[selectedArea][selectedIndex];
		float cx = (rect.x1 + rect.x2) / 2.0f;
		float cy = (rect.y1 + rect.y2) / 2.0f;

		float scale = 1.1f;


		// 선택한 사분면의 경계
		// 0: 좌상, 1: 좌하, 2: 우상, 3: 우하
		float minX = (selectedArea < 2) ? -1.0f : 0.0f;
		float maxX = minX + 1.0f;

		float minY = (selectedArea % 2 == 0) ? 0.0f : -1.0f;
		float maxY = minY + 1.0f;

		// 확대하되 각 좌표가 사분면 밖으로 나가지 않도록 제한
		rect.x1 = std::clamp(cx + (rect.x1 - cx) * scale, minX, maxX);
		rect.x2 = std::clamp(cx + (rect.x2 - cx) * scale, minX, maxX);
		rect.y1 = std::clamp(cy + (rect.y1 - cy) * scale, minY, maxY);
		rect.y2 = std::clamp(cy + (rect.y2 - cy) * scale, minY, maxY);
	}
	else if (((key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)) ||
		key == GLFW_KEY_KP_SUBTRACT) &&
		action == GLFW_PRESS) {
			Rectangle& rect = rects[selectedArea][selectedIndex];
			float cx = (rect.x1 + rect.x2) / 2.0f;
			float cy = (rect.y1 + rect.y2) / 2.0f;

			float scale = 0.9f;
			rect.x1 = cx + (rect.x1 - cx) * scale;
			rect.x2 = cx + (rect.x2 - cx) * scale;
			rect.y1 = cy + (rect.y1 - cy) * scale;
			rect.y2 = cy + (rect.y2 - cy) * scale;
	}
}
void slice4background() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	int halfW = width / 2;
	int halfH = height / 2;
	glEnable(GL_SCISSOR_TEST);
	uniform_real_distribution<float> color(-1.0f, 1.0f);

	glScissor(0, 0, halfW, halfH);
	glClearColor(bg[0][0], bg[0][1], bg[0][2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(halfW, 0, width - halfW, halfH);
	glClearColor(bg[1][0], bg[1][1], bg[1][2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(0, halfH, halfW, height - halfH);
	glClearColor(bg[2][0], bg[2][1], bg[2][2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(halfW, halfH, width - halfW, height - halfH);
	glClearColor(bg[3][0], bg[3][1], bg[3][2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_SCISSOR_TEST);


}
//void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) //--- 등록한 사용자 정의 콜백 함수
//{
//	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
//	{
//		double x, y;
//		glfwGetCursorPos(window, &x, &y); //--- 마우스의 위치를 얻어온다.
//		std::cout << "Clicked : " << x << ", " << y << std::endl;
//	}
//}
//void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) //--- 등록한 사용자 정의 콜백 함수
//{
//	std::cout << xpos << ", " << ypos << std::endl;
//}
void MouseButtonCallback(GLFWwindow* window,
	int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		if (width <= 0 || height <= 0) return;

		// 마우스 좌표 → 사각형을 그리는 -1~1 좌표
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		selectedArea = -1;
		selectedIndex = -1;

		// 나중에 그린 사각형부터 검사
		for (int area = 3; area >= 0; area--) {	// 1,2,3,4사분면 검사한다 
			for (int i = rectCount[area] - 1; i >= 0; i--) {
				const Rectangle& rect = rects[area][i];

				float left = std::min(rect.x1, rect.x2);	 //int a = 10;
															//int b = 20;
															//int result = std::min(a, b); // 10 반환
				float right = std::max(rect.x1, rect.x2);
				float bottom = std::min(rect.y1, rect.y2);
				float top = std::max(rect.y1, rect.y2);

				if (mx >= left && mx <= right &&
					my >= bottom && my <= top) {
					selectedArea = area;
					selectedIndex = i;
					return;
				}
			}
		}
	}
}
//
int main()
{
	//--- GLFW 초기화
	if (!glfwInit()) return -1;
	//--- OpenGL 버전 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//--- 윈도우 생성
	window = glfwCreateWindow(800, 600, "Keyboard Input", nullptr, nullptr);
	//--- 컨텍스트 활성화
	glfwMakeContextCurrent(window);
	//--- GLEW 초기화
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);
	//--- 키보드 콜백 함수 등록
	glfwSetKeyCallback(window, keyCallback);
	// --- 마우스 콜백 함수 등록
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	//--- 메인 루프
	cout << "tttttttty" << endl;
	while (!glfwWindowShouldClose(window)) {
		//--- 화면 그리기
		slice4background();

		//if(showRect)  glPrintRect(red, green, blue, rect_x1, rect_y1, rect_x2, rect_y2);

		for (int area = 0; area < 4; area++) {
			for (int i = 0; i < rectCount[area]; i++) {
				const Rectangle& rect = rects[area][i];

				glPrintRect(rect.red, rect.green, rect.blue,
					rect.x1, rect.y1, rect.x2, rect.y2);
			}
		}
		if (selectedArea >= 0 &&
			selectedIndex >= 0 &&
			selectedIndex < rectCount[selectedArea]) {

			const Rectangle& rect = rects[selectedArea][selectedIndex];

			glColor3f(1.0f, 0.0f, 0.0f); // 검정 테두리

			glBegin(GL_LINE_LOOP);
			glVertex2f(rect.x1, rect.y1); //꼭짓점 좌표 지정하는 함수 
			glVertex2f(rect.x2, rect.y1);
			glVertex2f(rect.x2, rect.y2);
			glVertex2f(rect.x1, rect.y2);
			glEnd();
		}
		glfwSwapBuffers(window);
		
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
//--- End of Main function
//--- 키보드 콜백 함수
