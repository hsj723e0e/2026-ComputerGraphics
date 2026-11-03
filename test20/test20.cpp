#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include <cmath>

GLuint vao, vbo_pos, vbo_color;
GLint modelLocation;

// PDF 41쪽: 정점, 삼각형 면, 모델을 저장하는 구조체
#define MAX_LINE_LENGTH 200
typedef struct { //교수님
    float x, y, z;
} Vertex;
typedef struct { //교수님
    unsigned int v1, v2, v3;
} Face;
typedef struct { //교수님
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
} Model;
Model cube{};

struct Body {
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 size = glm::vec3(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
};



Body floorBody;
Body beltBody[2];       // ㄱ자 벨트의 평면 2개
Body railBody[4];       // 노란 가장자리
Body legBody[8];        // 받침 다리
Body stripeBody[24];    // 움직이는 줄무늬
Body boxBody[5];        // 색상이 다른 택배 상자
Body numberBody[7];     // 숫자 1~5를 구성하는 막대

GLfloat* cubePosition = nullptr;
GLfloat* cubeColor = nullptr;
GLsizei cubeVertexCount = 0;

// 벨트는 평면: 삼각형 2개, 정점 6개
GLfloat planePosition[6][3] = {
    {-0.5f,0.0f,-0.5f}, {-0.5f,0.0f,0.5f}, {0.5f,0.0f,0.5f},
    {-0.5f,0.0f,-0.5f}, {0.5f,0.0f,0.5f}, {0.5f,0.0f,-0.5f}
};

// 순서: 위, 오른쪽 위, 오른쪽 아래, 아래, 왼쪽 아래, 왼쪽 위, 가운데
bool numberShape[5][7] = {
    {false,true, true, false,false,false,false}, // 1
    {true, true, false,true, true, false,true }, // 2
    {true, true, true, true, false,false,true }, // 3
    {false,true, true, false,false,true, true }, // 4
    {true, false,true, true, false,true, true }  // 5
};

// (x,z): 시작(-1.6,1.8) → 코너(-1.6,-1.6) → 끝(1.8,-1.6)
float boxDistance[5]{};
float stripeDistance[24]{};
const float firstLength = 3.4f;
const float totalLength = 6.8f;
const float beltHeight = 0.65f;
float beltSpeed = 0.6f;

// 키보드에서 연결할 상태값. 키보드 명령 자체는 구현하지 않는다.
bool isB = true;              // 기본은 이동 상태
bool isP = true;              // true: 원근 투영, false: 직각 투영
float cameraDistance = 7.5f;
float cameraOrbitAngle = 0.0f;
float cameraOrbitSpeed = 0.0f;
double lastTime = 0.0;

void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void resetScene() {
    isB = true;
    isP = true;
    beltSpeed = 0.6f;
    cameraDistance = 7.5f;
    cameraOrbitAngle = 0.0f;
    lastTime = glfwGetTime();

    floorBody = Body{};
    floorBody.position = glm::vec3(0.0f,-0.05f,0.0f);
    floorBody.size = glm::vec3(6.0f,0.1f,6.0f);
    floorBody.color = glm::vec3(0.27f,0.31f,0.27f);

    for (int i=0; i<2; ++i) {
        beltBody[i] = Body{};
        beltBody[i].color = glm::vec3(0.43f,0.45f,0.49f);
    }
    // 두 평면은 코너에서 만나며 서로 겹치지 않는다.
    beltBody[0].position = glm::vec3(-1.6f,beltHeight,0.55f);
    beltBody[0].size = glm::vec3(0.9f,1.0f,3.4f);
    beltBody[1].position = glm::vec3(0.1f,beltHeight,-1.6f);
    beltBody[1].size = glm::vec3(4.3f,1.0f,0.9f);

    for (int i=0; i<4; ++i) {
        railBody[i] = Body{};
        railBody[i].color = glm::vec3(0.85f,0.72f,0.10f);
    }
    railBody[0].position = glm::vec3(-2.08f,beltHeight,0.1f);
    railBody[0].size = glm::vec3(0.045f,0.07f,4.3f);
    railBody[1].position = glm::vec3(-1.12f,beltHeight,0.55f);
    railBody[1].size = glm::vec3(0.045f,0.07f,3.4f);
    railBody[2].position = glm::vec3(0.1f,beltHeight,-2.08f);
    railBody[2].size = glm::vec3(4.3f,0.07f,0.045f);
    railBody[3].position = glm::vec3(0.55f,beltHeight,-1.12f);
    railBody[3].size = glm::vec3(3.4f,0.07f,0.045f);

    for (int i=0; i<8; ++i) {
        legBody[i] = Body{};
        legBody[i].size = glm::vec3(0.07f,beltHeight,0.07f);
        legBody[i].color = glm::vec3(0.12f,0.14f,0.13f);
    }
    legBody[0].position = glm::vec3(-1.94f,0.325f,1.85f);
    legBody[1].position = glm::vec3(-1.26f,0.325f,1.85f);
    legBody[2].position = glm::vec3(-1.94f,0.325f,-1.6f);
    legBody[3].position = glm::vec3(-1.26f,0.325f,-1.6f);
    legBody[4].position = glm::vec3(0.0f,0.325f,-1.94f);
    legBody[5].position = glm::vec3(0.0f,0.325f,-1.26f);
    legBody[6].position = glm::vec3(1.85f,0.325f,-1.94f);
    legBody[7].position = glm::vec3(1.85f,0.325f,-1.26f);

    for (int i=0; i<24; ++i) {
        stripeBody[i] = Body{};
        stripeBody[i].size = glm::vec3(0.82f,0.012f,0.035f);
        stripeBody[i].color = glm::vec3(0.14f,0.16f,0.18f);
        stripeDistance[i] = totalLength * i / 24.0f;
        if (stripeDistance[i] < firstLength) {
            stripeBody[i].position = glm::vec3(-1.6f,beltHeight+0.006f,1.8f-stripeDistance[i]);
        }
        else {
            stripeBody[i].position = glm::vec3(-1.6f+stripeDistance[i]-firstLength,beltHeight+0.006f,-1.6f);
            stripeBody[i].rotation = glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(0.0f,1.0f,0.0f));
        }
    }
    for (int i=0; i<5; ++i) {
        boxBody[i] = Body{};
        boxBody[i].size = glm::vec3(0.38f,0.46f,0.50f);
        boxDistance[i] = 0.35f + totalLength * i / 5.0f;
        if (boxDistance[i] < firstLength) {
            boxBody[i].position = glm::vec3(-1.6f,beltHeight+0.012f+0.23f,1.8f-boxDistance[i]);
        }
        else {
            boxBody[i].position = glm::vec3(-1.6f+boxDistance[i]-firstLength,beltHeight+0.012f+0.23f,-1.6f);
            boxBody[i].rotation = glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(0.0f,1.0f,0.0f));
        }
    }
    boxBody[0].color = glm::vec3(0.85f,0.23f,0.20f);
    boxBody[1].color = glm::vec3(0.20f,0.43f,0.85f);
    boxBody[2].color = glm::vec3(0.22f,0.72f,0.28f);
    boxBody[3].color = glm::vec3(0.92f,0.52f,0.12f);
    boxBody[4].color = glm::vec3(0.88f,0.78f,0.18f);

    // 상자 윗면에 올릴 숫자 막대. 위치는 상자 중심 기준이다.
    for (int i=0; i<7; ++i) {
        numberBody[i] = Body{};
        numberBody[i].color = glm::vec3(0.03f);
    }
    numberBody[0].position = glm::vec3(0.0f,0.0f,-0.12f);
    numberBody[0].size = glm::vec3(0.16f,0.006f,0.022f);
    numberBody[1].position = glm::vec3(0.08f,0.0f,-0.06f);
    numberBody[1].size = glm::vec3(0.022f,0.006f,0.10f);
    numberBody[2].position = glm::vec3(0.08f,0.0f,0.06f);
    numberBody[2].size = glm::vec3(0.022f,0.006f,0.10f);
    numberBody[3].position = glm::vec3(0.0f,0.0f,0.12f);
    numberBody[3].size = glm::vec3(0.16f,0.006f,0.022f);
    numberBody[4].position = glm::vec3(-0.08f,0.0f,0.06f);
    numberBody[4].size = glm::vec3(0.022f,0.006f,0.10f);
    numberBody[5].position = glm::vec3(-0.08f,0.0f,-0.06f);
    numberBody[5].size = glm::vec3(0.022f,0.006f,0.10f);
    numberBody[6].position = glm::vec3(0.0f);
    numberBody[6].size = glm::vec3(0.16f,0.006f,0.022f);
}

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
        perror(filename);
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
            // PDF 예제의 일반 정점 번호 형식도 읽을 수 있게 한다.
            if (result != 3) {
                result = sscanf_s(line + 2, "%u %u %u", &v1, &v2, &v3);
            }
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

