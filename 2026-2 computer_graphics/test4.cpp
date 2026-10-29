#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>

#define MAXCOUNT 5
#define DEFAULT 0.9
GLFWwindow* window = nullptr;
std::mt19937 dre(std::random_device{}());


float prevMouseX = 0.0f, prevMouseY = 0.0f; // 클릭 위치를 알아보기(!마우스드래그)

struct Rect{
	float red, green, blue;
	float x1, y1, x2, y2;
	bool show = false;
	float width = 0.2f;
	float height = 0.2f;
	float speedX = DEFAULT;
	float speedY = DEFAULT;

	float verticalDistance = 0.0f;
	bool verticalMoving = false;
	float verticalDirection = -1.0f; // -1: 아래, 1: 위

	float left() const {
		return std::min(x1, x2);
	}

	float right() const {
		return std::max(x1, x2);
	}

	float bottom() const {
		return std::min(y1, y2);
	}

	float top() const {
		return std::max(y1, y2);
	}

	float width_len() const {
		return abs(x1 - x2);
	}
	float height_len() const {
		return abs(y1 - y2);
	}
};
struct Rect_Mem {
	float x1, x2, y1, y2;
};
struct Toggle {
	bool is1;
	bool is2;
	bool is3;
	bool is4;
	bool is5;

};
Toggle toggle{};

std::vector<Rect> rects;
std::vector<Rect_Mem> rects_mem;
int rectCount = 0;

//----------------------------------------------------------------------------------------------------------------------함수 정의선
void glPrintRect(GLfloat red, GLfloat green, GLfloat blue,
	GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glColor3f(red, green, blue);
	glRectf(x1, y1, x2, y2);
}
bool is2(Rect& rect, float dt) {
	float moveY = rect.speedY * dt;
	float remaining = rect.height - rect.verticalDistance;
	moveY = std::min(moveY, remaining);

	rect.y1 += moveY * rect.verticalDirection;
	rect.y2 += moveY * rect.verticalDirection;
	rect.verticalDistance += moveY;

	if (rect.verticalDirection < 0.0f && rect.bottom() <= -1.0f) {
		float correction = -1.0f - rect.bottom();
		rect.y1 += correction;
		rect.y2 += correction;

		rect.verticalDirection = 1.0f;
		rect.verticalDistance = 0.0f;
		return true;
	}
	if (rect.verticalDirection > 0.0f && rect.top() >= 1.0f) {
		float correction = 1.0f - rect.top();
		rect.y1 += correction;
		rect.y2 += correction;

		rect.verticalDirection = -1.0f;
		rect.verticalDistance = 0.0f; //세로로 이동한 거리 초기화
		return true;
	}
	if (rect.verticalDistance >= rect.height) {
		rect.verticalDistance = 0.0f;
		return true; // 세로 길이만큼 이동 완료
	}

	return false; // 아직 이동 중
}

