#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include "shader.h"

GLuint vao, vbo_pos, vbo_color;
GLint modelLocation;
float xAngle = 30.0f, yAngle = 30.0f;
const float PI = 3.14159265358979323846f;

// 그릴 도형의 종류
 enum Shape {
    CONE,
    CUBE,
    SPHERE,
    PYRAMID
};
struct Model { // 위치, 회전값, 크기, 모양 
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    float size = 1.0f;
    Shape shape = CUBE;
};
Model Sun; // 0: 처음 왼쪽에 있던 객체, 1: 처음 오른쪽에 있던 객체
Model Earth[3];
Model Moon[3];
glm::vec3 scenePosition = glm::vec3(0.0f); // 이동량을 나타내는 행령 

// test16의 교수님 코드와 같은 정점/면 저장 구조.
#define MAX_LINE_LENGTH 200
typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    unsigned int v1, v2, v3;
    int color; // test18 추가: 이 면에 적용할 색 번호
} Face;

typedef struct {
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
} ObjData;

struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
};
Mesh meshes[4]; // 2 : 원 
glm::vec3 faceColor[6] = {
    {1.0f,0.2f,0.2f}, {0.2f,0.4f,1.0f}, {0.2f,1.0f,1.0f},
    {1.0f,1.0f,0.2f}, {1.0f,0.3f,1.0f}, {0.2f,1.0f,0.3f}
};
GLfloat axisPosition[6][3] = {
    {-1.5f, 0, 0}, {1.5f, 0, 0}, {0, -1.5f, 0}, {0, 1.5f, 0},
    {0, 0, -1.5f}, {0, 0, 1.5f}
};
GLfloat axisColor[6][3] = {
    {1,0,0}, {1,0,0}, {0,1,0}, {0,1,0}, {0,0.5f,1}, {0,0.5f,1}
};

double lastTime = 0.0;
bool isM = true;
bool isP = false;

void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void resetScene() {
    Sun = Model{};
    Sun.shape = SPHERE;

    for (int i = 0; i < 3; ++i) {
        Earth[i] = Model{};
        Earth[i].shape = SPHERE;
        Earth[i].position.x= 0.8f;
        Earth[i].size = 0.7f;
    }
    
    for (int i = 0; i < 3; ++i) {
        Moon[i] = Model{};
        Moon[i].shape = SPHERE;
        Moon[i].position = glm::vec3(0.45f, 0.0f, 0.0f);
        Moon[i].size = 0.4f;
    }
}

void triangle(Mesh& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, int color) {
    // 삼각형 하나를 이루는 세 정점을 순서대로 저장한다.
    mesh.positions.push_back(a);
    mesh.positions.push_back(b);
    mesh.positions.push_back(c);

    int colorIndex = color % 6;
    for (int i = 0; i < 3; ++i) {
        mesh.colors.push_back(faceColor[colorIndex]);
    }
}

void read_newline(char* str) // test16과 동일
{
    char* pos = strchr(str, '\n');
    if (pos != NULL) {
        *pos = '\0';
    }
}

