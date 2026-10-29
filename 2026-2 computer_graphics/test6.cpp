#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>
GLFWwindow* window = nullptr;
#define DEFAULT 0.9

std::random_device dre;
std::mt19937 gen(dre());
using namespace std;

float prevMouseX = 0.0f;
float prevMouseY = 0.0f;
int selectedIndex = -1;
int RectCount = 0;
int pieceCount = 0;
int activePieceStart = -1;


struct Rect {
	float red, green, blue;
	float x1, y1, x2, y2;
	bool isShow = true;
	float speedX = DEFAULT;
	float speedY = DEFAULT;
	int mode = 0;
	Rect(float inx1, float iny1, float inx2, float iny2, float inred, float ingreen, float inblue) :
		x1{ inx1 }, y1{ iny1 }, x2{ inx2 }, y2{ iny2 }, red{ inred }, green{ ingreen }, blue{ inblue } {};
	Rect() {};
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
vector<Rect> pieces;

void InitRect() {
	std::uniform_real_distribution<float> color(0.0f, 1.0f); //색깔
	std::uniform_real_distribution<float> pos(-1.0f, 0.8f); //처음 위치
	std::uniform_int_distribution<int> rectcount(5, 10); // 사각형 갯수

	std::uniform_real_distribution<float> w(0.1f, 0.5f);
	std::uniform_real_distribution<float> h(0.1f, 0.5f);

	RectCount = rectcount(gen);

	for (int i = 0; i < RectCount; i++) {

		float x1 = pos(dre); float y1 = pos(dre);
		float width = w(dre); float height = h(dre);
		while ((x1 + width >= 1.0f) || (y1 + height >= 1.0f)){
			 x1 = pos(dre);  y1 = pos(dre);
			 width = w(dre);  height = h(dre);
		}
		float red = color(dre); float green = color(dre); float blue = color(dre);
		rects.push_back(Rect{ x1,y1,x1 + width ,y1 + height ,red,green,blue });
	}
	//cout << "가로: " << rects[0].width << "세로: " << rects[0].width << endl;
}
//for (Rect& rect : rects) {
//	if (rect.left() <= -1.0f || rect.right() >= 1.0f) {
//		rect.speedX *= -1.0f;
//	}
//
//	if (rect.bottom() <= -1.0f || rect.top() >= 1.0f) {
//		rect.speedY *= -1.0f;
//	}
//	float moveX = rect.speedX * dt;
//	float moveY = rect.speedY * dt;
//
//	rect.x1 += moveX;
//	rect.x2 += moveX;
//	rect.y1 += moveY;
//	rect.y2 += moveY;
//}
void split_Move1(float dt) {
	if (activePieceStart < 0) return;

	int i = activePieceStart;

	float distance = 0.2f * dt;

	for (int i = 0; i + 3 < pieces.size(); i += 4) {     if (pieces[i].mode != 1) continue; 

		pieces[i].move(-distance, 0.0f);
		pieces[i + 1].move(distance, 0.0f);
		pieces[i + 2].move(-distance, 0.0f);
		pieces[i + 3].move(distance, 0.0f);

	}

}
void split_Move2(float dt) {
	if (activePieceStart < 0) return;

	/*int i = activePieceStart;

	static int previousStart = -1;

	if (previousStart != activePieceStart) {
		pieces[i].speedX = -1.0f;
		pieces[i].speedY = 1.0f;

		pieces[i + 1].speedX = 1.0f;
		pieces[i + 1].speedY = 1.0f;

		pieces[i + 2].speedX = -1.0f;
		pieces[i + 2].speedY = -1.0f;

		pieces[i + 3].speedX = 1.0f;
		pieces[i + 3].speedY = -1.0f;

		previousStart = activePieceStart;
	}*/
	float distance = 0.2f + dt;

	for (int i = 0; i + 3 < pieces.size(); i += 4) { if (pieces[i].mode != 2) continue;
		for (int offset = 0; offset < 4; ++offset) {
			int idx = i + offset;
			if (pieces[idx].left() <= -1.0f || pieces[idx].right() >= 1.0f) {
				pieces[idx].speedX *= -1.0f;
			}
			if (pieces[idx].bottom() <= -1.0f || pieces[idx].top() >= 1.0f) {
				pieces[idx].speedY *= -1.0f;
			}
		}

		pieces[i].move(pieces[i].speedX * dt, pieces[i].speedY * dt);
		pieces[i + 1].move(pieces[i + 1].speedX * dt, pieces[i + 1].speedY * dt);
		pieces[i + 2].move(pieces[i + 2].speedX * dt, pieces[i + 2].speedY * dt);
		pieces[i + 3].move(pieces[i + 3].speedX * dt, pieces[i + 3].speedY * dt);
	}

}
void split_Move3(float dt) {
	if (activePieceStart < 0) return;

	int i = activePieceStart;

	float distance = 0.2f * dt;

	for (int i = 0; i + 3 < pieces.size(); i += 4) { if (pieces[i].mode != 3) continue;
		pieces[i].move(distance, 0.0f);
		pieces[i + 1].move(distance, 0.0f);
		pieces[i + 2].move(distance, 0.0f);
		pieces[i + 3].move(distance, 0.0f);

	}

} 
void split_Move4(float dt) { 
	for (Rect& piece : pieces) {
		piece.move(piece.speedX * dt, piece.speedY * dt);
	}
}
void split_4_pieces() { // 1. 클릭하면 즉시 4개로 나눌것인가
	float spiltX = 0.0f;
	float spiltY = 0.0f;

	Rect original = rects[selectedIndex];
	rects.erase(rects.begin() + selectedIndex);
	selectedIndex = -1;

	float cx = (original.left() + original.right()) / 2;
	float cy = (original.top() + original.bottom()) / 2;

	float left = original.left();
	float right = original.right();
	float top = original.top();
	float bottom = original.bottom();

	float gap = 0.02f;
	activePieceStart = static_cast<int>(pieces.size());
	pieces.push_back(Rect{
		   left - gap, cy + gap, cx - gap, top + gap,
		   original.red, original.green, original.blue
		}); // 왼쪽 위

	pieces.push_back(Rect{
		cx + gap, cy + gap, right + gap, top + gap,
		original.red, original.green, original.blue
		}); // 오른쪽 위

	pieces.push_back(Rect{
		left - gap, bottom - gap, cx - gap, cy - gap,
		original.red, original.green, original.blue
		}); // 왼쪽 아래

	pieces.push_back(Rect{
		cx + gap, bottom - gap, right + gap, cy - gap,
		original.red, original.green, original.blue
		}); // 오른쪽 아래

	for (int offset = 0; offset < 4; offset++) {
		Rect& piece = pieces[activePieceStart + offset];

		piece.mode = original.mode;

		if (piece.mode == 1) {
			// 현재 네 코드의 1번 동작: 왼쪽 두 개 / 오른쪽 두 개
			piece.speedX = (offset % 2 == 0) ? -0.2f : 0.2f;
			piece.speedY = 0.0f;
		}
		else if (piece.mode == 2) {
			// 왼쪽 위, 오른쪽 위, 왼쪽 아래, 오른쪽 아래
			piece.speedX = (offset % 2 == 0) ? -1.0f : 1.0f;
			piece.speedY = (offset < 2) ? 1.0f : -1.0f;
		}
		else if (piece.mode == 3) {
			// 네 조각 모두 오른쪽
			piece.speedX = 0.2f;
			piece.speedY = 0.0f;
		}
	}
	static double lastTime = glfwGetTime();

	pieceCount = static_cast<int>(pieces.size());
}

void split_8_pieces() { // 2. 클릭하면 즉시 8개로 나눌 것인가 
	if (selectedIndex < 0 || selectedIndex >= static_cast<int>(rects.size())) return;

	Rect original = rects[selectedIndex];
	rects.erase(rects.begin() + selectedIndex);
	selectedIndex = -1;

	float cx = (original.left() + original.right()) / 2.0f;
	float cy = (original.bottom() + original.top()) / 2.0f;
	float width = (original.right() - original.left()) / 3.0f;
	float height = (original.top() - original.bottom()) / 3.0f;
	activePieceStart = static_cast<int>(pieces.size());

	// 3 x 3 배치에서 가운데를 제외한 8개를 생성한다.
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			if (x == 0 && y == 0) continue;

			float px = cx + x * width;
			float py = cy + y * height;
			Rect piece{
				px - width / 2.0f, py - height / 2.0f,
				px + width / 2.0f, py + height / 2.0f,
				original.red, original.green, original.blue
			};

			// 대각선도 상하좌우와 같은 속도로 이동한다.
			float length = std::sqrt(static_cast<float>(x * x + y * y));
			piece.speedX = 0.2f * x / length;
			piece.speedY = 0.2f * y / length;
			pieces.push_back(piece);
			piece.mode = original.mode;

		}
	}
	pieceCount = static_cast<int>(pieces.size());
}
void UpDate() {
	static double lastTime = glfwGetTime();

	double now = glfwGetTime();
	float dt = static_cast<float>(now - lastTime);
	lastTime = now;

	float amount = 0.02f * dt;

	for (Rect& piece : pieces) {
		if (!piece.isShow) continue;

		float width = piece.right() - piece.left();
		float height = piece.top() - piece.bottom();

		// 너무 작아지면 숨김: 좌표가 뒤집히는 것도 방지
		if (width <= 2.0f * amount + 0.01f ||
			height <= 2.0f * amount + 0.01f) {
			piece.isShow = false;
			continue;
		}

		piece.x1 += amount;
		piece.x2 -= amount;
		piece.y1 += amount;
		piece.y2 -= amount;
		piece.red -= .0001; piece.green -= .0001; piece.blue -= .0001;
	}
	split_Move1(dt);
	split_Move2(dt);
	split_Move3(dt);
	split_Move4(dt);
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
		UpDate();
		for (const Rect& rect : rects) {
			if (rect.isShow) {
				glColor3f(rect.red, rect.green, rect.blue);
				glRectf(rect.x1, rect.y1, rect.x2, rect.y2);
			}
		}
		for (const Rect& piece : pieces) {
			if (piece.isShow) {
				glColor3f(piece.red, piece.green, piece.blue);
				glRectf(piece.x1, piece.y1, piece.x2, piece.y2);
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

}
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) //--- 등록한 사용자 정의 콜백 함수
{

}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		selectedIndex = -1;

		// 나중에 그린 사각형부터 검사
		for (int i = static_cast<int>(rects.size()) - 1; i >= 0; i--) {
			if (rects.empty()) {
				return;
			}
			 Rect& rect = rects[i];

			if (mx >= rect.left() && mx <= rect.right() &&
				my >= rect.bottom() && my <= rect.top()) {
				selectedIndex = i;		// 여기서 마우스로 클릭한게 selected index로 값을 대입한다 
				prevMouseX = mx; //(!마우스드래그)
				prevMouseY = my; //(!마우스드래그)
				cout << "선택한 사각형 " << selectedIndex << endl;

				uniform_int_distribution<int> m(1, 4); // 모드 선택
				rect.mode = m(dre);

				if (rect.mode == 1 || rect.mode == 2 || rect.mode == 3) {
					split_4_pieces();
				}
				else if (rect.mode == 4) {
					split_8_pieces();
				}
				return;
			}

		}

	}
}
