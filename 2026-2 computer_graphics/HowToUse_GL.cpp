//#include <GL/glew.h>
//#include <GL/glfw3.h>
//#include <iostream>
//int main() {
//	//--- GLFW 초기화
//	if (!glfwInit()) {
//		std::cerr << "GLFW 초기화 실패!" << std::endl;
//		return -1;
//	}
//	//--- OpenGL 버전 설정 (예: 3.3 Core Profile)
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	//--- 윈도우 생성
//	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
//	if (!window) {
//		std::cerr << "윈도우 생성 실패!" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	//--- 컨텍스트 설정
//	glfwMakeContextCurrent(window);
//	//--- GLEW 초기화
//	glewExperimental = GL_TRUE; // 최신 기능 사용
//	if (glewInit() != GLEW_OK) {
//		std::cerr << "GLEW 초기화 실패!" << std::endl;
//		return -1;
//	}
//		//--- 뷰포트 설정
//		glViewport(0, 0, 800, 600);
//	//--- 메인 루프
//	while (!glfwWindowShouldClose(window)) {
//		// 입력 처리
//		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//			glfwSetWindowShouldClose(window, true);
//		// 화면 지우기 (파란색)
//		glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // RGBA (파랑)
//		glClear(GL_COLOR_BUFFER_BIT);
//		// 버퍼 교체
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//	//--- 종료 처리
//	glfwDestroyWindow(window);
//	glfwTerminate();
//	return 0;
//}
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>
GLFWwindow* window = nullptr;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

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
		glClearColor(0.0, 1.0, 1.0, 1.0f);	//화면을 그립니다 
		glClear(GL_COLOR_BUFFER_BIT); // 프레임 버퍼의 내용을 초기화

		// 더 자세한 사항은 test3.cpp를 보세요

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

}

