
#include <iostream>
#include <GL/glew.h>
#include <stdio.h>  // PDF 41쪽: OBJ 읽기에 사용하는 헤더
#include <stdlib.h> // PDF 41쪽: malloc, exit
#include <string.h> // PDF 41쪽: strchr
#include <GL/glfw3.h>
#include <gl/glm/glm.hpp>                  // PDF 2쪽: GLM 헤더 포함
#include <gl/glm/ext.hpp>                  // PDF 2쪽: GLM 헤더 포함
#include <gl/glm/gtc/matrix_transform.hpp> // PDF 2쪽: GLM 변환 함수 헤더 포함
#include "shader.h"
#include <random>
GLuint vao, vbo_pos, vbo_color;
GLint modelLocation;
float xAngle = 30.0f;
float yAngle = 30.0f;

std::random_device dre;
std::mt19937 gen(dre());
std::uniform_real_distribution<float> pos(-0.6f, 0.6f);

// PDF 41쪽: 정점, 삼각형 면, 모델을 저장하는 구조체
#define MAX_LINE_LENGTH 200
typedef struct { //교수님
    float x, y, z;
} Vertex;
typedef struct { //교수님
    unsigned int v1, v2, v3;
} Face;
struct  Model { //교수님
    enum class AnimationMode {
        SINGLE_FACE, TWO_FACES, FOUR_FACES
    };
    AnimationMode mode = AnimationMode::SINGLE_FACE;

    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;

    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    float angleX = 0.0f; // x축으로 얼마나 회전되었나
    float angleY = 0.0f; // y축으로 얼마나 회전되었나
    float angleZ = 0.0f;
    float openAngleF = 0.0f;
    float left_face = 0.0f; // 왼쪽면이 얼마나 회전되었나
    float right_face = 0.0f;
    float edge1 = 0.0f; // 1번 모서리 얼마나 회전되었나
    float edge2 = 0.0f; // 2번 모서리 얼마나 회전되었나
    float edge4 = 0.0f; // 1번 모서리 얼마나 회전되었나
    float edge5 = 0.0f; // 2번 모서리 얼마나 회전되었나

    float rotateSpeedX = 0.0f; // x축으로 얼마나 빨리 화전할 것인가
    float rotateSpeedY = 0.0f;
    float rotateSpeedZ = 0.0f;
    float openAngleSpeedF = 0.0f; // 열리는 애니메이션 얼마나 빠르게
    float left_faceSpeed = 0.0f; // 왼쪽 면 돌리는 속도 얼마나 빠르게
    float right_faceSpeed = 0.0f;
    float edge1_speed = 0.0f; // 1번 모서리 얼마나 빠르게 회전할 것인가
    float edge2_speed = 0.0f; // 2번 모서리 얼마나 빠르게 회전할 것인가
    float edge4_speed = 0.0f; // 1번 모서리 얼마나 빠르게 회전할 것인가
    float edge5_speed = 0.0f; // 2번 모서리 얼마나 빠르게 회전할 것인가

    float scaleX = 1.0f; // 초기값 1배율
    float scaleY = 1.0f;

    float scaleSpeedX = 0.0f;
    float scaleSpeedY = 0.0f;
    void move(float dt) {
        dx = glm::clamp(dx, -0.3f, 0.3f);
        dy = glm::clamp(dy, -0.3f, 0.3f);
        dz = glm::clamp(dz, -0.3f, 0.3f);
    }
    void rotate(float dt) {
        angleX += rotateSpeedX * dt;
        angleY += rotateSpeedY * dt;
        angleZ += rotateSpeedZ * dt;
        openAngleF += openAngleSpeedF * dt;
        if (openAngleF >= 180.0) openAngleF = 180.0;
        else if (openAngleF <= -0) openAngleF = -0;
        left_face += left_faceSpeed * dt;
        right_face += right_faceSpeed * dt;
        edge1 += edge1_speed * dt;
        edge2 += edge2_speed * dt;
        edge2 = glm::clamp(edge2, 0.0f, 90.0f);
        edge4 += edge4_speed * dt;
        edge5 += edge5_speed * dt;
    }
    void scale(float dt) {
        scaleX += scaleSpeedX * dt;
        scaleY += scaleSpeedY * dt;
        scaleX = glm::clamp(scaleX , 0.0f, 1.0f);
        scaleY = glm::clamp(scaleY, 0.0f, 1.0f);
    }
    int twoFaceCommand = 1;
    int fourFaceCommand = 1;
} ;