void Move(float dt) {
    if (!isB) return;

    // 줄무늬와 상자를 같은 속도로 이동시킨다. 끝에 도달하면 시작으로 돌아간다.
    for (int i=0; i<24; ++i) {
        stripeDistance[i] += beltSpeed * dt;
        stripeDistance[i] = std::fmod(stripeDistance[i],totalLength);
        stripeBody[i].rotation = glm::mat4(1.0f);
        if (stripeDistance[i] < firstLength) {
            stripeBody[i].position = glm::vec3(-1.6f,beltHeight+0.006f,1.8f-stripeDistance[i]);
        }
        else {
            stripeBody[i].position = glm::vec3(-1.6f+stripeDistance[i]-firstLength,beltHeight+0.006f,-1.6f);
            stripeBody[i].rotation = glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(0.0f,1.0f,0.0f));
        }
    }
    for (int i=0; i<5; ++i) {
        boxDistance[i] += beltSpeed * dt;
        boxDistance[i] = std::fmod(boxDistance[i],totalLength);
        boxBody[i].rotation = glm::mat4(1.0f);
        if (boxDistance[i] < firstLength) {
            boxBody[i].position = glm::vec3(-1.6f,beltHeight+0.012f+0.23f,1.8f-boxDistance[i]);
        }
        else {
            boxBody[i].position = glm::vec3(-1.6f+boxDistance[i]-firstLength,beltHeight+0.012f+0.23f,-1.6f);
            boxBody[i].rotation = glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(0.0f,1.0f,0.0f));
        }
    }
    cameraOrbitAngle += cameraOrbitSpeed * dt;
}