void read_obj_file(const char* filename, ObjData* model) // 교수님 코드 기반
{
    FILE* file;
    fopen_s(&file, filename, "r"); // 1. 파일 열기
    if (!file) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];
    model->vertex_count = 0;
    model->face_count = 0;
    while (fgets(line, sizeof(line), file)) { // 2. 정점과 삼각형 개수 세기
        if (line[0] == 'v' && line[1] == ' ') {
            model->vertex_count++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            model->face_count++;
        }
    }
    if (ferror(file) || model->vertex_count == 0 || model->face_count == 0) {
        fprintf(stderr, "Cannot read mesh: %s\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_SET); // 3. 파일 처음으로 돌아가기
    model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
    model->faces = (Face*)malloc(model->face_count * sizeof(Face));
    if (!model->vertices || !model->faces) {
        fclose(file);
        exit(EXIT_FAILURE);
    }
    size_t vertex_index = 0, face_index = 0;
    int color = 0;
    while (fgets(line, sizeof(line), file)) { // 4. 한 줄씩 읽어 배열에 저장
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ') {
            if (vertex_index >= model->vertex_count) {
                fclose(file);
                exit(EXIT_FAILURE);
            }
            int result = sscanf_s(line + 2, "%f %f %f",
                &model->vertices[vertex_index].x,
                &model->vertices[vertex_index].y,
                &model->vertices[vertex_index].z);
            if (result != 3 || !std::isfinite(model->vertices[vertex_index].x) ||
                !std::isfinite(model->vertices[vertex_index].y) ||
                !std::isfinite(model->vertices[vertex_index].z)) {
                fprintf(stderr, "Invalid vertex: %s\n", line);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            vertex_index++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int v1 = 0, v2 = 0, v3 = 0;
            // test16과 동일: 법선 번호는 %*u로 읽고 저장하지 않는다.
            int result = sscanf_s(line + 2, "%u//%*u %u//%*u %u//%*u", &v1, &v2, &v3);
            if (result != 3 || v1 == 0 || v2 == 0 || v3 == 0 ||
                v1 > model->vertex_count || v2 > model->vertex_count || v3 > model->vertex_count ||
                face_index >= model->face_count) {
                fprintf(stderr, "Invalid face: %s\n", line);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            model->faces[face_index].v1 = v1 - 1; // OBJ 번호 1부터 → 배열 번호 0부터
            model->faces[face_index].v2 = v2 - 1;
            model->faces[face_index].v3 = v3 - 1;
            model->faces[face_index].color = color;
            face_index++;
        }
        else if (strncmp(line, "usemtl ", 7) == 0) { // test18 추가: 면의 색 읽기
            int result = sscanf_s(line + 7, "color%d", &color);
            if (result != 1 || color < 0 || color >= 6) {
                fprintf(stderr, "Invalid color: %s\n", line);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (ferror(file) || vertex_index != model->vertex_count || face_index != model->face_count) {
        fprintf(stderr, "Incomplete mesh: %s\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file); // 5. 파일 닫기
}

void Earth_orbit(Model& model, float degrees, float tilt_degrees) { // 모델, 이번에 얼마나 회전할지, 
    glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(tilt_degrees), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec4 originalAxis(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 tiledAxis = tilt * originalAxis;
    glm::vec3 orbitAxis = glm::vec3(tiledAxis);


    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), orbitAxis);
    // 중앙 y축 공전: 위치와 방향을 함께 회전한다.
    model.position = glm::vec3(R * glm::vec4(model.position, 1.0f)); // 위치 바꾸기
    model.rotation = R * model.rotation; // 자기자신을 회전
}

void Moon_orbit(Model& model, float degrees) { // 모델, 이번에 얼마나 회전할지, 

    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
    // 중앙 y축 공전: 위치와 방향을 함께 회전한다.
    model.position = glm::vec3(R * glm::vec4(model.position, 1.0f)); // 위치 바꾸기
    model.rotation = R * model.rotation; // 자기자신을 회전
}

void Move(float dt) {
    float speed0 = 30.0f;
    float speed1 = 45.0f;
    float speed2 = 60.0f;

    float degrees0 = speed0 * dt;
    float degrees1 = speed1 * dt;
    float degrees2 = speed2 * dt;

    Earth_orbit(Earth[0], degrees0, 0.0f);
    Earth_orbit(Earth[1], degrees1, 45.0f);
    Earth_orbit(Earth[2], degrees2, -45.0f);

    float moondegrees = 90.0f * dt;
    Moon_orbit(Moon[0], moondegrees);
    Moon_orbit(Moon[1], moondegrees);
    Moon_orbit(Moon[2], moondegrees);


}

float Timer() {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    lastTime = now;
    return dt;
}

void InitBuffer() {
    const char* filenames[4] = { "cone.txt", "cube.txt", "sphere.txt", "pyramid.txt" };
    for (int i = 0; i < 4; ++i) {
        ObjData model{};
        read_obj_file(filenames[i], &model);
        meshes[i] = Mesh{};
        // test16처럼 면에 저장한 정점 번호로 좌표를 찾아 그릴 배열에 넣는다.
        for (size_t j = 0; j < model.face_count; ++j) {
            Face face = model.faces[j];
            //if (i == SPHERE) { // 원을 그릴 경우 한 색깔라만 나오게 한다    
            //    face.color = 1;
            //}
            Vertex a = model.vertices[face.v1];
            Vertex b = model.vertices[face.v2];
            Vertex c = model.vertices[face.v3];
            triangle(meshes[i], {a.x, a.y, a.z}, {b.x, b.y, b.z},
                {c.x, c.y, c.z}, face.color);
        }
        free(model.vertices);
        free(model.faces);
    }
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(1);
}
// -------------------------------------------------------------
void drawOrbit(const glm::mat4& transform, float radius) {
    const int count = 120;
    glm::vec3 positions[count];
    glm::vec3 colors[count];

    for (int i = 0; i < count; i++) {
        float degrees = 360.0f * i / count;
        float radians = glm::radians(degrees);

        positions[i].x = radius * std::cos(radians);
        positions[i].y = radius * std::sin(radians);
        positions[i].z = 0.0f;

        colors[i] = glm::vec3(0.7f, 0.7f, 0.7f);
    }

    glUniformMatrix4fv(
        modelLocation, 1, GL_FALSE, &transform[0][0]
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(positions),
        positions, GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(colors),
        colors, GL_DYNAMIC_DRAW
    );

    glDrawArrays(GL_LINE_LOOP, 0, count);
}
// -------------------------------------------------------------

void drawScene() {
    glEnable(GL_DEPTH_TEST); 
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(vao);
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0), glm::radians(xAngle), glm::vec3(1.0f,0.0f,0.0f));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0), glm::radians(yAngle), glm::vec3(0.0f,1.0f,0.0f));

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.5f); //--카메라 위치 
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); //--카메라가 바라보는 기준점
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);  //--카메라의 위쪽 방향
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    glm::mat4 projection;
    if (!isP) {
        projection = glm::ortho(-1.7f, 1.7f, -1.7f, 1.7f, -10.0f, 10.0f); // 4개인자, 값 낮게하면 좁은공간을 화면전체에 표시 즉 크게보임, 깊이
    } else projection = glm::perspective(glm::radians(68.43f), 1.0f, 0.1f, 100.0f); // fovy:커지면 넓게보여 물체가 작아짐, aspect:크면 가로가 넓게보임
    // near: 커지면 가까운 물체가 더 많이 잘림 far :  커지면 더 먼 물체까지 표시 가능

    glm::mat4 sceneMove = glm::translate(glm::mat4(1.0f),scenePosition);

    glm::mat4 base = projection * view * sceneMove* Rx * Ry; // 좌표계와 두 객체에 동일하게 적용
    // 좌표축에는 전체 이동을 적용하지 않는다.
    glm::mat4 axisTransform = projection * view * Rx * Ry; // 축은 이동안함 

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &axisTransform[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_DYNAMIC_DRAW);
    if (!isM) { // 솔리드 ? 와이어? 뭐하지
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, 6);
    //--------------------------------------------------------------
    // xy 원을 xz 평면으로 돌린다.
    glm::mat4 xzPlane = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    glm::mat4 tiltPlus = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(45.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    glm::mat4 tiltMinus = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(-45.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    // 행성의 공전 반지름은 현재 0.8
    drawOrbit(base * xzPlane, 0.8f);
    drawOrbit(base * tiltPlus * xzPlane, 0.8f);
    drawOrbit(base * tiltMinus * xzPlane, 0.8f);
    //--------------------------------------------------------------
    // 1. Sun 그리기
    {
        const Model& model = Sun;
        const Mesh& mesh = meshes[model.shape];

        glm::mat4 T = glm::translate(glm::mat4(1), model.position);
        glm::mat4 S = glm::scale(glm::mat4(1), glm::vec3(model.size));
        glm::mat4 modelTransform = base * T * model.rotation * S; // 중요!

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.positions.size() * sizeof(glm::vec3),
            mesh.positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.colors.size() * sizeof(glm::vec3),
            mesh.colors.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0,
            static_cast<GLsizei>(mesh.positions.size()));
    }

    // 2. Earth 3개 그리기
    for (int i = 0; i < 3; i++) {
        const Model& model = Earth[i];
        const Mesh& mesh = meshes[model.shape];

        glm::mat4 T = glm::translate(glm::mat4(1), model.position);
        glm::mat4 S = glm::scale(glm::mat4(1), glm::vec3(model.size));
        glm::mat4 modelTransform = base * T * model.rotation * S; // 중요!

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.positions.size() * sizeof(glm::vec3),
            mesh.positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.colors.size() * sizeof(glm::vec3),
            mesh.colors.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0,
            static_cast<GLsizei>(mesh.positions.size()));
    }

    // 3. Moon 3개 그리기
    for (int i = 0; i < 3; i++) {
        const Model& model = Moon[i];
        const Mesh& mesh = meshes[model.shape];

        glm::vec4 earthCenter = Rx * Ry * glm::vec4(Earth[i].position, 1.0f);
        glm::mat4 earthMove = glm::translate(glm::mat4(1.0f), glm::vec3(earthCenter));
        //--------------------------------------------------------------
        // 달 궤도의 중심을 해당 Earth로 옮긴다.
        glm::mat4 moonOrbitTransform = projection * view * sceneMove * earthMove;

        // 달의 공전 반지름은 현재 0.45
        drawOrbit(moonOrbitTransform, 0.45f);
        //--------------------------------------------------------------

        glm::mat4 moonMove = glm::translate(glm::mat4(1.0f), model.position);
        glm::mat4 S = glm::scale(glm::mat4(1.0f),glm::vec3(model.size));

        glm::mat4 modelTransform = projection * view * sceneMove * earthMove* moonMove * model.rotation * S;

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.positions.size() * sizeof(glm::vec3),
            mesh.positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glBufferData(GL_ARRAY_BUFFER,
            mesh.colors.size() * sizeof(glm::vec3),
            mesh.colors.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0,
            static_cast<GLsizei>(mesh.positions.size()));
    }
}

