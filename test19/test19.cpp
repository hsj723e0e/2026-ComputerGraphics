#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
Model cube{};

struct Body {
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 size = glm::vec3(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
};


Body bottomBody[2];
Body midBody;
Body topBody[2];
Body flagpole[2];
Body gunBarrel[2];
Body floorBody;
glm::vec3 scenePosition(0.0f);
float sizedy = 0.05f;                 // 배율 증가
bool isT;
float midAngleY = 0.0f;
float midRotateSpeed = 60.0f;
bool isL;
float topBodyTransformSpeed = 0.46f;
bool isG;
float gunBarrelRotateAngle = 0.0f;
float gunBarrelRotateSpeed = 30.0f;
bool isP;
float flagpoleRotateAngle = 0.0f;
float flagpoleRotateSpeed = 30.0f;

GLfloat* cubePosition = nullptr;
GLfloat* cubeColor = nullptr;
GLsizei cubeVertexCount = 0;
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
    floorBody = Body{};
    floorBody.position = glm::vec3(0.0f, -0.05f, 0.0f);
    floorBody.rotation = glm::mat4(1.0f);
    floorBody.size = glm::vec3(5.0f, 0.05f, 5.0f);
    floorBody.color = glm::vec3(222.0f / 255.0f, 235.0f / 255.0f, 247.0f / 255.0f);

    for (int i = 0; i < 2; ++i) {
        bottomBody[i] = Body{};
        bottomBody[i].rotation = glm::mat4(1.0f);
        bottomBody[i].size = glm::vec3(0.4f, 0.15f, 0.2f);
        bottomBody[i].color = glm::vec3(0.2f, 0.2f, 0.2f);
    }
    bottomBody[0].position = glm::vec3(-0.23f, 0.05f, 0.0f);
    bottomBody[1].position = glm::vec3(0.23f, 0.05f, 0.0f);

    midBody = Body{};
    midBody.position = glm::vec3(0.0f, 0.1625f, 0.0f);
    midBody.rotation = glm::mat4(1.0f);
    midBody.size = glm::vec3(0.5f, 0.075f, 0.2f);
    midBody.color = glm::vec3(0.65f, 0.65f, 0.65f);

    for (int i = 0; i < 2; ++i) {
        topBody[i] = Body{};
        topBody[i].rotation = glm::mat4(1.0f);
        topBody[i].size = glm::vec3(0.25f, 0.1f, 0.2f);
        topBody[i].color = glm::vec3(0.2f, 0.8f, 0.2f);
    }
    topBody[0].position = glm::vec3(-0.23f, 0.25f, 0.0f);
    topBody[1].position = glm::vec3(0.23f, 0.25f, 0.0f);