Model cube{};
Model cone{};
Model initialCube{};
Model initialCone{};

// 좌표를 직접 적지 않고 cube.txt에서 읽은 데이터로 채운다.
GLfloat* cubePosition = nullptr;
GLfloat* cubeColor = nullptr;
GLsizei cubeVertexCount = 0;

GLfloat* conePosition = nullptr;
GLfloat* coneColor = nullptr;
GLsizei coneVertexCount = 0;
// PDF 40쪽 cube.obj의 면 순서: 뒤, 왼쪽, 위, 오른쪽, 아래, 앞
GLfloat faceColor[6][3] = {
    {1.0f, 0.0f, 0.0f}, // 뒷면: 빨강
    {0.0f, 0.0f, 1.0f}, // 왼쪽: 파랑 !
    {0.0f, 1.0f, 1.0f}, // 위: 하늘색 !
    {1.0f, 1.0f, 0.0f}, // 오른쪽: 노랑
    {1.0f, 0.0f, 1.0f}, // 아래: 자홍색
    {0.0f, 1.0f, 0.0f}  // 앞면: 초록 !
};

GLfloat axisPosition[6][3] = {
    {-0.9f, 0.0f, 0.0f}, {0.9f, 0.0f, 0.0f}, // x축선 그리기
    {0.0f,-0.9f, 0.0f}, {0.0f, 0.9f, 0.0f}, // y축선 그리기
    {0.0f, 0.0f,-0.9f}, {0.0f, 0.0f, 0.9f} // z축선 그리기 (모두 점 2개로)
};
GLfloat axisColor[6][3] = {
    {1.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, // x축의 색깔 
    {0.0f,0.7f,0.0f}, {0.0f,0.7f,0.0f}, // y축의 색깔
    {0.0f,0.0f,1.0f}, {0.0f,0.0f,1.0f}  // z축의 색깔
};
bool showCone{};
bool isH{}; // 은면제거 유무
bool isRotateX{}; // X축 회전 --> x축은 인사
bool isRotateY{}; // Y축 회전 --> y축은 코끼리코 
bool isRotateZ{}; // Z축 회전 --> Z축은 덤블링
bool isT{}; // 윗면의 가운데 축 (z축)을 중심으로 회전/멈추기
bool isF{}; // 앞면이 위축을 기준으로 회전하여 열기/닫기
bool isS{}; // 옆면이 중점을 기준으로 제자리에서 회전/멈추기
bool isB{}; // 뒷면이 제 자리에서 작아지면서 없어진다/다시 커지면서 원래 크기가 된다
bool is1{}, is2{}, is4{}, is5{};

int num1, num2 = 0; // num1은 1~4번 모서리 중에 하나를 고른다 
int num5 = 0; // num5 는 4면 애니메이션 때 2개의 옆면을 지정하는 값


static double lastTime = 0.0;

void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height); // 그림을 그릴 영역의 크기와 위치를 조정하는 함수
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);



void read_newline(char* str) // 교수님
{
    char* pos = strchr(str, '\n');
    if (pos != NULL) *pos = '\0';
}