void Reshape(GLFWwindow*, int width, int height) {
    int size;
    if (width < height) {
        size = width;
    }
    else {
        size = height;
    }
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    // 키를 처음 누르거나, 누른 상태로 반복 입력될 때만 처리한다.
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    if (key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    //if (key == GLFW_KEY_S) {
    //    resetScene();
    //    return;
    //}

    if (key == GLFW_KEY_M) {
        isM = !isM; 
        return;
    }
    if (key == GLFW_KEY_P) {
        isP = !isP;
        return;
    }
    if (key == GLFW_KEY_W) {
        scenePosition.y += 0.05f;
        return;
    }
    if (key == GLFW_KEY_S) {
        scenePosition.y -= 0.05f;
        return;
    }
    if (key == GLFW_KEY_A) {
        scenePosition.x -= 0.05f;
        return;
    }
    if (key == GLFW_KEY_D) {
        scenePosition.x += 0.05f;
        return;
    }
}

int main() {
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 800, "Test 18 - Modeling Transform", nullptr, nullptr);
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
    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();
    if (!shaderProgramID) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
    InitBuffer();
    resetScene();
    glfwSetFramebufferSizeCallback(window, Reshape);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    Reshape(window, width, height);
    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // 입력 처리 → 시간 계산 → 애니메이션 업데이트 → 그리기 → 화면 표시
        float dt = Timer();
        Move(dt);
        drawScene();
        glfwSwapBuffers(window);
    }
    glDeleteBuffers(1, &vbo_pos);
    glDeleteBuffers(1, &vbo_color);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgramID);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