float Timer() {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    lastTime = now;
    return dt;
}

int main()
{
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 800, "Test 20 - Conveyor Belt", nullptr, nullptr);
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
    glfwSwapInterval(1);

    // 셰이더 파일을 읽고 프로그램을 만든다.
    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();
    if (shaderProgramID == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
    read_obj_file("cube.txt", &cube);
    glEnable(GL_DEPTH_TEST);
    InitBuffer();
    resetScene();
    glfwSetFramebufferSizeCallback(window, Reshape);
    glfwSetKeyCallback(window, keyCallback);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    Reshape(window, width, height);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float dt = Timer();
        Move(dt);
        drawScene();
        glfwSwapBuffers(window);
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

void InitBuffer()
{
    cubeVertexCount = static_cast<GLsizei>(cube.face_count * 3);

    cubePosition = (GLfloat*)malloc(
        cubeVertexCount * 3 * sizeof(GLfloat));

    cubeColor = (GLfloat*)malloc(
        cubeVertexCount * 3 * sizeof(GLfloat));

    if (!cubePosition || !cubeColor) {
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < cube.face_count; ++i) {
        unsigned int indices[3] = {
            cube.faces[i].v1,
            cube.faces[i].v2,
            cube.faces[i].v3
        };

        for (int j = 0; j < 3; ++j) {
            Vertex vertex = cube.vertices[indices[j]];
            size_t offset = (i * 3 + j) * 3;

            cubePosition[offset] = (vertex.x - 0.5f);
            cubePosition[offset + 1] = (vertex.y - 0.5f);
            cubePosition[offset + 2] = (vertex.z - 0.5f);

            // 파란색
            cubeColor[offset] = 0.2f;
            cubeColor[offset + 1] = 0.5f;
            cubeColor[offset + 2] = 1.0f;
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);

    // 정점 위치: 셰이더의 location 0
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // 정점 색상: 셰이더의 location 1
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void drawScene() {
    glClearColor(0.55f,0.73f,0.88f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(vao);

    glm::mat4 projection;
    if (isP) {
        projection = glm::perspective(glm::radians(45.0f),1.0f,0.1f,100.0f);
    }
    else {
        float range = cameraDistance * 0.41421356f;
        projection = glm::ortho(-range,range,-range,range,0.1f,100.0f);
    }
    // 바닥은 기울이지 않고 카메라를 높은 대각선에 놓아 쿼터뷰를 만든다.
    glm::vec3 cameraTarget(0.0f,0.35f,0.0f);
    glm::vec3 cameraOffset = glm::normalize(glm::vec3(1.1f,1.25f,1.6f)) * cameraDistance;
    glm::mat4 cameraOrbit = glm::rotate(glm::mat4(1.0f),glm::radians(cameraOrbitAngle),glm::vec3(0.0f,1.0f,0.0f));
    cameraOffset = glm::vec3(cameraOrbit * glm::vec4(cameraOffset,0.0f));
    glm::vec3 cameraPos = cameraTarget + cameraOffset;
    glm::vec3 cameraUp(0.0f,1.0f,0.0f);
    glm::mat4 view = glm::lookAt(cameraPos,cameraTarget,cameraUp);
    glm::mat4 base = projection * view;

    // 바닥q
    {
        const Body& f = floorBody;
        glm::mat4 T = glm::translate(glm::mat4(1.0f), f.position);
        glm::mat4 R = f.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), f.size);
        glm::mat4 fTransform = base * T * R * S;
        
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &fTransform[0][0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            cubeVertexCount * 3 * sizeof(GLfloat),
            cubePosition, GL_DYNAMIC_DRAW);
        
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, f.color.r, f.color.g, f.color.b);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
        glEnableVertexAttribArray(1);
    }

    // 컨베이어 벨트: 평면 2개
    {
        for (int i = 0; i < 2; ++i) {
            const Body& b = beltBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), b.position);
            glm::mat4 R = b.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), b.size);
            glm::mat4 bTransform = base * T * R * S;
            
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &bTransform[0][0]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER, sizeof(planePosition), planePosition, GL_DYNAMIC_DRAW);
            
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, b.color.r, b.color.g, b.color.b);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnableVertexAttribArray(1);
        }
    }

    // 노란 가장자리
    {
        for (int i = 0; i < 4; ++i) {
            const Body& r = railBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), r.position);
            glm::mat4 R = r.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), r.size);
            glm::mat4 rTransform = base * T * R * S;
            
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &rTransform[0][0]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);
            
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, r.color.r, r.color.g, r.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnableVertexAttribArray(1);
        }
    }

    // 받침 다리
    {
        for (int i = 0; i < 8; ++i) {
            const Body& l = legBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), l.position);
            glm::mat4 R = l.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), l.size);
            glm::mat4 lTransform = base * T * R * S;
            
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &lTransform[0][0]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);
            
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, l.color.r, l.color.g, l.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnableVertexAttribArray(1);
        }
    }

    // 움직이는 줄무늬
    {
        for (int i = 0; i < 24; ++i) {
            const Body& s = stripeBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), s.position);
            glm::mat4 R = s.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), s.size);
            glm::mat4 sTransform = base * T * R * S;
            
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &sTransform[0][0]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);
            
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, s.color.r, s.color.g, s.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnableVertexAttribArray(1);
        }
    }

    // 택배 상자
    {
        for (int i = 0; i < 5; ++i) {
            const Body& b = boxBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), b.position);
            glm::mat4 R = b.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), b.size);
            glm::mat4 bTransform = base * T * R * S;
            
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &bTransform[0][0]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);
            
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, b.color.r, b.color.g, b.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnableVertexAttribArray(1);
        }
    }

    // 상자 윗면의 번호. 상자의 이동과 회전을 번호에도 적용한다.
    {
        for (int i = 0; i < 5; ++i) {
            const Body& b = boxBody[i];
            glm::mat4 boxT = glm::translate(glm::mat4(1.0f), b.position);
            glm::mat4 boxR = b.rotation;
            for (int j = 0; j < 7; ++j) {
                if (!numberShape[i][j]) continue;
                const Body& n = numberBody[j];
                glm::vec3 numberPosition = n.position + glm::vec3(0.0f,b.size.y*0.5f+0.005f,0.0f);
                glm::mat4 T = glm::translate(glm::mat4(1.0f), numberPosition);
                glm::mat4 R = n.rotation;
                glm::mat4 S = glm::scale(glm::mat4(1.0f), n.size);
                glm::mat4 numberTransform = base * boxT * boxR * T * R * S;

                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &numberTransform[0][0]);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
                glBufferData(GL_ARRAY_BUFFER,
                    cubeVertexCount * 3 * sizeof(GLfloat),
                    cubePosition, GL_DYNAMIC_DRAW);
                glDisableVertexAttribArray(1);
                glVertexAttrib3f(1,n.color.r,n.color.g,n.color.b);
                glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
                glEnableVertexAttribArray(1);
            }
        }
    }
}

void Reshape(GLFWwindow*, int width, int height)
{
    // 창 크기가 바뀌어도 좌표계의 비율을 유지한다.
    int size = width < height ? width : height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    // 연습용: 키보드 명령은 여기에서 직접 구현한다.
    // b: isB, +/-: cameraDistance, p/P: isP
    // 왼쪽/오른쪽: cameraOrbitAngle, r: resetScene(), q: 종료
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        isB = !isB;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        cameraDistance -= 0.2;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        cameraDistance += 0.2;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        isP = !isP;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        cameraOrbitSpeed = -30.0f;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        cameraOrbitSpeed = 30.0f;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        isB = true;   
        isP = true;
        cameraDistance = 7.5f;
        cameraOrbitAngle = 0.0f;
        cameraOrbitSpeed = 0.0f;
    }
}
