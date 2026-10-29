#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
//--- 필요한 헤더파일 선언
#include <GL/glew.h>
#include <GL/glfw3.h>
using namespace std;
//--- 기존 사용자 정의 함수
void make_vertexShaders(); // 버텍스 세이더 객체 만들기
void make_fragmentShaders(); // 프래그먼트 세이더 객체 만들기
GLuint make_shaderProgram();  // 세이터 프로그램 생성하기 
void InitBuffer();
GLvoid drawScene(); // 출력함수 
GLvoid Reshape(int w, int h);
//--- 기존 변수 선언
GLint width, height;
GLuint shaderProgramID; // 쉐이더 프로그램 만들기
GLuint vertexShader;// 얘는 버텍스 세이더 객체
GLuint fragmentShader; // 얘는 프래그먼트 세이터 객체 
GLuint vao, vbo_pos, vbo_color; // 버퍼 생성 
//--- 개인 변수 선언
std::random_device dre;
std::mt19937 gen(dre());
std::uniform_real_distribution<float> pos(-0.8f, 0.8f);
std::uniform_real_distribution<float> color(0.0f, 1.0f);
const int MAXNUM = 50;
enum ShapeType {
	POINT, LINE, TRIANGLE, RECTANGLE
};
int selectedIndex = -1;
double prevMouseX = 0.0;
double prevMouseY = 0.0;
//--- 개인 함수 선언
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window,int button, int action, int mods);
//--- 구조체 선언
struct Shape {
	ShapeType type;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float x = 0.0f, y = 0.0f;
	bool isshow = true;
	int mode = 0;

	Shape(ShapeType type, float inx, float iny, float inred, float ingreen, float inblue) : type{ type }, r { inred }, g{ ingreen }, b{ inblue }, x{ inx }, y{ iny }
	{}

	Shape() {}

