#include <iostream>
#include <fstream>
#include <string>
//--- 필요한 헤더파일 선언
#include <GL/glew.h>
#include <GL/glfw3.h>
using namespace std;
//--- 사용자 정의 함수
void make_vertexShaders(); // 버텍스 세이더 객체 만들기
void make_fragmentShaders(); // 프래그먼트 세이더 객체 만들기
GLuint make_shaderProgram();  // 세이터 프로그램 생성하기 
void InitBuffer();
GLvoid drawScene(); // 출력함수 
GLvoid Reshape(int w, int h);
//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; // 쉐이더 프로그램 만들기
GLuint vertexShader;// 얘는 버텍스 세이더 객체
GLuint fragmentShader; // 얘는 프래그먼트 세이터 객체 
GLuint vao, vbo[2]; // 버퍼 생성 

const GLfloat triShape[3][3] = {
	{-0.5f, -0.5f, 0.0f},
	{ 0.5f, -0.5f, 0.0f},
	{ 0.0f,  0.5f, 0.0f}
};

const GLfloat colors[3][3] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f}
};

std::string filetobuf(const char* file)
{
	std::ifstream shaderFile(file);

	if (!shaderFile.is_open())
	{
		return "";
	}

	std::string source(
		(std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	shaderFile.close();
	return source;
}
//--- 메인 함수
int main(int argc, char** argv)
{
	width = 500;
	height = 500;
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
	//--- 렌더링 루프
	while (!glfwWindowShouldClose(window))
	{
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
	//--- 점의 크기 설정
	//glPointSize(5.0f);
	//--- 삼각형 그리기
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	std::string vertexSource = filetobuf("vertex.glsl"); // filetobuf: 사용자 정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	const char* source = vertexSource.c_str();
	//--- 셰이더 생성하고 코드 연결한 후 컴파일 하기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);  // vertex shader 생성
	glShaderSource(vertexShader, 1, &source, NULL);  // vertex shader 소스 지정
	glCompileShader(vertexShader); // vertex shader 컴파일 
	GLint result; // 여기서부터 1234
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	} // 여기까지 1234 에러체크 하는 부분이므로 꼭 넣을 것 
}
//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	std::string fragmentSource = filetobuf("fragment.glsl"); // filetobuf: 사용자 정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	const char* source = fragmentSource.c_str();
	//--- 셰이더 생성하고 코드 연결한 후 컴파일 하기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // fragment shader 생성
	glShaderSource(fragmentShader, 1, &source, NULL); // fragment shader 소스 시정
	glCompileShader(fragmentShader); // fragment shader 컴파일
	GLint result;  // 여기서부터 1234
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}  // 여기까지 1234 에러체크 하는 부분이므로 꼭 넣을 것 
}
//--- 버퍼 생성하고 데이터 받아오기
void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 생성하고 ID 얻기 -> vao를 만들어주는 함수 
	glBindVertexArray(vao); //--- VAO를 현재 사용할 대상으로 지정


	glGenBuffers(2, vbo); // 버퍼 id 생성 

	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 버퍼객체에 저장할 데이터 타입 지정, bind후에는 모든 버퍼는 바인딩 된 버퍼를 사용한다 
	//--- triShape 배열의 사이즈: sizeof(triShape) = 9 * float
	glBufferData(GL_ARRAY_BUFFER, sizeof(triShape), triShape, GL_STATIC_DRAW); // bind된 버퍼에 데이터를 넣는 것이다 \
		//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // bind된 버퍼에서 속성에 번호를 어떻게 설정햇는가
	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0); // 속성의 번호를 활성화 


	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	//--- colors 배열의 사이즈: sizeof(colors) = 9 *float
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);
}
//--- 세이더 프로그램 생성하기
GLuint make_shaderProgram()
{
	//-- shader Program
	shaderProgramID = glCreateProgram(); // shader program 생성
	glAttachShader(shaderProgramID, vertexShader); // 두 shader(vertex부분)을 program에 연결
	glAttachShader(shaderProgramID, fragmentShader); // 두 shader(fragment부분)을 program에 연결
	glLinkProgram(shaderProgramID); // 세이터 프로그램 링크
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader); // 링크가 끝났으므로 셰이더 객체 삭제 
	glDeleteShader(fragmentShader); // 링크가 끝났으므로 셰이더 객체 삭제
	//--- 링크 성공 여부 확인
	GLint result; // 여기서부터 1234
	GLchar errorLog[512];
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog
		(shaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return 0;
	} // 여기까지 1234 에러 체크하는 부분이므로 꼭 넣을 것 
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID); // shader program 사용
	return shaderProgramID;
}