    for (int i = 0; i < 2; ++i) {
        // 정점 좌표를 -0.5~0.5로 변환했으므로 size의 절반이 반높이다.
        flagpole[i] = Body{};
        flagpole[i].rotation = glm::mat4(1.0f);
        flagpole[i].size = glm::vec3(0.02f, 0.18f, 0.02f);
        flagpole[i].position = topBody[i].position + glm::vec3(0.0f, (topBody[i].size.y + flagpole[i].size.y) * 0.5f, 0.0f);
        flagpole[i].color = glm::vec3(1.0f, 0.8f, 0.0f);

        // 앞쪽은 +Z 방향. 포신 뒷면을 상부 몸체 앞면에 붙인다.
        gunBarrel[i] = Body{};
        gunBarrel[i].rotation = glm::mat4(1.0f);
        gunBarrel[i].size = glm::vec3(0.015f, 0.015f, 0.16f);
        gunBarrel[i].position = topBody[i].position + glm::vec3(0.0f, 0.0f, (topBody[i].size.z + gunBarrel[i].size.z) * 0.5f);
        gunBarrel[i].color = glm::vec3(0.9f, 0.9f, 0.1f);
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
    if (isT) {
        midAngleY += midRotateSpeed * dt;
    }
    if (isL) {
        topBody[0].position.x += topBodyTransformSpeed * dt;
        topBody[1].position.x -= topBodyTransformSpeed * dt;
        flagpole[0].position.x += topBodyTransformSpeed * dt;
        flagpole[1].position.x -= topBodyTransformSpeed * dt;
        gunBarrel[0].position.x += topBodyTransformSpeed * dt;
        gunBarrel[1].position.x -= topBodyTransformSpeed * dt;
        if (topBody[0].position.x >= 0.23f) {
            isL = false;
            topBodyTransformSpeed = -topBodyTransformSpeed;
        }
        else if (topBody[0].position.x <= -0.23f) {
            isL = false;
            topBodyTransformSpeed = -topBodyTransformSpeed;

        }
    }
    if (isG) {
        gunBarrelRotateAngle += gunBarrelRotateSpeed * dt;
    }
    if (isP) {
        flagpoleRotateAngle += flagpoleRotateSpeed * dt;
        if (flagpoleRotateAngle >= 30.0f) {
            flagpoleRotateSpeed = -30.0f;
        }
        else if (flagpoleRotateAngle <= -30.0f) {
            flagpoleRotateSpeed = 30.0f;
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // 정점 색상: 셰이더의 location 1
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
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

    glm::mat4 sceneMove = glm::translate(glm::mat4(1.0f), scenePosition);

    glm::mat4 base = projection * Rx * Ry * sceneMove;
    glm::mat4 axisTransform = projection * Rx * Ry;
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &axisTransform[0][0]);

    // 축
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(axisPosition), axisPosition, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(axisColor), axisColor, GL_STATIC_DRAW);

    glDrawArrays(GL_LINES, 0, 6);

    // 땅
    {
        Body& f = floorBody;
        glm::mat4 T = glm::translate(glm::mat4(1.0f), f.position);
        glm::mat4 R = f.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), f.size);
        glm::mat4 floorTransform = axisTransform * T * R * S; // 중요!

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &floorTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            cubeVertexCount * 3 * sizeof(GLfloat),
            cubePosition, GL_DYNAMIC_DRAW);

        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, f.color.r, f.color.g, f.color.b);

        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);

        glEnableVertexAttribArray(1);
    }
    // 아래몸체
    {
        for (int i = 0; i < 2; ++i) {
            const Body& bb = bottomBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), bb.position);
            glm::mat4 R = bb.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), bb.size);
            glm::mat4 bbTransform = base * T * R * S; // 중요!

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &bbTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubeColor, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, bb.color.r, bb.color.g, bb.color.b);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glEnableVertexAttribArray(1);
        }
    }
    // 중앙몸체
    {
        const Body& mb = midBody;
        glm::mat4 T = glm::translate(glm::mat4(1.0f), mb.position);
        glm::mat4 R = mb.rotation;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), mb.size);
        glm::mat4 midRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(midAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 mbTransform = base * midRotateY * T * R * S;

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &mbTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            cubeVertexCount * 3 * sizeof(GLfloat),
            cubePosition, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER,
            cubeVertexCount * 3 * sizeof(GLfloat),
            cubeColor, GL_DYNAMIC_DRAW);

        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, mb.color.r, mb.color.g, mb.color.b);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableVertexAttribArray(1);

    }
    // 상부몸체
    {
        for (int i = 0; i < 2; ++i) {
            const Body& tb = topBody[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), tb.position);
            glm::mat4 R = tb.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), tb.size);
            glm::mat4 midRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(midAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 tbTransform = base * midRotateY * T * R * S; // 중요!

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &tbTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubeColor, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, tb.color.r, tb.color.g, tb.color.b);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glEnableVertexAttribArray(1);
        }
    }
    // 깃대
    {
        for (int i = 0; i < 2; ++i) {
            const Body& fp = flagpole[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), fp.position);
            glm::mat4 R = fp.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), fp.size);
            glm::mat4 midRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(midAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 flagpoleRotate; glm::mat4 toCenter; glm::mat4 toOrigin;
            if (i == 0) {
                flagpoleRotate = glm::rotate(glm::mat4(1.0f), glm::radians(flagpoleRotateAngle), glm::vec3(1.0f, 0.0f, 0.0f));
                toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, fp.size.y * 0.5f , 0.0f));
                toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -fp.size.y * 0.5f , 0.0f));
            }
            else if (i == 1) {
                flagpoleRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-flagpoleRotateAngle), glm::vec3(1.0f, 0.0f, 0.0f));
                toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, fp.size.y * 0.5f, 0.0f));
                toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -fp.size.y * 0.5f, 0.0f));
            }
            glm::mat4 fpTransform = base * midRotateY * T * toOrigin * flagpoleRotate * toCenter * R * S;

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &fpTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubeColor, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, fp.color.r, fp.color.g, fp.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnableVertexAttribArray(1);
        }
    }
    // 포신
    {
        for (int i = 0; i < 2; ++i) {
            const Body& gb = gunBarrel[i];
            glm::mat4 T = glm::translate(glm::mat4(1.0f), gb.position);
            glm::mat4 R = gb.rotation;
            glm::mat4 S = glm::scale(glm::mat4(1.0f), gb.size);
            glm::mat4 midRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(midAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 gunBarrelRotate; glm::mat4 toCenter; glm::mat4 toOrigin;
            if (i == 0) {
                gunBarrelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(gunBarrelRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
                toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.23f, 0.0f, 0.0f));
                toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.23f, 0.0f, 0.0f));
            }
            else if (i == 1) {
                gunBarrelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-gunBarrelRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
                toCenter = glm::translate(glm::mat4(1.0f), glm::vec3(-0.23f, 0.0f, 0.0f));
                toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.23f, 0.0f, 0.0f));
            }
            glm::mat4 gbTransform = base * toOrigin * gunBarrelRotate * toCenter * midRotateY * T * R * S;

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &gbTransform[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubePosition, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
            glBufferData(GL_ARRAY_BUFFER,
                cubeVertexCount * 3 * sizeof(GLfloat),
                cubeColor, GL_DYNAMIC_DRAW);

            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, gb.color.r, gb.color.g, gb.color.b);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
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

void keyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        scenePosition.x += 0.05f;
        return;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        scenePosition.x -= 0.05f;
        return;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        scenePosition.z -= 0.05f;
        return;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        scenePosition.z += 0.05f;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (bottomBody[0].size.y > 0.4) sizedy = -0.03f;
        else if (bottomBody[0].size.y < 0.05) sizedy = 0.03f;
        for (int i = 0; i < 2; ++i) {
            bottomBody[i].size.y += sizedy; // 배율증가량임

            bottomBody[i].position.y += sizedy * 0.5;
            topBody[i].position.y += sizedy;
            flagpole[i].position.y += sizedy;
            gunBarrel[i].position.y += sizedy;
        }
        midBody.position.y += sizedy;
    } 
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        isT = !isT;
        if (mods & GLFW_MOD_SHIFT) {
            midRotateSpeed = -60.0f;
        }
        else midRotateSpeed = 60.0f;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        isL = true;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        isG = !isG;
        if (mods & GLFW_MOD_SHIFT) {
            gunBarrelRotateSpeed = -30.0f;
        }
        else gunBarrelRotateSpeed = 30.0f;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        isP = !isP;
    } 
}