void read_obj_file(const char* filename, Model* model) // 교수님 - OBJ파일 읽기 샘플
{
    FILE* file;
    fopen_s(&file, filename, "r"); // PDF 41쪽: 파일 열기
    if (!file) {
        perror("Error opening cube.txt");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];
    model->vertex_count = 0;
    model->face_count = 0;
    while (fgets(line, sizeof(line), file)) { // PDF 41쪽: 정점과 삼각형 개수 세기
        if (line[0] == 'v' && line[1] == ' ') model->vertex_count++;
        else if (line[0] == 'f' && line[1] == ' ') model->face_count++;
    }

    fseek(file, 0, SEEK_SET); // PDF 41쪽: 파일 처음으로 돌아가기
    model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex)); // PDF 41쪽
    model->faces = (Face*)malloc(model->face_count * sizeof(Face)); // PDF 41쪽
    if (!model->vertices || !model->faces) {
        fclose(file);
        exit(EXIT_FAILURE);
    }
    size_t vertex_index = 0, face_index = 0;
    while (fgets(line, sizeof(line), file)) { // PDF 41쪽: 한 줄씩 읽기
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ') {
            int result = sscanf_s(line + 2, "%f %f %f", // PDF 41쪽: 정점 좌표 읽기
                &model->vertices[vertex_index].x,
                &model->vertices[vertex_index].y,
                &model->vertices[vertex_index].z);
            if (result != 3) {
                fprintf(stderr, "Invalid vertex: %s\n", line);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            vertex_index++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int v1 = 0, v2 = 0, v3 = 0;
            // PDF 41쪽 sscanf_s를 PDF 40쪽의 1//2 형식에 맞춘다.
            // %*u는 법선 번호를 읽고 저장하지 않는다. 기존 test13에서 수정했던 서식이다.
            int result = sscanf_s(line + 2, "%u//%*u %u//%*u %u//%*u", &v1, &v2, &v3);
            if (result != 3 || v1 == 0 || v2 == 0 || v3 == 0 ||
                v1 > model->vertex_count || v2 > model->vertex_count || v3 > model->vertex_count) {
                fprintf(stderr, "Invalid face: %s\n", line);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            model->faces[face_index].v1 = v1 - 1; // PDF 41쪽: OBJ 번호 1부터 → 배열 번호 0부터
            model->faces[face_index].v2 = v2 - 1;
            model->faces[face_index].v3 = v3 - 1;
            face_index++;
        }
    }
    fclose(file); // PDF 41쪽: 파일 닫기
}
float Timer() {

    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    lastTime = now;

    return dt;
}
void Move(float dt) { // 0.016초마다 한번씩 실행중
    cube.move(dt);
    cone.move(dt);
}
void Rotate(float dt) {
    cube.rotate(dt);
    cone.rotate(dt);
}
void Scale(float dt) {
    cube.scale(dt);
    cone.scale(dt);
}
int main()
{
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Test 13 - Cube", nullptr, nullptr); // PDF 30쪽: 창 생성 예제
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    if (shaderProgramID == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    // 프로그램에서 uniform 변수의 위치를 가져온다, 리턴값 : uniform 변수 위치(-1찾지못함)
    modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform"); // 버텍스 세이더에서 모델링 변환 위치 가져오기
    // 은면제거 함수, 은면제거를 하면 관측자 시점에서 가까운 면이 보이고, 가까운 면에 그려진 면은 안보이게 그린다
    read_obj_file("cube.txt", &cube); // 텍스트 파일에서 OBJ 형식의 좌표와 면 읽기
    read_obj_file("cone.txt", &cone);
    InitBuffer();
    initialCube = cube;
    initialCone = cone;
    glfwSetFramebufferSizeCallback(window, Reshape); // “이 창의 그림을 출력하는 영역 크기가 바뀌면 Reshape()를 호출해라"
    int width, height; //현재 출력 영역의 가로·세로 크기를 저장할 정수 변수를 선언
    glfwGetFramebufferSize(window, &width, &height); // 창의 실제 그림 출력 영역 크기를 픽셀 단위로 가져옴.
    Reshape(window, width, height); // 프로그램 시작 시 출력 영역을 설정하기 위해 Reshape()를 직접 한 번 호출

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) { // PDF 30쪽: 렌더링 루프 예제
        float dt = Timer();
        Move(dt);
        Rotate(dt);
        Scale(dt);

        drawScene(); // PDF 30쪽: 렌더링 루프에서 drawScene 호출
        glfwSwapBuffers(window); // PDF 30쪽: 화면 출력 예제
        glfwPollEvents();         // PDF 30쪽: 이벤트 처리 예제

    }

    glDeleteBuffers(1, &vbo_pos);
    glDeleteBuffers(1, &vbo_color);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgramID);
    free(cube.vertices);
    free(cube.faces);
    free(cubePosition);
    free(cubeColor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void InitBuffer() // 한번만 호출
{
    cubeVertexCount = static_cast<GLsizei>(cube.face_count * 3);
    // PDF 41쪽의 malloc 방식으로, 그릴 정점 수만큼 배열을 만든다.
    cubePosition = (GLfloat*)malloc(cubeVertexCount * 3 * sizeof(GLfloat));
    cubeColor = (GLfloat*)malloc(cubeVertexCount * 3 * sizeof(GLfloat));
    if (!cubePosition || !cubeColor) exit(EXIT_FAILURE);
    // 파일의 면 번호로 정점을 찾아 삼각형마다 3개씩 순서대로 저장한다.
    for (size_t triangle = 0; triangle < cube.face_count; triangle++) { // 0 부터 11까지
        unsigned int indices[3] = { // indices[0] = 1, indices[1] = 7 indices[2] = 5 이거 12번 반복! 값은 달라짐!
            cube.faces[triangle].v1, cube.faces[triangle].v2, cube.faces[triangle].v3
        };
        for (int vertex = 0; vertex < 3; vertex++) {
            Vertex position = cube.vertices[indices[vertex]]; // position = 정점8개 좌표중 2번째(indices[0] = 1 ) 정점8개 좌표중 8번째(indices[0] = 1 ) 정점8개 좌표중 6번째(indices[0] = 1 ) 즉 3개 만들어짐
            size_t offset = (triangle * 3 + vertex) * 3;
            // 파일의 0~1 좌표를 -0.4~0.4로 바꾸어 원점 중심에 놓는다.
            cubePosition[offset] = (position.x - 0.5f) * 0.8f; // cubePosition[108]이 만들어짐
            cubePosition[offset + 1] = (position.y - 0.5f) * 0.8f; // x , y , z 각각 cubePosition으로 만들어지기 떄문임 즉 3 * 3 * 12
            cubePosition[offset + 2] = (position.z - 0.5f) * 0.8f;
            // cube.txt의 연속된 삼각형 2개가 정육면체의 같은 면이다.
            for (int component = 0; component < 3; component++)
                cubeColor[offset + component] = faceColor[triangle / 2][component];
        }
    }

    coneVertexCount = static_cast<GLsizei>(cone.face_count * 3);
    conePosition = (GLfloat*)malloc(coneVertexCount * 3 * sizeof(GLfloat));
    coneColor = (GLfloat*)malloc(coneVertexCount * 3 * sizeof(GLfloat));

    if (!conePosition || !coneColor) exit(EXIT_FAILURE);

    for (size_t triangle = 0; triangle < cone.face_count; triangle++) {
        unsigned int indices[3] = {
            cone.faces[triangle].v1,
            cone.faces[triangle].v2,
            cone.faces[triangle].v3
        };

        for (int vertex = 0; vertex < 3; vertex++) {
            Vertex position = cone.vertices[indices[vertex]];
            size_t offset = (triangle * 3 + vertex) * 3;

            conePosition[offset] = (position.x - 0.5f) * 0.8f;
            conePosition[offset + 1] = (position.y - 0.5f) * 0.8f;
            conePosition[offset + 2] = (position.z - 0.5f) * 0.8f;

            // 옆면 4개는 각각 다른 색, 밑면 삼각형 2개는 같은 색
            int face = triangle < 4 ? static_cast<int>(triangle) : 4;

            for (int component = 0; component < 3; component++) {
                coneColor[offset + component] = faceColor[face][component];
            }
        }
    }

    glGenVertexArrays(1, &vao); // 버텍스 array 생성
    glBindVertexArray(vao); //  VAO를 가진다는 의미
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // vbo_pos를 작업 대상으로 선택                             // 버퍼 오브젝트의 데이터를 생성 
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // cubePosition에 있는 내용을  vbo_pos 버퍼에 복사 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // “0번 속성은 vbo_pos에서 실수 3개씩 읽어라.”
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // “1번 속성은 vbo_color에서 실수 3개씩 읽어라.”
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
}

void drawScene()
{
    if (isH)glDisable(GL_DEPTH_TEST);
    else glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 배경색 설정
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 색상 버퍼와 깊이 버퍼 초기화
    glUseProgram(shaderProgramID); // 셰이더 프로그램 사용 
    glBindVertexArray(vao);         // VAO 바인딩 , VAO에는 위치속성 0번과, 색상속성 1번을 어떤 버퍼에서 읽을지 설정되어 있다.

    //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 축 그리기
    // PDF 3쪽: mat4/vec3 생성자와 단위 행렬 초기화, 25쪽: 변환 행렬 생성 예제
    glm::mat4 Rx = glm::mat4(1.0f); // 4x4 단위행렬, 행렬 초기화 --> x축 회전행렬 준비
    glm::mat4 Ry = glm::mat4(1.0f); // 4x4 단위행렬, 행렬 초기화 --> y축 회전행렬 준비
    // PDF 3쪽: rotate 함수, 5쪽: radians 함수, 12쪽: x/y/z축 회전, 25쪽: 회전 코드 예제
    Rx = glm::rotate(Rx, glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // x축을 회전축으로 지정함. 물체를 회전하겠다 
    Ry = glm::rotate(Ry, glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // y축을 회전축으로 지정함. 물체를 회전하겠다 
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f); // 3차원 좌표를 화면에 표시하기 위한 정사영 행렬을 만듭니다
    glm::mat4 modelTransform = projection * Rx * Ry; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정점 위치를 저장하는 버퍼선택, “지금부터 GL_ARRAY_BUFFER에 하는 작업은 vbo_pos에 적용해”
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_STATIC_DRAW); // axisPosition의 좌표 6개를 GPU의 vbo_pos에 저장합니다
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 6);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4  RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

    if (!showCone && cube.mode == Model::AnimationMode::SINGLE_FACE) {

        //------------------------------------------------------------------------------------------------------------------------------------------------------------------윗면 Z축 중심 회전
        glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
        glm::mat4 RotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, 0.0f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

        modelTransform = projection * Rx * Ry * RotateY * RotateX * toCenter * RotateZ * toOriginal; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 12, 6); // 정육면체 그리기
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------ 앞면 X축 중심 열기 닫기
        glm::mat4 toCenter2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, -0.4f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
        glm::mat4 RotateOpenF = glm::rotate(glm::mat4(1.0f), glm::radians(-cube.openAngleF), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 toOriginal2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.4f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

        modelTransform = projection * Rx * Ry * RotateY * RotateX * toOriginal2 * RotateOpenF * toCenter2; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 30, 6); // 정육면체 그리기
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------ 옆면 왼쪽 오른쪽 

        glm::mat4 toCenter3 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, -0.0f, -0.0f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
        glm::mat4 RotateLeftSide = glm::rotate(glm::mat4(1.0f), glm::radians(-cube.left_face), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 toOriginal3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, -0.0f, -0.0f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

        modelTransform = projection * Rx * Ry * RotateY * RotateX * toOriginal3 * RotateLeftSide * toCenter3; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 6, 6); // 왼쪽 옆면
        }

        glm::mat4 toCenter4 = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 0.0f, 0.0f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
        glm::mat4 RotateRightSide = glm::rotate(glm::mat4(1.0f), glm::radians(-cube.right_face), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 toOriginal4 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, 0.0f, 0.0f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

        modelTransform = projection * Rx * Ry * RotateY * RotateX * toOriginal4 * RotateRightSide * toCenter4; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 18, 6); // 오른쪽 옆면
        }

        //------------------------------------------------------------------------------------------------------------------------------------------------------------------ 뒷면 크기 조절하기
        glm::mat4 scaleBack = glm::scale(glm::mat4(1.0f), glm::vec3(cube.scaleX, cube.scaleY, 1.0f));

        modelTransform = projection * Rx * Ry * RotateY * RotateX * scaleBack; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 0, 6); // 뒷면 그리기
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------  나머지 다 그리기

        RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
        RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

        modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (!showCone) { // 큐브 선택이라면
            glDrawArrays(GL_TRIANGLES, 24, 6); // 정육면체 그리기

        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------  나머지 다 그리기

    }
    else if (!showCone && cube.mode == Model::AnimationMode::TWO_FACES) {
        float a, b = 0.0f;
        if (num1 == 1) {
            a = -0.4f, b = 0.4f;
        }
        else if (num1 == 2) {
            a = 0.4f, b = 0.4f;
        }
        else if (num1 == 3) {
            a = -0.4f, b = -0.4f;
        }
        else if (num1 == 4) {
            a = 0.4f, b = -0.4f;
        }
        if (cube.twoFaceCommand == 1) {


            glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(a, 0.0f, b)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
            glm::mat4 edge1 = glm::rotate(glm::mat4(1.0f), glm::radians(cube.edge1), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(-a, 0.0f, -b)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

            modelTransform = projection * Rx * Ry * RotateY * RotateX * toCenter * edge1 * toOriginal; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                if (num1 == 1) { glDrawArrays(GL_TRIANGLES, 6, 6);  glDrawArrays(GL_TRIANGLES, 30, 6); } // 정육면체 그리기
                else if (num1 == 2) { glDrawArrays(GL_TRIANGLES, 30, 6);   glDrawArrays(GL_TRIANGLES, 18, 6); }
                else if (num1 == 3) { glDrawArrays(GL_TRIANGLES, 0, 6); glDrawArrays(GL_TRIANGLES, 6, 6); }
                else if (num1 == 4) { glDrawArrays(GL_TRIANGLES, 18, 6); glDrawArrays(GL_TRIANGLES, 0, 6); }

            }

            RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) {
                if (num1 == 1) {        // 뒤, 위, 오른쪽, 아래
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 18, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                }
                else if (num1 == 2) {   // 뒤, 왼쪽, 위, 아래
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glDrawArrays(GL_TRIANGLES, 6, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                }
                else if (num1 == 3) {   // 위, 오른쪽, 아래, 앞
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 18, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                    glDrawArrays(GL_TRIANGLES, 30, 6);
                }
                else if (num1 == 4) {   // 왼쪽, 위, 아래, 앞
                    glDrawArrays(GL_TRIANGLES, 6, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                    glDrawArrays(GL_TRIANGLES, 30, 6);
                }
            }
        }
        //-----------------------------------------------
        // 
        // ------------------------------------------------------------------------------------------------------------------------------------
        else if (cube.twoFaceCommand == 2) {
            glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
            glm::mat4 edge2 = glm::rotate(glm::mat4(1.0f), glm::radians(cube.edge2), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

            modelTransform = projection * Rx * Ry * RotateY * RotateX * toCenter * edge2 * toOriginal; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                if (num1 == 1) { glDrawArrays(GL_TRIANGLES, 6, 6); } // 정육면체 그리기
                else if (num1 == 2) { glDrawArrays(GL_TRIANGLES, 30, 6); }
                else if (num1 == 3) { glDrawArrays(GL_TRIANGLES, 0, 6); }
                else if (num1 == 4) { glDrawArrays(GL_TRIANGLES, 18, 6); }

            }

            toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // 윗면을 y = -0.4만큼 내려서 윗면 중심을 원점에 맞춤
            edge2 = glm::rotate(glm::mat4(1.0f), glm::radians(-cube.edge2), glm::vec3(0.0f, 1.0f, 0.0f));
            toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // 회전한 윗면을 y = +0.4만큼 올려서 원래 위치로 돌려놓음

            modelTransform = projection * Rx * Ry * RotateY * RotateX * toCenter * edge2 * toOriginal; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                if (num1 == 1) { glDrawArrays(GL_TRIANGLES, 30, 6); } // 정육면체 그리기
                else if (num1 == 2) { glDrawArrays(GL_TRIANGLES, 18, 6); }
                else if (num1 == 3) { glDrawArrays(GL_TRIANGLES, 6, 6); }
                else if (num1 == 4) { glDrawArrays(GL_TRIANGLES, 0, 6); }

            }
            RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) {
                if (num1 == 1) {        // 뒤, 위, 오른쪽, 아래
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 18, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                }
                else if (num1 == 2) {   // 뒤, 왼쪽, 위, 아래
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glDrawArrays(GL_TRIANGLES, 6, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                }
                else if (num1 == 3) {   // 위, 오른쪽, 아래, 앞
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 18, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                    glDrawArrays(GL_TRIANGLES, 30, 6);
                }
                else if (num1 == 4) {   // 왼쪽, 위, 아래, 앞
                    glDrawArrays(GL_TRIANGLES, 6, 6);
                    glDrawArrays(GL_TRIANGLES, 12, 6);
                    glDrawArrays(GL_TRIANGLES, 24, 6);
                    glDrawArrays(GL_TRIANGLES, 30, 6);
                }
            }
        }

    }
    else if (!showCone && cube.mode == Model::AnimationMode::FOUR_FACES) {
        if (cube.fourFaceCommand == 1) {
            glm::mat4 edge4 = glm::rotate(glm::mat4(1.0f), glm::radians(cube.edge4), glm::vec3(0.0f, 1.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX * edge4; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                glDrawArrays(GL_TRIANGLES, 0, 6); // 정육면체 그리기 
                glDrawArrays(GL_TRIANGLES, 6, 6); // 정육면체 그리기
                glDrawArrays(GL_TRIANGLES, 18, 6); // 정육면체 그리기
                glDrawArrays(GL_TRIANGLES, 30, 6); // 정육면체 그리기
            }

            RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                glDrawArrays(GL_TRIANGLES, 12, 6); // 정육면체 그리기 
                glDrawArrays(GL_TRIANGLES, 24, 6); // 정육면체 그리기
            }

        }
        else if (cube.fourFaceCommand == 2) {
            glm::mat4 edge5 = glm::rotate(glm::mat4(1.0f), glm::radians(cube.edge5), glm::vec3(1.0f, 0.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX * edge5; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                glDrawArrays(GL_TRIANGLES, 12, 6); // 윗면
                glDrawArrays(GL_TRIANGLES, 24, 6); // 아래면 
                glDrawArrays(GL_TRIANGLES, 0, 6); // 정육면체 그리기
                glDrawArrays(GL_TRIANGLES, 30, 6); // 정육면체 그리기
            }

            RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cube.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

            modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
            if (!showCone) { // 큐브 선택이라면
                glDrawArrays(GL_TRIANGLES, 18, 6); // 정육면체 그리기 
                glDrawArrays(GL_TRIANGLES, 6, 6); // 정육면체 그리기
            }

        }
    }
    else if (showCone && cone.mode == Model::AnimationMode::SINGLE_FACE) {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ 피라미드 도형 그리기

        RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cone.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
        RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cone.angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTransform = projection * Rx * Ry * RotateY * RotateX; // 위 세 행렬을 하나로 합친다, y축 회전 → x축 회전 → 투영, 즉 모델링변환이다!!!!!
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]); // 만든 행렬을 셰이더의 modelTransform에 전달합니다.

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos); // 정육면체 정점 위치 버퍼 선택
        glBufferData(GL_ARRAY_BUFFER, coneVertexCount * 3 * sizeof(GLfloat), conePosition, GL_STATIC_DRAW); // 위치 버퍼를 cubePosition 데이터로 교체합니다.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color); // 정육면체 색깔 버퍼 선택 
        glBufferData(GL_ARRAY_BUFFER, coneVertexCount * 3 * sizeof(GLfloat), coneColor, GL_STATIC_DRAW); // 색상 버퍼를 cubeColor 데이터로 교체
        if (showCone) {
            glDrawArrays(GL_TRIANGLES, 0, coneVertexCount); // 원뿔 그리기

        }

        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    }
}

