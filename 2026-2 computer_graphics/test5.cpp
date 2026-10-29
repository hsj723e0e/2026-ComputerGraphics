#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>
GLFWwindow* window = nullptr;

const float RECTX = 0.08f;
const float RECTY = 0.106f;
std::random_device dre;
std::mt19937 gen(dre());
using namespace std;

float prevMouseX = 0.0f;
float prevMouseY = 0.0f;
int spareRect = 0;

struct Rect {
	float red, green, blue;
	float x1, y1, x2, y2;
	float width = RECTX;
	float height = RECTY;
	bool isShow = true;
	Rect(float inx1, float iny1, float inx2, float iny2, float inred, float ingreen, float inblue) :
		x1{ inx1 }, y1{ iny1 }, x2{ inx2 }, y2{ iny2 }, red{ inred }, green{ ingreen }, blue{ inblue } {};
	Rect() {};
	void move(float dx, float dy) {
		x1 += dx;
		x2 += dx;
		y1 += dy;
		y2 += dy;
	}
	float left() const {return std::min(x1, x2);}
	float right() const {return std::max(x1, x2);}
	float bottom() const {return std::min(y1, y2);}
	float top() const { return std::max(y1, y2);}


};
struct Eraser {
	float red, green, blue;
	float x1, y1, x2, y2;
	float width = RECTX * 2;
	float height = RECTY * 2;
	bool isShow;
	Eraser(float inx1, float iny1, float inx2, float iny2, float inred, float ingreen, float inblue) :
		x1{ inx1 }, y1{ iny1 }, x2{ inx2 }, y2{ iny2 }, red{ inred }, green{ ingreen }, blue{ inblue } {};
	Eraser() {};
	void move(float dx, float dy) {
		x1 += dx;
		x2 += dx;
		y1 += dy;
		y2 += dy;
	}
	float left() const { return std::min(x1, x2); }
	float right() const { return std::max(x1, x2); }
	float bottom() const { return std::min(y1, y2); }
	float top() const { return std::max(y1, y2); }


};
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

vector<Rect> rects;
vector<Eraser> eraser;
void InitRect() {
	std::uniform_real_distribution<float> color(0.0f, 1.0f);
	std::uniform_real_distribution<float> pos(-1.0f, 0.9f);

	std::uniform_int_distribution<int> RectCount(20, 40);

	int count = RectCount(gen);
	for (int i = 0; i < count; i++) {

		float x1 = pos(dre); float y1 = pos(dre);
		float red = color(dre); float green = color(dre); float blue = color(dre);
		rects.push_back(Rect{ x1,y1,x1+ RECTX,y1+RECTY,red,green,blue });
	}
	//cout << "가로: " << rects[0].width << "세로: " << rects[0].width << endl;
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


	InitRect();

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		//--- 화면 그리기
		glClearColor(0.0, 1.0, 1.0, 1.0f);	//화면을 그립니다 
		glClear(GL_COLOR_BUFFER_BIT); // 프레임 버퍼의 내용을 초기화

		for (const Rect& rect : rects) {
			if (rect.isShow) {
				glColor3f(rect.red, rect.green, rect.blue);
				glRectf(rect.x1, rect.y1, rect.x2, rect.y2);
			}
		}
		for (const Eraser& e : eraser) {
			if (e.isShow) {
				glColor3f(e.red, e.green, e.blue);
				glRectf(e.x1, e.y1, e.x2, e.y2);
			}
		}
		glfwSwapBuffers(window); //버퍼교체

		glfwPollEvents(); //이벤트 큐에 있는거 가져와 실행시키기
	}
	glfwDestroyWindow(window); //생성한 윈도우 객체를 삭제

	glfwTerminate(); //GLFW 사용을 끝내고, GLFW가 사용하던 자원을 정리
	return 0;
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		rects.clear();
		spareRect = 0;
		InitRect();
	}
}
bool isTouching(const Rect& rect, const Eraser& eraser) {
	if (rect.right() >= eraser.left() && rect.left() <= eraser.right() &&
		rect.top() >= eraser.bottom() && rect.bottom() <= eraser.top() && rect.isShow) {
		return true;
	}
	else {
		return false;
	}
}
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) //--- 등록한 사용자 정의 콜백 함수
{	//(!마우스드래그)
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

	if (eraser.empty()) {
		return;
	}
	Eraser& e = eraser.back();
	e.x1 += dx;
	e.x2 += dx;
	e.y1 += dy;
	e.y2 += dy;

	prevMouseX = mx; //mx와 my는 마우스를 움직이면서 지금 내 위치를 말하는 것이다.
	prevMouseY = my;
	
	for (auto it = rects.begin(); it != rects.end();) { // 닿으면 rect가 없어지는 코드
		if (isTouching(*it, e)) {
			float centerX = (e.x1 + e.x2) / 2.0f;
			float centerY = (e.y1 + e.y2) / 2.0f;
			
			e.width *= 1.1f;
			e.height *= 1.1f;

			e.x1 = centerX - e.width / 2.0f;
			e.x2 = centerX + e.width / 2.0f;
			e.y1 = centerY + e.height / 2.0f;
			e.y2 = centerY - e.height / 2.0f;

			e.red = it->red; e.green = it->green; e.blue = it->blue;
			//it = rects.erase(it);
			it->isShow = false;

		}
		else {
			++it;
		}

	}
	
}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		uniform_real_distribution<float> color(0.0f, 1.0f);
		if (width <= 0 || height <= 0) return;
		if (eraser.empty()) {
			eraser.push_back(Eraser{ }); // 먼저 사각형 추가 
		}
		Eraser& e = eraser.back(); // 마지막으로 추가한 사각형 참조
		e.x1 = mx - (e.width / 2);
		e.x2 = mx + (e.width / 2);
		e.y1 = my + (e.height / 2);
		e.y2 = my - (e.height / 2);
		e.red = 0.0f; e.green = 0.0f; e.blue = 0.0f;
		e.isShow = true;

		if (mx >= e.left() && mx <= e.right() &&
			my >= e.bottom() && my <= e.top()) {
			prevMouseX = mx; //(!마우스드래그)
			prevMouseY = my; //(!마우스드래그)
			return;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		if (eraser.empty()) {
			return;
		}
		Eraser& e = eraser.back();
		e.width = 0.2f;
		e.height = 0.266;
		e.isShow = false;
		for (Rect& rect : rects) {
			rect.isShow = true;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (spareRect >= 10)return;
		rects.push_back(Rect{});

		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);
		
		Rect& rect = rects.back();
		rect.x1 = mx - (rect.width / 2);
		rect.x2 = mx + (rect.width / 2);
		rect.y1 = my + (rect.height / 2);
		rect.y2 = my - (rect.height / 2);
		uniform_real_distribution<float> color(0.0f, 1.0f);

		rect.red = color(dre); rect.green = color(dre); rect.blue = color(dre);
		spareRect++;
		if (eraser.empty()) {
			return;
		}
		Eraser& e = eraser.back();
		
		float centerX = (e.x1 + e.x2) / 2.0f;
		float centerY = (e.y1 + e.y2) / 2.0f;

		e.width *= .9f;
		e.height *= .9f;

		e.x1 = (centerX - e.width)/ 2.0f;
		e.x2 = (centerX + e.width) / 2.0f;
		e.y1 = (centerY + e.height) / 2.0f;
		e.y2 = (centerY - e.height) / 2.0f;
	}
	cout << "사각형 갯수: " << rects.size() << endl;
}