void Movement(float dt) {

	if (toggle.is1) { // 대각선 이동
		for (Rect& rect : rects) {
			if (rect.left() <= -1.0f || rect.right() >= 1.0f) {
				rect.speedX *= -1.0f;
			}

			if (rect.bottom() <= -1.0f || rect.top() >= 1.0f) {
				rect.speedY *= -1.0f;
			}
			float moveX = rect.speedX * dt;
			float moveY = rect.speedY * dt;

			rect.x1 += moveX;
			rect.x2 += moveX;
			rect.y1 += moveY;
			rect.y2 += moveY;
		}
	}
	//else if (toggle.is2) {
	//	for (Rect& rect : rects) {

	//		if (rect.left() <= -1.0f || rect.right() >= 1.0f) {
	//			rect.verticalMoving = true;
	//			if (rect.verticalMoving) {
	//				bool finished = is2(rect, dt);

	//				if (finished) {
	//					rect.verticalMoving = false; // 세로 이동 종료
	//					rect.speedX *= -1.0f;        // 가로 방향 반전
	//				}
	//			}
	//		}

	//		float moveX = rect.speedX * dt;

	//		rect.x1 += moveX;
	//		rect.x2 += moveX;

	//	}
	//}
	else if (toggle.is2) {
		for (Rect& rect : rects) {

			// 벽에 닿은 뒤 세로 이동 중
			if (rect.verticalMoving) {
				if (is2(rect, dt)) {
					rect.verticalMoving = false;
					rect.speedX *= -1.0f;
				}
				continue;
			}

			// 오른쪽 벽 충돌
			if (rect.right() >= 1.0f && rect.speedX > 0.0f) {
				float correction = rect.right() - 1.0f;
				rect.x1 -= correction;
				rect.x2 -= correction;

				rect.verticalMoving = true;
				rect.verticalDistance = 0.0f;
				continue;
			}

			// 왼쪽 벽 충돌
			if (rect.left() <= -1.0f && rect.speedX < 0.0f) {
				float correction = -1.0f - rect.left();
				rect.x1 += correction;
				rect.x2 += correction;

				rect.verticalMoving = true;
				rect.verticalDistance = 0.0f;
				continue;
			}
		
			// 평소에는 가로 이동
			float moveX = rect.speedX * dt;
			rect.x1 += moveX;
			rect.x2 += moveX;
		}
	}
	else if (toggle.is3) {
		for (Rect& rect : rects) {
			float moveX = rect.speedX * dt;
			float moveY = rect.speedY * dt;

			if (rect.right() > 1.0f && rect.speedX > 0.0f) {
				float correction = rect.right() - 1.0f;
				rect.x1 -= correction;
				rect.x2 -= correction;

				rect.speedX = 0;
				rect.speedY = -0.3f;
			}
			else if (rect.bottom() < -1.0 && rect.speedY < 0.0f)  {
				float correction = -1.0 - rect.bottom() ;
				rect.y1 += correction;
				rect.y2 += correction;

				rect.speedX = -0.3f;
				rect.speedY = 0;
			}
			else if (rect.left() < -1.0 && rect.speedX < 0.0f) {
				float correction = -1.0 - rect.left();
				rect.x1 += correction;
				rect.x2 += correction;

				rect.speedX = 0;
				rect.speedY = 0.3f;
			}
			else if (rect.top() > 1.0 && rect.speedY > 0.0f) {
				float correction = rect.top() - 1.0f;
				rect.y1 -= correction;
				rect.y2 -= correction;

				rect.speedX = 0.3f;
				rect.speedY = 0;
			}
			rect.x1 += moveX;
			rect.x2 += moveX;
			rect.y1 += moveY;
			rect.y2 += moveY;
		}
	}
	else if (toggle.is4) {
		static float dirX = 1.0f;
		static float dirY = 1.0f;
		
		for (Rect& rect : rects) {

			if (rect.width_len() < 0.15) {
				dirX = -1.0f;

			}
			else if (rect.width_len() > 0.3) {
				dirX = 1.0f;
			}
			if (rect.height_len() < 0.15) {
				dirY = -1.0f;
			}
			else if (rect.height_len() > 0.3) {
				dirY = 1.0f;
			}
			float moveX = 0.1 * dt * dirX;
			float moveY = 0.1 * dt * dirY;

			rect.x1 += moveX;
			rect.x2 -= moveX;
			rect.y1 += moveY;
			rect.y2 -= moveY;
		}
	}
	else if (toggle.is5) {
		static double lastTime = glfwGetTime();
		std::uniform_real_distribution<float> color(0.0f, 1.0f);

		if (glfwGetTime() - lastTime >= 1.0) {
			for (Rect& rect : rects) {
				rect.red = color(dre); rect.green = color(dre); rect.blue = color(dre);
			}
			lastTime = glfwGetTime();
		}

	}
}
void UpDate() {
	static double lastTime = glfwGetTime();

	double now = glfwGetTime();
	float dt = static_cast<float>(now - lastTime);
	lastTime = now;

	Movement(dt);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		// ESC 키를 눌렀을 때 작업
		glfwSetWindowShouldClose(window, GLFW_TRUE);	
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		toggle.is1 = !toggle.is1; toggle.is2 = false; toggle.is3 = false; toggle.is4 = false; toggle.is5 = false;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		toggle.is2 = !toggle.is2; toggle.is1 = false; toggle.is3 = false; toggle.is4 = false; toggle.is5 = false;
		if (toggle.is2) {
			for (Rect& rect : rects) {
				rect.speedX = DEFAULT;
				rect.speedY = DEFAULT;
			}
		}
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		toggle.is3 = !toggle.is3; toggle.is2 = false; toggle.is1 = false; toggle.is4 = false; toggle.is5 = false;
		if (toggle.is3) {
			for (Rect& rect : rects) {
				rect.speedX = DEFAULT;
				rect.speedY = 0;
			}
		}
	}
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		toggle.is4 = !toggle.is4; toggle.is2 = false; toggle.is3 = false; toggle.is1 = false; toggle.is5 = false;
	}
	else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		toggle.is5 = !toggle.is5; toggle.is2 = false; toggle.is3 = false; toggle.is4 = false; toggle.is1 = false;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		toggle.is1 = false;  toggle.is2 = false; toggle.is3 = false; toggle.is4 = false;  toggle.is5 = false;
	}
	else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		for (int i = 0; i < rectCount; i++) {
			Rect& rect = rects[i];
			const Rect_Mem& rect_mem = rects_mem[i];

			rect.x1 = rect_mem.x1;
			rect.x2 = rect_mem.x2;
			rect.y1 = rect_mem.y1;
			rect.y2 = rect_mem.y2;
		}
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		rects.clear();
		rects_mem.clear();
		rectCount = 0;
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

	prevMouseX = mx; //mx와 my는 마우스를 움직이면서 지금 내 위치를 말하는 것이다.
	prevMouseY = my;


}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	/*if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		if (selectedIndex >= 0 && selectedIndex < rectCount) {
			사각형_두개만들기();
		}
		return;
	}*/
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (rectCount >= 5) return;

		double x, y;
		glfwGetCursorPos(window, &x, &y);		//마우스 움직이는거 좌표 알게 하자 

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		if (width <= 0 || height <= 0) return;
		rects.push_back(Rect{}); // 먼저 사각형 추가 
		Rect& rect = rects.back(); // 마지막으로 추가한 사각형 참조

		
		rects_mem.push_back(Rect_Mem{ });
		Rect_Mem& rect_mem = rects_mem.back();
		// 마우스 좌표 → 사각형을 그리는 -1~1 좌표
		float mx = static_cast<float>(2.0 * x / width - 1.0);
		float my = static_cast<float>(1.0 - 2.0 * y / height);

		std::uniform_real_distribution<float> color(0.0f, 1.0f);

		float hwidth = rect.width / 2;
		float hheight = rect.height / 2;
		
		rect.red = color(dre); rect.blue = color(dre); rect.green = color(dre);
		rect.x1 = mx - hwidth;
		rect.y1 = my - hheight;
		rect.x2 = mx + hwidth;
		rect.y2 = my + hheight;

		rect_mem.x1 = rect.x1;
		rect_mem.x2 = rect.x2;
		rect_mem.y1 = rect.y1;
		rect_mem.y2 = rect.y2;

		rectCount++;

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
	//glfwSetCursorPosCallback(window, CursorPosCallback);     // 마우스 움직임 함수 등록

	double lastTime = glfwGetTime(); 
	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		//--- 화면 그리기
		glClearColor(0.3, 0.3, 0.3, 1.0f);	//화면을 그립니다 
		glClear(GL_COLOR_BUFFER_BIT); // 프레임 버퍼의 내용을 초기화

		UpDate();

		for (const Rect& rect : rects) {
			glPrintRect(rect.red, rect.green, rect.blue,
				rect.x1, rect.y1, rect.x2, rect.y2);
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