void Reshape(GLFWwindow* window, int width, int height) // 그림을 그릴 영역의 크기와 위치를 조정하는 함수
{
    // 창 크기가 바뀌어도 정육면체 비율을 유지하는 정사각형 출력 영역.
    int size = width < height ? width : height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        showCone = !showCone;
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        isH = !isH;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {

    }
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        isRotateY = false; isRotateZ = false;
        isRotateX = !isRotateX;
        if (isRotateX) {
            cube.rotateSpeedX = 60.0f;
            cone.rotateSpeedX = 60.0f;
        }
        else {
            cube.rotateSpeedX = -60.0f;
            cone.rotateSpeedX = -60.0f;

        }

    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        isRotateX = false; isRotateZ = false;
        isRotateY = !isRotateY;
        if (isRotateY) {
            cube.rotateSpeedY = 60.0f;
            cone.rotateSpeedY = 60.0f;
        }
        else {
            cube.rotateSpeedY = 0.0f;
            cone.rotateSpeedY = 0.0f;

        }
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        cube = initialCube;
        cone = initialCone;
        showCone = false;
        cube.mode = Model::AnimationMode::SINGLE_FACE;
        cone.mode = Model::AnimationMode::SINGLE_FACE;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        isRotateX = false; isRotateY = false;
        isRotateZ = !isRotateZ;
        if (isRotateZ) {
            cube.rotateSpeedZ = 60.0f;
            cone.rotateSpeedZ = 60.0f;
        }
        else {
            cube.rotateSpeedZ = 0.0f;
            cone.rotateSpeedZ = 0.0f;
        }
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        isRotateX = false; isRotateY = false;
        isF = !isF;
        if (isF) {
            cube.openAngleSpeedF = 60.0f;
            cone.openAngleSpeedF = 60.0f;
        }
        else {
            cube.openAngleSpeedF = -60.0f;
            cone.openAngleSpeedF = -60.0f;
        }
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        isRotateX = false; isRotateY = false;
        isS = !isS;
        if (isS) {
            cube.left_faceSpeed = 60.0f;
            cube.right_faceSpeed = 60.0f; // 여기서부터 cone의 스피드는 조정하지 않겠다 
        }
        else {
            cube.left_faceSpeed = 0.0f;
            cube.right_faceSpeed = 0.0f;
        }
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        isRotateX = false; isRotateY = false;
        isB = !isB;
        if (isB) {
            cube.scaleSpeedX = 0.5f;
            cube.scaleSpeedY = 0.5f;
        }
        else {
            cube.scaleSpeedX = -0.5f;
            cube.scaleSpeedY = -0.5f;
        }
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        cube = initialCube;
        cone = initialCone;
        showCone = false;
        cube.mode = Model::AnimationMode::TWO_FACES;
        is2 = false;
        is1 = !is1;
        cube.twoFaceCommand = 1;

        std::uniform_int_distribution uid{ 1,4 }; // 1은 왼쪽 2는 앞면 3은 오른쪽 4는 뒷면
        if (is1) {
            cube.edge1_speed = 60.0f;
        }
        else {
            cube.edge1_speed = 0.0f;
        }
        num1 = uid(dre);

    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        cube = initialCube;
        cone = initialCone;
        showCone = false;

        cube.mode = Model::AnimationMode::TWO_FACES;
        cube.twoFaceCommand = 2;

        is1 = false;
        is2 = true;

        std::uniform_int_distribution uid{ 1,4 }; // 1은 왼쪽 2는 앞면 3은 오른쪽 4는 뒷면
        num1 = uid(dre);

        cube.edge2 = 0.0f;
        cube.edge2_speed = 60.0f;

    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        cube = initialCube;
        cone = initialCone;
        showCone = false;

        cube.mode = Model::AnimationMode::FOUR_FACES;
        cube.fourFaceCommand = 1;

        is1 = true;
        is2 = false;

        cube.edge4 = 0.0f;
        cube.edge4_speed = 60.0f;
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        cube = initialCube;
        cone = initialCone;
        showCone = false;

        cube.mode = Model::AnimationMode::FOUR_FACES;
        cube.fourFaceCommand = 2;

        is1 = false;
        is2 = true;

        cube.edge5 = 0.0f;
        cube.edge5_speed = 60.0f;

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

    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

    }
}