	void move(float dx, float dy) {
		x += dx;
		y += dy;
		if (x >= 1.0f) x -= 2.0f;
		else if (x < -1.0f) x += 2.0f;
		if (y >= 1.0f) y -= 2.0f;
		else if (y < -1.0f) y += 2.0f;
	}
};
vector<Shape> shapes;

 GLfloat triShape[3][3] = {
	{-0.5f, -0.5f, 0.0f},
	{ 0.5f, -0.5f, 0.0f},
	{ 0.0f,  0.5f, 0.0f}
};

 GLfloat colors[3][3] = {
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
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
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
	// 
	//--- 삼각형 그리기
	for (const Shape& shape : shapes)
	{
		if (!shape.isshow) continue;

		if (shape.type == POINT) {
			const float size = 0.1f;

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
		else if (shape.type == TRIANGLE) {
			const float size = 0.1f;

			GLfloat position[3][3] = {
			   {shape.x - size, shape.y - size, 0.0f},
			   {shape.x + size, shape.y - size, 0.0f},
			   {shape.x,        shape.y + size, 0.0f}
			};

			GLfloat color[3][3] = {
				{shape.r, shape.g, shape.b},
				{shape.r, shape.g, shape.b},
				{shape.r, shape.g, shape.b}
			};

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, sizeof(position),
				position, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(color),
				color, GL_DYNAMIC_DRAW);

			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else if (shape.type == RECTANGLE) {
			const float size = 0.1f;

			GLfloat position[6][3] = {
				// 첫 번째 삼각형
				{shape.x - size, shape.y - size, 0.0f}, // 왼쪽 아래
				{shape.x + size, shape.y - size, 0.0f}, // 오른쪽 아래
				{shape.x + size, shape.y + size, 0.0f}, // 오른쪽 위

				// 두 번째 삼각형
				{shape.x - size, shape.y - size, 0.0f}, // 왼쪽 아래
				{shape.x + size, shape.y + size, 0.0f}, // 오른쪽 위
				{shape.x - size, shape.y + size, 0.0f}  // 왼쪽 위		
			};

			GLfloat color[6][3];
			for (int i = 0; i < 6; i++) {
				color[i][0] = shape.r;
				color[i][1] = shape.g;
				color[i][2] = shape.b;
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); 
			glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else if (shape.type == LINE) {
			const float size = 0.1f;

			GLfloat position[2][3] = {
				// 첫 번째 삼각형
				{shape.x - size, shape.y, 0.0f}, // 왼쪽
				{shape.x + size, shape.y, 0.0f}, // 오른쪽 
			};

			GLfloat color[2][3];
			for (int i = 0; i < 2; i++) {
				color[i][0] = shape.r;
				color[i][1] = shape.g;
				color[i][2] = shape.b;
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
			glDrawArrays(GL_LINES, 0, 2);
		}
	}
	// 선택된 도형 표시
	if (selectedIndex >= 0 &&
		selectedIndex < static_cast<int>(shapes.size()))
	{
		const Shape& shape = shapes[selectedIndex];

		if (shape.isshow)
		{
			const float size = 0.1f;

			GLfloat position[4][3] = {};
			GLfloat color[4][3] = {}; // 모두 0 → 검정색
			int count = 4;

			if (shape.type == TRIANGLE)
			{
				position[0][0] = shape.x - size;
				position[0][1] = shape.y - size;

				position[1][0] = shape.x + size;
				position[1][1] = shape.y - size;

				position[2][0] = shape.x;
				position[2][1] = shape.y + size;

				count = 3;
			}
			else
			{
				float halfWidth = size;
				float halfHeight = size;

				// 선과 점은 주변에 작은 선택 상자 표시
				if (shape.type == LINE)
				{
					halfHeight = 0.02f;
				}
				else if (shape.type == POINT)
				{
					halfWidth = 0.02f;
					halfHeight = 0.02f;
				}

				// 외곽을 따라 순서대로 배치
				position[0][0] = shape.x - halfWidth;
				position[0][1] = shape.y - halfHeight;

				position[1][0] = shape.x + halfWidth;
				position[1][1] = shape.y - halfHeight;

				position[2][0] = shape.x + halfWidth;
				position[2][1] = shape.y + halfHeight;

				position[3][0] = shape.x - halfWidth;
				position[3][1] = shape.y + halfHeight;
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, sizeof(position),
				position, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(color),
				color, GL_DYNAMIC_DRAW);

			glDrawArrays(GL_LINE_LOOP, 0, count);
		}
	}
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


	glGenBuffers(1, &vbo_pos); // 버퍼 id 생성 
	glGenBuffers(1, &vbo_color); // 버퍼 id 생성 

	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 버퍼객체에 저장할 데이터 타입 지정, bind후에는 모든 버퍼는 바인딩 된 버퍼를 사용한다 
	//--- triShape 배열의 사이즈: sizeof(triShape) = 9 * float
	glBufferData(GL_ARRAY_BUFFER, sizeof(triShape), triShape, GL_STATIC_DRAW); // bind된 버퍼에 데이터를 넣는 것이다 \
	//--- 좌표값을 attribute 인덱스 0번에 명시한다: 버텍스 당 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // bind된 버퍼에서 속성에 번호를 어떻게 설정햇는가
	//--- attribute 인덱스 0번을 사용가능하게 함
	glEnableVertexAttribArray(0); // 속성의 번호를 활성화 


	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
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
void Move(char move) {
	if (selectedIndex < 0 || selectedIndex >= static_cast<int>(shapes.size()))return;
	Shape& shape = shapes[selectedIndex];

	float dx = 0.05f, dy = 0.05f;
	if(move == 'w') shape.move(0.0f, dy);
	else if(move == 'a') shape.move(-dx, 0.0f);
	else if (move == 's') shape.move(0.0f, -dy);
	else if (move == 'd') shape.move(dx, 0.0f);
	else if (move == 'i') shape.move(-dx, dy);
	else if (move == 'j') shape.move(dx, dy);
	else if (move == 'k') shape.move(-dx, -dy);
	else if (move == 'l') shape.move(dx, -dy);

}
void Move_all(int move) {
	float dx = 0.05f, dy = 0.05f;

	for (Shape& shape : shapes) {
		if (move == 1) shape.move(-dx, 0.0f);
		else if (move == 2) shape.move(dx, 0.0f);
		else if (move == 3) shape.move(0.0f, dy);
		else if (move == 4) shape.move(0.0f, -dy);

	}
}
void DeleteShape() {
	shapes.clear();
	selectedIndex = -1;
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		float x = pos(dre); float y = pos(dre);
		float r = color(dre); float g = color(dre); float b = color(dre);
		ShapeType type = POINT;
		shapes.push_back(Shape(POINT, x, y, r, g, b));
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		float x = pos(dre); float y = pos(dre);
		float r = color(dre); float g = color(dre); float b = color(dre); 
		ShapeType type = TRIANGLE;
		shapes.push_back(Shape(TRIANGLE,x,y,r,g,b));
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		float x = pos(dre); float y = pos(dre);
		float r = color(dre); float g = color(dre); float b = color(dre);
		ShapeType type = RECTANGLE;
		shapes.push_back(Shape(RECTANGLE, x, y, r, g, b));
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		float x = pos(dre); float y = pos(dre);
		float r = color(dre); float g = color(dre); float b = color(dre);
		ShapeType type = LINE;
		shapes.push_back(Shape(LINE, x, y, r, g, b));
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('w');
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('a');
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('s');
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('d');
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('i');
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('j');
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('k');
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		if (shapes.size() >= MAXNUM) return;
		Move('l');
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		Move_all(1);
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		Move_all(2);
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		Move_all(3);
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		Move_all(4);
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		DeleteShape();
	}
}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

		selectedIndex = -1;
		// 나중에 그린 사각형부터 검사

		for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i)
		{
			const Shape& shape = shapes[i];
			if (!shape.isshow) continue;

			const float size = 0.1f;
			float dx = mx - shape.x;
			float dy = my - shape.y;
			bool hit = false;

			if (shape.type == RECTANGLE)
			{
				hit = (dx >= -size && dx <= size &&
					dy >= -size && dy <= size);
			}
			else if (shape.type == TRIANGLE)
			{
				// 아래쪽은 넓고 위로 갈수록 좁아지는 현재 삼각형
				if (dy >= -size && dy <= size)
				{
					float halfWidth = (size - dy) / 2.0f;
					hit = (dx >= -halfWidth && dx <= halfWidth);
				}
			}
			else if (shape.type == LINE)
			{
				// 가로선 주변에 클릭 여유를 줌
				const float tolerance = 0.02f;

				hit = (dx >= -size && dx <= size &&
					dy >= -tolerance && dy <= tolerance);
			}
			else if (shape.type == POINT)
			{
				// 작은 점 주변에 클릭 여유를 줌
				const float tolerance = 0.02f;

				hit = (dx >= -tolerance && dx <= tolerance &&
					dy >= -tolerance && dy <= tolerance);
			}

			if (hit)
			{
				selectedIndex = i;
				prevMouseX = mx;
				prevMouseY = my;
				return;
			}
		}

	}
}
