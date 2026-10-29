#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
using namespace std;
GLFWwindow* window = nullptr;
std::mt19937 dre(std::random_device{}());

//GLfloat red, green, blue, rect_x1, rect_y1, rect_x2, rect_y2;
int selectedIndex = -1;
float prevMouseX = 0.0f, prevMouseY = 0.0f; // 클릭 위치를 알아보기(!마우스드래그)

struct Rectangle {
	float red, green, blue;
	float x1, y1, x2, y2;
	bool show = false;
	int width; int height;
};
Rectangle rects[20]{};

int rectCount = 0;

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

	

	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		if (rectCount >= 10) return;

		Rectangle& rect = rects[rectCount];

		uniform_real_distribution<float> color(0.0f, 1.0f);


		uniform_real_distribution<float> posX(-1.0f, 1.0f);
		uniform_real_distribution<float> posY(-1.0f, 1.0f);

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
		rects[rectCount].width = rects[rectCount].x2 - rects[rectCount].x1;
		rects[rectCount].height = rects[rectCount].y2 - rects[rectCount].y1;

		rectCount++;
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		uniform_real_distribution<float> color(0.0f, 1.0f);

		Rectangle& rect = rects[selectedIndex];
		rect.red = color(dre);
		rect.green = color(dre);
		rect.blue = color(dre);
	}
	
		
}
void 사각형_두개만들기();
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) //--- 등록한 사용자 정의 콜백 함수
{	//(!마우스드래그)
	if (selectedIndex < 0 || selectedIndex >= rectCount) return;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)return; //마우스 떼면 리턴

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (width <= 0 || height <= 0)return;

	float mx = static_cast<float>(2.0 * xpos / width - 1.0);
	float my = static_cast<float>(1.0 - 2.0 * ypos / height);

	float dx = mx - prevMouseX; // mx는 내가 지금 움직인 위치, prevMouse는 
								// 마우스를 클릭했을 때 위치 
								// 따라서 dx = 마우스를 클릭하고 내가 마우스를 이동한 거리
	float dy = my - prevMouseY;

	Rectangle& rect = rects[selectedIndex];
	rect.x1 += dx;
	rect.x2 += dx;
	rect.y1 += dy;
	rect.y2 += dy;

	prevMouseX = mx; //mx와 my는 마우스를 움직이면서 지금 내 위치를 말하는 것이다.
	prevMouseY = my;


}
void splitRectangle(int index) {
	if (index < 0 || index >= rectCount) return;
	if (rectCount >= 20) return; // 하나가 두 개가 되므로 빈자리 필요

	Rectangle original = rects[index];

	float left = std::min(original.x1, original.x2);
	float right = std::max(original.x1, original.x2);
	float top = std::max(original.y1, original.y2);    // 위쪽
	float bottom = std::min(original.y1, original.y2); // 아래쪽

	float middle = (left + right) / 2.0f;
	float gapX = (right - left) * 0.39f;
	float gapY = (top - bottom);

	uniform_real_distribution<float> randomX(gapX*0.5, gapX);

	float a = randomX(dre);
	float b = randomX(dre);

	rects[index].x1 = left - a;
	rects[index].x2 = middle - a;
	rects[index].y1 = bottom;
	rects[index].y2 = top;

	rects[rectCount] = original;
	rects[rectCount].x1 = middle +  b;
	rects[rectCount].x2 = right + b;
	rects[rectCount].y1 = top ;
	rects[rectCount].y2 = bottom;

	uniform_real_distribution<float> color(0.0f, 1.0f);

	rects[index].red = color(dre);
	rects[index].green = color(dre);
	rects[index].blue = color(dre);

	rects[rectCount].red = color(dre);
	rects[rectCount].green = color(dre);
	rects[rectCount].blue = color(dre);


	rectCount++;
}

void MouseButtonCallback(GLFWwindow* window,int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		if (selectedIndex >= 0 && selectedIndex < rectCount) {
			사각형_두개만들기();
		}
		return;
	}
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
		for (int i = rectCount- 1; i >= 0; i--) {
			const Rectangle& rect = rects[i];

			float left = std::min(rect.x1, rect.x2);	 //int a = 10;
			//int b = 20;
			//int result = std::min(a, b); // 10 반환
			float right = std::max(rect.x1, rect.x2);
			float bottom = std::min(rect.y1, rect.y2);
			float top = std::max(rect.y1, rect.y2);

			if (mx >= left && mx <= right &&
				my >= bottom && my <= top) {
				selectedIndex = i;		// 여기서 마우스로 클릭한게 selected index로 값을 대입한다 
				prevMouseX = mx; //(!마우스드래그)
				prevMouseY = my; //(!마우스드래그)
				return;
			}
		}
		
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
		selectedIndex = -1;

		// 나중에 그린 사각형부터 검사
		for (int i = rectCount - 1; i >= 0; i--) {
			const Rectangle& rect = rects[i];

			float left = std::min(rect.x1, rect.x2);	 //int a = 10;
			//int b = 20;
			//int result = std::min(a, b); // 10 반환
			float right = std::max(rect.x1, rect.x2);
			float bottom = std::min(rect.y1, rect.y2);
			float top = std::max(rect.y1, rect.y2);

			if (mx >= left && mx <= right &&
				my >= bottom && my <= top) {
				selectedIndex = i;		// 여기서 마우스로 클릭한게 selected index로 값을 대입한다 
				splitRectangle(i);

				return;
			}
		}

	}


}

