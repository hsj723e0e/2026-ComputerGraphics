#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
using namespace std;

float r = 1.0f, g = 1.0f, b = 1.0f;
std::mt19937 dre(std::random_device{}());
double last_time = 0.0;
double n = 2.0;
bool isT = false;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		// ESC 키를 눌렀을 때 작업
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		r = 0.0f, g = 0.5f, b = 0.5f; // 청록색
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		r = 1.0f, g = 0.0f, b = 1.0f; // 자홍색
	}
	if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		r = 1.0f, g = 1.0f, b = 0.0f; // 노랑색
	}
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		r = 0.5f, g = 0.5f, b = 0.5f; // 회색
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		r = 0.0f, g = 0.0f, b = 0.0f; // 검정색
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		std::uniform_real_distribution<float> uid(0.0f, 1.0f);
		r = uid(dre), g = uid(dre), b = uid(dre); // 랜덤색
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		isT = true;


	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		isT = false;

	}
}
int main()
{
	//--- GLFW 초기화
	if (!glfwInit()) return -1;
		//--- OpenGL 버전 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "Keyboard Input", nullptr, nullptr);
	//--- 컨텍스트 활성화
	glfwMakeContextCurrent(window);
	//--- GLEW 초기화
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);
	//--- 키보드 콜백 함수 등록
	glfwSetKeyCallback(window, keyCallback);
	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
			//--- 화면 그리기
		glClearColor(r, g, b, 1.0f);	//화면을 그립니다 
		glClear(GL_COLOR_BUFFER_BIT);

		double time = glfwGetTime();	// 시간을 반환합니다. 1.2초뒤에는 time값이 1.2입니다.
		if (time - last_time >= n) {
			if (isT) {
				std::uniform_real_distribution<float> uid(0.0f, 1.0f);

				r = uid(dre), g = uid(dre), b = uid(dre);
				last_time = time;
			}
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
