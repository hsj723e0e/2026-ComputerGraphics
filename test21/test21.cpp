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

#define MAX_LINE_LENGTH 200
typedef struct { // 교수님
    float x, y, z;
} Vertex;
typedef struct { // 교수님
    unsigned int v1, v2, v3;
} Face;
typedef struct { // 교수님
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
} Model;
Model cube{}, sphere{};

struct Body {
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 size = glm::vec3(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
};

Body wallBody[4];      // 왼쪽, 오른쪽, 위쪽, 뒤쪽
Body floorBody[2];     // 아래쪽 평면을 반으로 나누어 열 수 있게 한다.
Body boxBody[3];       // 바닥의 육면체 3개
Body ballBody[5];      // 공은 최대 5개

glm::vec3 ballSpeed[5];
float boxSpeed[3]{};
int ballCount = 1;     // 실행하면 공 1개가 튕긴다. B키로 최대 5개까지 추가한다.

const float roomHalfWidth = 1.5f;
const float roomHalfHeight = 1.5f;
const float roomHalfDepth = 1.5f;

// 조작: Z/Shift+Z = 장면 Z 이동, Y/Shift+Y = 장면 Y축 회전,
// B = 공 추가(최대 5개), O = 바닥 열기/닫기, C = 초기화, Q/Esc = 종료.
// 마우스 왼쪽 버튼을 누르고 좌우로 드래그하면 방이 Z축으로 기울어진다.
float sceneZ = 0.0f;
float sceneAngleY = 0.0f;
bool isFloorOpen = false; // O키로 바닥 열림 상태를 전환한다.
float floorOpenAngle = 0.0f;

// 마우스로 방을 Z축에 대해 기울인다.
float roomAngleZ = 0.0f;
bool isDragging = false;
double lastMouseX = 0.0;
double lastTime = 0.0;

GLfloat* cubePosition = nullptr;
GLfloat* cubeColor = nullptr;
GLsizei cubeVertexCount = 0;
GLfloat* spherePosition = nullptr;
GLfloat* sphereColor = nullptr;
GLsizei sphereVertexCount = 0;

// XY 평면. 반시계 방향으로 정점을 연결하므로 앞면은 +Z를 바라본다.
// 각 벽을 회전시켜 앞면이 방 안쪽을 바라보게 한다.
GLfloat planePosition[6][3] = {
    {-0.5f,-0.5f,0.0f}, {0.5f,-0.5f,0.0f}, {0.5f,0.5f,0.0f},
    {-0.5f,-0.5f,0.0f}, {0.5f,0.5f,0.0f}, {-0.5f,0.5f,0.0f}
};

void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void resetScene() {
    sceneZ = 0.0f;
    sceneAngleY = 0.0f;
    roomAngleZ = 0.0f;
    isFloorOpen = false;
    floorOpenAngle = 0.0f;
    isDragging = false;
    lastMouseX = 0.0;
    lastTime = glfwGetTime();
    ballCount = 1;

    for (int i = 0; i < 4; ++i) {
        wallBody[i] = Body{};
        wallBody[i].size = glm::vec3(3.0f, 3.0f, 1.0f);
    }
    wallBody[0].position = glm::vec3(-roomHalfWidth, 0.0f, 0.0f);
    wallBody[0].rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    wallBody[0].color = glm::vec3(0.36f, 0.39f, 0.44f);

    wallBody[1].position = glm::vec3(roomHalfWidth, 0.0f, 0.0f);
    wallBody[1].rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    wallBody[1].color = glm::vec3(0.46f, 0.49f, 0.54f);

    wallBody[2].position = glm::vec3(0.0f, roomHalfHeight, 0.0f);
    wallBody[2].rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    wallBody[2].color = glm::vec3(0.29f, 0.32f, 0.37f);

    wallBody[3].position = glm::vec3(0.0f, 0.0f, -roomHalfDepth);
    wallBody[3].color = glm::vec3(0.63f, 0.66f, 0.70f);

    for (int i = 0; i < 2; ++i) {
        floorBody[i] = Body{};
        floorBody[i].position = glm::vec3(0.0f, -roomHalfHeight, 0.0f);
        floorBody[i].rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        floorBody[i].size = glm::vec3(roomHalfWidth, roomHalfDepth * 2.0f, 1.0f);
        floorBody[i].color = glm::vec3(0.52f, 0.55f, 0.59f);
    }
    floorBody[0].position.x = -roomHalfWidth * 0.5f;
    floorBody[1].position.x = roomHalfWidth * 0.5f;

    for (int i = 0; i < 3; ++i) {
        boxBody[i] = Body{};
        boxBody[i].color = glm::vec3(0.93f, 0.18f, 0.14f);
        boxSpeed[i] = 0.0f;
    }
    boxBody[0].size = glm::vec3(0.30f, 0.30f, 0.30f);
    boxBody[1].size = glm::vec3(0.38f, 0.42f, 0.38f);
    boxBody[2].size = glm::vec3(0.25f, 0.35f, 0.25f);
    boxBody[0].position = glm::vec3(0.0f, -roomHalfHeight + boxBody[0].size.y * 0.5f, -0.85f);
    boxBody[1].position = glm::vec3(0.0f, -roomHalfHeight + boxBody[1].size.y * 0.5f, 0.0f);
    boxBody[2].position = glm::vec3(0.0f, -roomHalfHeight + boxBody[2].size.y * 0.5f, 0.85f);

    // sphere.txt의 반지름 0.3을 InitBuffer에서 1로 만든다.
    // 따라서 공의 size.x는 곧 충돌 판정에 사용하는 반지름이다.
    for (int i = 0; i < 5; ++i) {
        ballBody[i] = Body{};
        ballBody[i].size = glm::vec3(0.11f);
        ballBody[i].position = glm::vec3(-0.65f + 0.28f * i, 0.45f - 0.17f * i, -0.65f + 0.26f * i);
        ballBody[i].color = glm::vec3(0.18f, 0.25f, 1.0f);
        ballSpeed[i] = glm::vec3(0.65f + 0.12f * i, 0.90f - 0.08f * i, -0.75f + 0.10f * i);
        if (i % 2 == 1) ballSpeed[i].x = -ballSpeed[i].x;
    }
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
    // 바닥 열림 상태가 켜지면 양쪽 바닥이 아래로 90도 열린다.
    if (isFloorOpen && floorOpenAngle < 90.0f) {
        floorOpenAngle += 90.0f * dt;
        if (floorOpenAngle > 90.0f) floorOpenAngle = 90.0f;
    }
    else if (!isFloorOpen && floorOpenAngle > 0.0f) {
        floorOpenAngle -= 90.0f * dt;
        if (floorOpenAngle < 0.0f) floorOpenAngle = 0.0f;
    }

    for (int i = 0; i < ballCount; ++i) {
        Body& b = ballBody[i];
        float radius = b.size.x;

        // 바닥이 열리는 동안부터 공은 아래로 떨어진다.
        if (floorOpenAngle > 0.0f) ballSpeed[i].y -= 2.0f * dt;
        b.position += ballSpeed[i] * dt;

        // 앞면은 그리지 않지만 공이 화면 밖으로 나오지 않도록 Z 범위는 제한한다.
        if (b.position.x + radius > roomHalfWidth) {
            b.position.x = roomHalfWidth - radius;
            ballSpeed[i].x = -std::fabs(ballSpeed[i].x);
        }
        else if (b.position.x - radius < -roomHalfWidth) {
            b.position.x = -roomHalfWidth + radius;
            ballSpeed[i].x = std::fabs(ballSpeed[i].x);
        }
        if (b.position.y + radius > roomHalfHeight) {
            b.position.y = roomHalfHeight - radius;
            ballSpeed[i].y = -std::fabs(ballSpeed[i].y);
        }
        // 바닥이 완전히 닫혀 있을 때만 아래쪽 벽에 튕긴다.
        else if (floorOpenAngle == 0.0f && b.position.y - radius < -roomHalfHeight) {
            b.position.y = -roomHalfHeight + radius;
            ballSpeed[i].y = std::fabs(ballSpeed[i].y);
        }
        if (b.position.z + radius > roomHalfDepth) {
            b.position.z = roomHalfDepth - radius;
            ballSpeed[i].z = -std::fabs(ballSpeed[i].z);
        }
        else if (b.position.z - radius < -roomHalfDepth) {
            b.position.z = -roomHalfDepth + radius;
            ballSpeed[i].z = std::fabs(ballSpeed[i].z);
        }
    }

    for (int i = 0; i < 3; ++i) {
        Body& b = boxBody[i];
        if (floorOpenAngle == 0.0f) {
            // 방을 기울이면 바닥의 낮은 쪽으로 육면체가 미끄러진다.
            // Z축 회전각이 +이면 왼쪽, -이면 오른쪽으로 이동한다.
            float speedX = -1.2f * std::sin(glm::radians(roomAngleZ));
            b.position.x += speedX * dt;
            b.position.y = -roomHalfHeight + b.size.y * 0.5f;
            boxSpeed[i] = 0.0f;

            if (b.position.x + b.size.x * 0.5f > roomHalfWidth)
                b.position.x = roomHalfWidth - b.size.x * 0.5f;
            else if (b.position.x - b.size.x * 0.5f < -roomHalfWidth)
                b.position.x = -roomHalfWidth + b.size.x * 0.5f;
        }
        else {
            // 열린 바닥 아래로 떨어진다. boxSpeed는 낙하 속도이다.
            boxSpeed[i] -= 2.0f * dt;
            b.position.y += boxSpeed[i] * dt;
        }
    }
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

    GLFWwindow* window = glfwCreateWindow(800, 800, "Test 21 - Bouncing Balls", nullptr, nullptr);
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
    read_obj_file("sphere.txt", &sphere);
    glEnable(GL_DEPTH_TEST); // 은면제거 - 관측자의 시점에서 가까운 면이 보이고 가까운 면에 가려진 면은 안보이게 그린다
    glEnable(GL_CULL_FACE); // 컬링 - 뒷면은 그릴 필요 없으므로 제거함 
    glCullFace(GL_BACK); // 컬링 뒤에면 제거 
    glFrontFace(GL_CCW); // enable, cullface, fronface 3개가 한세트 
    InitBuffer();
    resetScene();
    glfwSetFramebufferSizeCallback(window, Reshape);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    Reshape(window, width, height);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float dt = Timer();
        if (dt > 0.03f) dt = 0.03f; // 창을 오래 잡고 있어도 벽을 뚫지 않도록 한다.
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
    free(sphere.vertices);
    free(sphere.faces);
    free(spherePosition);
    free(sphereColor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void InitBuffer()
{
    cubeVertexCount = static_cast<GLsizei>(cube.face_count * 3);
    cubePosition = (GLfloat*)malloc(cubeVertexCount * 3 * sizeof(GLfloat));
    cubeColor = (GLfloat*)malloc(cubeVertexCount * 3 * sizeof(GLfloat));
    sphereVertexCount = static_cast<GLsizei>(sphere.face_count * 3);
    spherePosition = (GLfloat*)malloc(sphereVertexCount * 3 * sizeof(GLfloat));
    sphereColor = (GLfloat*)malloc(sphereVertexCount * 3 * sizeof(GLfloat));
    if (!cubePosition || !cubeColor || !spherePosition || !sphereColor) exit(EXIT_FAILURE);

    for (size_t i = 0; i < cube.face_count; ++i) {
        unsigned int indices[3] = {cube.faces[i].v1, cube.faces[i].v2, cube.faces[i].v3};
        for (int j = 0; j < 3; ++j) {
            Vertex vertex = cube.vertices[indices[j]];
            size_t offset = (i * 3 + j) * 3;
            cubePosition[offset] = vertex.x - 0.5f;
            cubePosition[offset + 1] = vertex.y - 0.5f;
            cubePosition[offset + 2] = vertex.z - 0.5f;
            cubeColor[offset] = 0.93f;
            cubeColor[offset + 1] = 0.18f;
            cubeColor[offset + 2] = 0.14f;
        }
    }

    for (size_t i = 0; i < sphere.face_count; ++i) {
        unsigned int indices[3] = {sphere.faces[i].v1, sphere.faces[i].v2, sphere.faces[i].v3};
        for (int j = 0; j < 3; ++j) {
            Vertex vertex = sphere.vertices[indices[j]];
            size_t offset = (i * 3 + j) * 3;
            spherePosition[offset] = vertex.x / 0.3f;
            spherePosition[offset + 1] = vertex.y / 0.3f;
            spherePosition[offset + 2] = vertex.z / 0.3f;
            sphereColor[offset] = 0.18f;
            sphereColor[offset + 1] = 0.25f;
            sphereColor[offset + 2] = 1.0f;
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void drawScene()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(vao);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::vec3 cameraPos(0.0f, 0.0f, 7.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    glm::mat4 sceneMove = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, sceneZ));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(sceneAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(roomAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 base = projection * view * sceneMove * Ry * Rz;

    // 벽 4개: 평면의 앞면이 방 안쪽을 바라보도록 resetScene에서 회전했다.
    for (int i = 0; i < 4; ++i) {
        const Body& w = wallBody[i];
        glm::mat4 T = glm::translate(glm::mat4(1.0f), w.position);
        glm::mat4 R = w.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), w.size);
        glm::mat4 wallTransform = base * T * R * S;
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &wallTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planePosition), planePosition, GL_DYNAMIC_DRAW);
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, w.color.r, w.color.g, w.color.b);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnableVertexAttribArray(1);
    }

    // 바닥: 왼쪽/오른쪽 가장자리를 경첩으로 삼아 아래로 연다.
    for (int i = 0; i < 2; ++i) {
        const Body& f = floorBody[i];
        glm::mat4 T = glm::translate(glm::mat4(1.0f), f.position);
        glm::mat4 R = f.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), f.size);
        glm::mat4 toCenter(1.0f), toOrigin(1.0f), floorRotate(1.0f);
        if (i == 0) {
            toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(-roomHalfWidth * 0.5f, 0.0f, 0.0f));
            toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(roomHalfWidth * 0.5f, 0.0f, 0.0f));
            floorRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-floorOpenAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        else {
            toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(roomHalfWidth * 0.5f, 0.0f, 0.0f));
            toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-roomHalfWidth * 0.5f, 0.0f, 0.0f));
            floorRotate = glm::rotate(glm::mat4(1.0f), glm::radians(floorOpenAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        glm::mat4 floorTransform = base * T * toCenter * floorRotate * toOrigin * R * S;
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &floorTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planePosition), planePosition, GL_DYNAMIC_DRAW);
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, f.color.r, f.color.g, f.color.b);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnableVertexAttribArray(1);
    }

    // 바닥의 육면체 3개
    for (int i = 0; i < 3; ++i) {
        const Body& b = boxBody[i];
        glm::mat4 T = glm::translate(glm::mat4(1.0f), b.position);
        glm::mat4 R = b.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), b.size);
        glm::mat4 boxTransform = base * T * R * S;
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &boxTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubePosition, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, cubeVertexCount * 3 * sizeof(GLfloat), cubeColor, GL_DYNAMIC_DRAW);
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, b.color.r, b.color.g, b.color.b);
        // Z축으로 나란히 놓인 육면체들이 겹쳐 보여도 구분되도록 테두리를 그린다.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glVertexAttrib3f(1, 0.35f, 0.03f, 0.02f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnableVertexAttribArray(1);
    }

    // 공: 위치/크기/색상 사용 방식은 실습 18의 sun, earth와 같다.
    for (int i = 0; i < ballCount; ++i) {
        const Body& b = ballBody[i];
        glm::mat4 T = glm::translate(glm::mat4(1.0f), b.position);
        glm::mat4 R = b.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), b.size);
        glm::mat4 ballTransform = base * T * R * S;
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &ballTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, sphereVertexCount * 3 * sizeof(GLfloat), spherePosition, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER, sphereVertexCount * 3 * sizeof(GLfloat), sphereColor, GL_DYNAMIC_DRAW);
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, b.color.r, b.color.g, b.color.b);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        glEnableVertexAttribArray(1);
    }
}