int search_overlap() {
	const Rectangle& a = rects[selectedIndex]; //드래그중인 사각형

	float aleft = std::min(a.x1, a.x2);
	float aright = std::max(a.x1, a.x2);
	float atop = std::max(a.y1, a.y2);
	float abottom = std::min(a.y1, a.y2);
	
	int index = 0;
	for (int index = 0; index < rectCount; index++) {
		if (index == selectedIndex) continue;
		const Rectangle& b = rects[index];

		float bleft = std::min(b.x1,b.x2);
		float bright = std::max(b.x1, b.x2);
		float btop = std::max(b.y1, b.y2);
		float bbottom = std::min(b.y1, b.y2);


		if ((aright > bleft) && (bright > aleft) && (btop > abottom) && (atop > bbottom)) {
			return index;

		}
	}
	return -1;

}
void 사각형_두개만들기() {
	int index = search_overlap(); // 드래그에서 겹쳐진 사각형

	if (index == -1)return;
	const Rectangle& a = rects[selectedIndex]; //드래그중인 사각형
	const Rectangle& b = rects[index];

	float left = std::min({ a.x1, a.x2, b.x1, b.x2 });
	float right = std::max({ a.x1, a.x2, b.x1, b.x2 });
	float bottom = std::min({ a.y1, a.y2, b.y1, b.y2 });
	float top = std::max({ a.y1, a.y2, b.y1, b.y2 });

	uniform_real_distribution<float> color(0.0f, 1.0f);
	

	/*for (int i = selectedIndex; i < rectCount - 1; i++) {
		rects[i] = rects[i + 1];
	}
	rectCount--;
	for (int i = index; i < rectCount - 1; i++) {
		rects[index] = rects[index + 1];
	}
	rectCount--;

	
	glPrintRect(color(dre), color(dre), color(dre), left, top, right, bottom);
	rectCount++;*/
	Rectangle& merged = rects[selectedIndex];

	merged.x1 = left;
	merged.x2 = right;
	merged.y1 = bottom;
	merged.y2 = top;

	merged.red = color(dre);
	merged.green = color(dre);
	merged.blue = color(dre);

	// 상대 사각형만 삭제
	for (int i = index; i < rectCount - 1; i++) {
		rects[i] = rects[i + 1];
	}
	rectCount--;

	// 삭제로 밀린 선택 번호 수정
	if (index < selectedIndex) {
		selectedIndex--;
	}

}

int main()
{
	//--- GLFW 초기화
	if (!glfwInit()) return -1; // G
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// OpenGL 버전 설정 (정수점)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGL 버전 설정 (소수점)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // modern openGL을 쓸거냐, 레거시 openGL를 쓸거냐 (3주차까지만)
	window = glfwCreateWindow(800, 600, "Keyboard Input", nullptr, nullptr); // 윈도우 생성(창 + OpenGL Context 생성)
	glfwMakeContextCurrent(window); // 컨텍스트 활성화(생성한 Context를 현재 사용할 것으로 지정)
	glewExperimental = GL_TRUE; // 최신 기능 사용
	glewInit(); // GLEW초기화 (1.이거하면 OpenGL modern 함수 사용가능  2.두 단계 이후 호출)
	glViewport(0, 0, 800, 600);					// 뷰포트 설정
	glfwSetKeyCallback(window, keyCallback);			// 키보드 콜백 함수 등록
	glfwSetMouseButtonCallback(window, MouseButtonCallback);	// 마우스 콜백 함수 등록
	glfwSetCursorPosCallback(window, CursorPosCallback);     // 마우스 움직임 함수 등록

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		//--- 화면 그리기
		glClearColor(1.0, 1.0, 1.0, 1.0f);	//화면을 그립니다 
		glClear(GL_COLOR_BUFFER_BIT); // 프레임 버퍼의 내용을 초기화

		for (int index = 0; index < rectCount; index++) {
			const Rectangle& rect = rects[index];

			glPrintRect(rect.red, rect.green, rect.blue,
				rect.x1, rect.y1, rect.x2, rect.y2);
		}
		if (selectedIndex >= 0 && selectedIndex < rectCount) {

			const Rectangle& rect = rects[selectedIndex];

			glColor3f(1.0f, 0.0f, 0.0f); // 검정 테두리

			glBegin(GL_LINE_LOOP);		// 테두리 그리는 거
			glVertex2f(rect.x1, rect.y1); //꼭짓점 좌표 지정하는 함수 
			glVertex2f(rect.x2, rect.y1);
			glVertex2f(rect.x2, rect.y2);
			glVertex2f(rect.x1, rect.y2);
			glEnd();
		}
		glfwSwapBuffers(window); //버퍼교체

		glfwPollEvents(); //이벤트 큐에 있는거 가져와 실행시키기
	}
	glfwDestroyWindow(window); //생성한 윈도우 객체를 삭제

	glfwTerminate(); //GLFW 사용을 끝내고, GLFW가 사용하던 자원을 정리
	return 0;
}
//--- End of Main function
//--- 키보드 콜백 함수