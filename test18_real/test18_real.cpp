#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include "shader.h"

GLuint vao, vbo_pos, vbo_color;
GLint modelLocation;
float xAngle = 30.0f;
float yAngle = 30.0f;

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

Model sphere{}; // 불변

struct Body {
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 size = glm::vec3(1.0f);
} ;

Body sun{};
Body earth[3]{};
Body moon[3]{};

GLfloat* spherePosition = nullptr;
GLfloat* sphereColor = nullptr;
GLsizei sphereVertexCount = 0;

// 각 축은 두 정점을 연결한 선으로 그린다.
GLfloat axisPosition[6][3] = {
    { -0.9f, 0.0f, 0.0f },{ 0.9f, 0.0f, 0.0f }, // x축
    { 0.0f, -0.9f, 0.0f },{ 0.0f, 0.9f, 0.0f }, // y축
    { 0.0f, 0.0f, -0.9f },{ 0.0f, 0.0f, 0.9f }  // z축
};
GLfloat axisColor[6][3] = {
    { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f }, // 빨강
    { 0.0f, 0.7f, 0.0f },{ 0.0f, 0.7f, 0.0f }, // 초록
    { 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f }  // 파랑
};

double lastTime = 0.0;
void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void resetScene() {

    sun = Body{};
    sun.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sun.rotation = glm::mat4(1.0f);
    sun.size = glm::vec3(1.0f);

    for (int i = 0; i < 3; ++i) {
        earth[i] = Body{};
        earth[i].position = glm::vec3(1.0f, 0.0f, 0.0f);
        earth[i].rotation = glm::mat4(1.0f);
        earth[i].size = glm::vec3(0.6f);

    }

    for (int i = 0; i < 3; ++i) {
        moon[i] = Body{};
        moon[i].position = glm::vec3(0.3f, 0.0f, 0.0f);
        moon[i].rotation = glm::mat4(1.0f);
        moon[i].size = glm::vec3(0.3f);
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
void Moon_orbit(Body& moon, float degrees) {
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    // 중앙 y축 공전: 위치와 방향을 함께 회전한다.
    moon.position = glm::vec3(R * glm::vec4(moon.position, 1.0f)); // 위치 바꾸기
}

void Earth_orbit(Body& earth, float degrees, float tilt_degrees) { // 모델, 이번에 얼마나 회전할지, 
    glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(tilt_degrees), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec4 originalAxis(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 tiledAxis = tilt * originalAxis;
    glm::vec3 orbitAxis = glm::vec3(tiledAxis);

    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), orbitAxis);
    // 중앙 y축 공전: 위치와 방향을 함께 회전한다.
    earth.position = glm::vec3(R * glm::vec4(earth.position, 1.0f)); // 위치 바꾸기
}

void Move(float dt) {
    float speed0 = 30.0f;
    float speed1 = 45.0f;
    float speed2 = 60.0f;

    float degree0 = speed0 * dt;
    float degree1 = speed1 * dt;
    float degree2 = speed2 * dt;

    Earth_orbit(earth[0], degree0, 0.0f);
    Earth_orbit(earth[1], degree1, 45.0f);
    Earth_orbit(earth[2], degree2, -45.0f);

    float moondegree = 90.0 * dt;
    Moon_orbit(moon[0], moondegree);
    Moon_orbit(moon[1], moondegree);
    Moon_orbit(moon[2], moondegree);

}

float Timer() {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    lastTime = now;
    return dt;
}

void InitBuffer()
{
    sphereVertexCount = static_cast<GLsizei>(sphere.face_count * 3);

    spherePosition = (GLfloat*)malloc(
        sphereVertexCount * 3 * sizeof(GLfloat));

    sphereColor = (GLfloat*)malloc(
        sphereVertexCount * 3 * sizeof(GLfloat));

    if (!spherePosition || !sphereColor) {
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < sphere.face_count; ++i) {
        unsigned int indices[3] = {
            sphere.faces[i].v1,
            sphere.faces[i].v2,
            sphere.faces[i].v3
        };

        for (int j = 0; j < 3; ++j) {
            Vertex vertex = sphere.vertices[indices[j]];
            size_t offset = (i * 3 + j) * 3;

            spherePosition[offset] = vertex.x;
            spherePosition[offset + 1] = vertex.y;
            spherePosition[offset + 2] = vertex.z;
            
            sphereColor[offset] = 0.2f;
            sphereColor[offset + 1] = 0.5f;
            sphereColor[offset + 2] = 1.0f;
            

        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);

    // 정점 위치: 셰이더의 location 0
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // 정점 색상: 셰이더의 location 1
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

// XZ 평면의 원을 선으로 연결하고, 전달받은 행렬로 배치한다.
void drawOrbit(const glm::mat4& transform, float radius)
{
    const int count = 120;
    GLfloat positions[count][3];
    GLfloat colors[count][3];

    for (int i = 0; i < count; ++i) {
        float angle = glm::radians(360.0f * i / count);
        positions[i][0] = radius * std::cos(angle);
        positions[i][1] = 0.0f;
        positions[i][2] = radius * std::sin(angle);

        colors[i][0] = 0.6f;
        colors[i][1] = 0.6f;
        colors[i][2] = 0.6f;
    }

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &transform[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINE_LOOP, 0, count);
}

void drawScene()
{

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(vao);

    // test13의 기본 변환: y축 회전 → x축 회전 → 정사영
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 base = projection * Rx * Ry;
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);

    // 축
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(axisPosition), axisPosition, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(axisColor), axisColor, GL_STATIC_DRAW);

    glDrawArrays(GL_LINES, 0, 6);

    // 지구의 공전 경로: 태양 중심, XZ 평면 및 X축으로 +/-45도 기울인 평면.
    const float tilts[3] = {0.0f, 45.0f, -45.0f};
    for (int i = 0; i < 3; ++i) {
        glm::mat4 tilt = glm::rotate(
            glm::mat4(1.0f), glm::radians(tilts[i]),
            glm::vec3(1.0f, 0.0f, 0.0f));
        drawOrbit(base * tilt, 1.0f);
    }

    // 달의 공전 경로: 각 지구를 중심으로 이동하며 XZ 평면과 평행하다.
    for (int i = 0; i < 3; ++i) {
        glm::mat4 earthMove = glm::translate(glm::mat4(1.0f), earth[i].position);
        drawOrbit(base * earthMove, 0.3f);
    }
    //-----------------------------------------------------------------------------------------
    // 구 (태양)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), sun.position);
        glm::mat4 R = sun.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), sun.size);
        glm::mat4 sunTransform = base * T * R * S; // 중요!

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &sunTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            sphereVertexCount * 3 * sizeof(GLfloat),
            spherePosition, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER,
            sphereVertexCount * 3 * sizeof(GLfloat),
            sphereColor, GL_DYNAMIC_DRAW);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
    // 지구
    {
        for (int i = 0; i < 3; ++i) {
            const Body& e = earth[i];

            glm::mat4 T = glm::translate(glm::mat4(1.0f), e.position);
            glm::mat4 R = e.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), e.size);
            glm::mat4 earthTransform = base * T * R * S; // 중요!

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &earthTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                sphereVertexCount * 3 * sizeof(GLfloat),
                spherePosition, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, 0.0f, 1.0f, 0.0f);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glEnableVertexAttribArray(1);
        }

    }
    // 달
    {
        for (int i = 0; i < 3; ++i) {
            const Body& m = moon[i];

            glm::mat4 T = glm::translate(glm::mat4(1.0f), m.position);
            glm::mat4 R = m.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), m.size);

            glm::vec4 earthCenter = glm::vec4(earth[i].position, 1.0f);
            glm::mat4 earthMove = glm::translate(glm::mat4(1.0f), glm::vec3(earthCenter));

            glm::mat4 moonTransform = base * earthMove * T * R * S; // 중요!

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &moonTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                sphereVertexCount * 3 * sizeof(GLfloat),
                spherePosition, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, 1.0f, 0.0f, 0.0f);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glEnableVertexAttribArray(1);
        }

    }
}

void Reshape(GLFWwindow*, int width, int height)
{
    // 창 크기가 바뀌어도 좌표계의 비율을 유지한다.
    int size = width < height ? width : height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main()
{
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Test 18 Real - XYZ Axes", nullptr, nullptr);
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
    glEnable(GL_DEPTH_TEST);
    read_obj_file("sphere.txt", &sphere);
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
    free(sphere.vertices);
    free(sphere.faces);
    free(spherePosition);
    free(sphereColor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