void Reshape(GLFWwindow*, int width, int height)
{
    int side = width < height ? width : height;
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // GLFW는 대소문자를 같은 키로 전달하므로 Shift 상태로 구분한다.
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    if (key == GLFW_KEY_Z) {
        sceneZ += shift ? -0.1f : 0.1f;
        // 방의 가장 가까운 부분이 카메라의 근평면을 넘지 않도록 제한한다.
        if (sceneZ > 4.0f) sceneZ = 4.0f;
    }
    else if (key == GLFW_KEY_Y) {
        sceneAngleY += shift ? 5.0f : -5.0f;
        sceneAngleY = std::fmod(sceneAngleY, 360.0f);
    }
    else if (action == GLFW_PRESS) {
        // 추가/전환 명령은 키를 누르고 있어도 한 번만 실행한다.
        if (key == GLFW_KEY_B) {
            if (ballCount < 5) ++ballCount;
        }
        else if (key == GLFW_KEY_O) {
            isFloorOpen = !isFloorOpen;
        }
        else if (key == GLFW_KEY_C) {
            resetScene();
        }
        else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        isDragging = true;
        double mouseY;
        glfwGetCursorPos(window, &lastMouseX, &mouseY);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isDragging = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // 창 밖에서 버튼을 놓았을 때도 다음 이동에 방이 끌려가지 않게 한다.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        isDragging = false;
    if (isDragging) {
        float dx = static_cast<float>(xpos - lastMouseX);
        roomAngleZ -= dx * 0.35f;
        // 바닥이 뒤집히지 않는 범위에서 기울인다.
        if (roomAngleZ > 60.0f) roomAngleZ = 60.0f;
        else if (roomAngleZ < -60.0f) roomAngleZ = -60.0f;
    }
    lastMouseX = xpos;
}
