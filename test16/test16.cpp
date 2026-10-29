
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
typedef struct { //교수님q
    unsigned int v1, v2, v3;
} Face;
bool is6{}, is7{}, is8{}, is9{};
bool active6{}, active7{}, active8{}, active9{};
int num1, num2 = 0; // num1은 1~4번 모서리 중에 하나를 고른다 
int num5 = 0; // num5 는 4면 애니메이션 때 2개의 옆면을 지정하는 값

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
    float openAngle6 = 0.0f; // 열리는 각도
    float openAngle7[4]{ };
    float angle8{};
    float angle9{};

    int activeFace7 = 0;


    float rotateSpeedX = 0.0f; // x축으로 얼마나 빨리 화전할 것인가
    float rotateSpeedY = 0.0f;
    float openAngleSpeed6 = 0.0f; // 열리는 각도를 얼마나 빨리
    float openAngleSpeed7 = 0.0f;
    float AngleSpeed8 = 0.0f;
    float AngleSpeed9 = 0.0f;

    void move(float dt) {
        dx = glm::clamp(dx, -0.3f, 0.3f);
        dy = glm::clamp(dy, -0.3f, 0.3f);
        dz = glm::clamp(dz, -0.3f, 0.3f);
    }
    void rotate(float dt) {
        angleX += rotateSpeedX * dt;
        angleY += rotateSpeedY * dt;
        openAngle6 += openAngleSpeed6 * dt;   
        if (openAngle6 >= 233.33) openAngle6 = 233.33;
        else if (openAngle6 <= 0.0) openAngle6 = 0.0f;

        if (activeFace7 < 4 && openAngleSpeed7 != 0.0f) {
            openAngle7[activeFace7] += openAngleSpeed7 * dt;

            if (openAngle7[activeFace7] >= 90.0f) {
                openAngle7[activeFace7] = 90.0f;
                activeFace7++; // 다음 면으로 이동
            }
            else if (openAngle7[activeFace7] <= 0.0f) {
                openAngle7[activeFace7] = 0.0f;
                activeFace7++;
            }
        }
       /* if (is8) {
            angle8 += AngleSpeed8 * dt;
            if (angle8 >= 180.0) angle8 = 180.0;
            l

        }
        else {
            angle8 += AngleSpeed8 * dt;
            if (angle8 >= 360.0) angle8 = 0.0;

        }*/
        angle8 += AngleSpeed8 * dt;

        if (is8 && angle8 >= 180.0f) {
            angle8 = 180.0f;       // 교환된 위치 유지
            AngleSpeed8 = 0.0f;
        }
        else if (!is8 && angle8 >= 360.0f) {
            angle8 = 0.0f;         // 원래 위치로 복귀
            AngleSpeed8 = 0.0f;
        }
        angle9 += AngleSpeed9 * dt;
        if (angle9 >= 90.0) angle9 = 90.0;
        else if (angle9 <= 0.0) angle9 = 0.0f;


    }

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
bool showCone = true;
bool isH{}; // 은면제거 유무  
bool isRotateX{}; // X축 회전 --> x축은 인사
bool isRotateY{}; // Y축 회전 --> y축은 코끼리코 bool isRotateZ{}; // Z축 회전 --> Z축은 덤블링



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
    glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cone.angleX), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4  RotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cone.angleY), glm::vec3(0.0f, 1.0f, 0.0f));

    if (showCone) {
        
        // 버퍼 업로드는 한 번만
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, coneVertexCount * 3 * sizeof(GLfloat),
            conePosition, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, coneVertexCount * 3 * sizeof(GLfloat),
            coneColor, GL_STATIC_DRAW);

        // 순서: 왼쪽, 앞, 오른쪽, 뒤
        glm::vec3 centers[4] = {
            {-0.4f, -0.4f,  0.0f},
            { 0.0f, -0.4f,  0.4f},
            { 0.4f, -0.4f,  0.0f},
            { 0.0f, -0.4f, -0.4f}
        };

        // 모두 바깥쪽으로 열리도록 축 방향 지정
        glm::vec3 axes[4] = {
            {0.0f, 0.0f,  1.0f},  // Z축
            {1.0f, 0.0f,  0.0f}, // X축
            {0.0f, 0.0f, -1.0f}, // Z축
            {-1.0f, 0.0f, 0.0f} // X축
        };

    

        glm::mat4 base = projection * Rx * Ry * RotateY * RotateX;

        if (active6) {
            for (int i = 0; i < 4; i++) {
                glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), -centers[i]);
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(cone.openAngle6), axes[i]);
                glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), centers[i]);

                modelTransform = base * toOriginal * rotation * toCenter;
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

                glDrawArrays(GL_TRIANGLES, i * 3, 3);
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
                glDrawArrays(GL_TRIANGLES, 12, 6);
            }
        }
        else if (active7) {
            for (int i = 0; i < 4; i++) {
                glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), -centers[i]);
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(cone.openAngle7[i]),axes[i]);
                glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), centers[i]);

                modelTransform = base * toOriginal * rotation * toCenter;
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

                glDrawArrays(GL_TRIANGLES, i * 3, 3);
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
                glDrawArrays(GL_TRIANGLES, 12, 6);
            }
        }
        else if (active8){

            glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f));
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(cone.angle8), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, 0.0f));

            modelTransform = base * toOriginal * rotation * toCenter;
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawArrays(GL_TRIANGLES, 6, 3);

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
            //glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawArrays(GL_TRIANGLES, 3, 3);
            glDrawArrays(GL_TRIANGLES, 9, 3);
            glDrawArrays(GL_TRIANGLES, 12, 3);
            glDrawArrays(GL_TRIANGLES, 15, 3);


        }
        else if (active9) {
            for (int i = 0; i < 4; i++) {
                glm::mat4 toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, 0.0f));
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-cone.angle9),axes[i]);
                glm::mat4 toOriginal = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f));

                modelTransform = base * toOriginal * rotation * toCenter;
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

                glDrawArrays(GL_TRIANGLES, i * 3, 3);
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
                glDrawArrays(GL_TRIANGLES, 12, 6);

            }

        }
        else {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, coneVertexCount);
        }
        // 밑면은 회전하지 않음

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
        isRotateY = false; 
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
        isRotateX = false; 
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
        showCone = true;
        cube.mode = Model::AnimationMode::SINGLE_FACE;
        cone.mode = Model::AnimationMode::SINGLE_FACE;
    }
    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        is6 = !is6;
        active6 = true; active9 = false;
        active7 = false; active8 = false;
        if (is6) {
            cone.openAngleSpeed6 = 60.0f;
        }
        else {
            cone.openAngleSpeed6 = -60.0f;

        }

    }
    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        is7 = !is7;
        active6 = false; active8 = false;
        active7 = true;  active9 = false;
        cone.activeFace7 = 0;
        if (is7) {
            cone.openAngleSpeed7 = 60.0f;
        }
        else {
            cone.openAngleSpeed7 = -60.0f;

        }
    }
    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {

        active6 = false; active8 = true;
        active7 = false; active9 = false;
        is8 = !is8;
        cone.AngleSpeed8 = 60.0f;
    }
    if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
        active6 = false; active8 = false;
        active7 = false; active9 = true;
        is9 = !is9;
        if (is9) {
            cone.AngleSpeed9 = 60.0f;
        }
        else {
            cone.AngleSpeed9 = -60.0f;
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

        if (width <= 0 || height <= 0) return;

        // 마우스 좌표 → 사각형을 그리는 -1~1 좌표
        float mx = static_cast<float>(2.0 * x / width - 1.0);
        float my = static_cast<float>(1.0 - 2.0 * y / height);

    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

    }
}


