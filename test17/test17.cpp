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
Model objects[2]; // 0: 처음 왼쪽에 있던 객체, 1: 처음 오른쪽에 있던 객체
int selected = 3;

// test16의 교수님 코드와 같은 정점/면 저장 구조.
#define MAX_LINE_LENGTH 200
typedef struct {
    float x, y, z;
} Vertex;
typedef struct {
    unsigned int v1, v2, v3;
    int color; // test17 추가: 이 면에 적용할 색 번호
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
Mesh meshes[4];
glm::vec3 faceColor[6] = {
    {1.0f,0.2f,0.2f}, {0.2f,0.4f,1.0f}, {0.2f,1.0f,1.0f},
    {1.0f,1.0f,0.2f}, {1.0f,0.3f,1.0f}, {0.2f,1.0f,0.3f}
};
GLfloat axisPosition[6][3] = {
    {-1.5f,0,0}, {1.5f,0,0}, {0,-1.5f,0}, {0,1.5f,0},
    {0,0,-1.5f}, {0,0,1.5f}
};
GLfloat axisColor[6][3] = {
    {1,0,0}, {1,0,0}, {0,1,0}, {0,1,0}, {0,0.5f,1}, {0,0.5f,1}
};

enum Animation {
    NONE,
    THROUGH_ORIGIN,
    OVER_UNDER,
    SPIN_ORBIT
};
Animation animation = NONE;
Model animationStart[2];
float animationTime = 0.0f;
double lastTime = 0.0;

void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void resetScene() {
    objects[0] = Model{};
    objects[1] = Model{};
    objects[0].position.x = -0.75f;
    objects[1].position.x = 0.75f;
    objects[0].shape = CONE;
    objects[1].shape = CUBE;
    selected = 3;
    animation = NONE;
    animationTime = 0.0f;
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
        else if (strncmp(line, "usemtl ", 7) == 0) { // test17 추가: 면의 색 읽기
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

void orbit(Model& model, float degrees) {
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0,1,0));
    // 중앙 y축 공전: 위치와 방향을 함께 회전한다.
    model.position = glm::vec3(R * glm::vec4(model.position, 1.0f));
    model.rotation = R * model.rotation;
}

void startAnimation(Animation next) {
    if (next == SPIN_ORBIT && animation == SPIN_ORBIT) {
        animation = NONE;
        return;
    }
    // 애니메이션을 시작하는 순간의 상태를 복사해 둔다.
    for (int i = 0; i < 2; ++i) {
        animationStart[i] = objects[i];
    }
    animationTime = 0.0f;
    animation = next;
}

void Move(float dt) {
    if (animation == NONE) {
        return;
    }

    animationTime += dt;

    if (animation == SPIN_ORBIT) {
        // 매 프레임 시작 상태를 기준으로 계산한다.
        float wave = std::sin(animationTime * 1.5f);
        float orbitDegrees = animationTime * 45.0f;
        float rotationDegrees = animationTime * 90.0f;
        float rotationRadians = glm::radians(rotationDegrees);
        glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);

        for (int i = 0; i < 2; ++i) {
            objects[i] = animationStart[i];

            // 두 객체의 크기가 서로 반대 방향으로 변한다.
            float sizeDirection;
            if (i == 0) {
                sizeDirection = 1.0f;
            }
            else {
                sizeDirection = -1.0f;
            }
            float sizeMultiplier = std::exp(sizeDirection * 0.5f * wave);
            objects[i].size *= sizeMultiplier;

            orbit(objects[i], orbitDegrees);
            objects[i].rotation = glm::rotate(
                objects[i].rotation, rotationRadians, rotationAxis);
        }
        return;
    }

    // 자리 교환은 2초 동안 진행한다. progress는 0(시작)부터 1(완료)까지이다.
    float progress = animationTime / 2.0f;
    progress = glm::clamp(progress, 0.0f, 1.0f);

    for (int i = 0; i < 2; ++i) {
        glm::vec3 startPosition = animationStart[i].position;
        glm::vec3 targetPosition;
        if (i == 0) {
            targetPosition = animationStart[1].position;
        }
        else {
            targetPosition = animationStart[0].position;
        }

        if (animation == THROUGH_ORIGIN) {
            glm::vec3 origin(0.0f, 0.0f, 0.0f);
            if (progress < 0.5f) {
                // 앞 1초: 시작 위치에서 원점까지 이동한다.
                float firstHalfProgress = progress * 2.0f;
                objects[i].position = glm::mix(startPosition, origin, firstHalfProgress);
            }
            else {
                // 뒤 1초: 원점에서 상대 객체의 시작 위치까지 이동한다.
                float secondHalfProgress = (progress - 0.5f) * 2.0f;
                objects[i].position = glm::mix(origin, targetPosition, secondHalfProgress);
            }
        }
        else if (animation == OVER_UNDER) {
            objects[i].position = glm::mix(startPosition, targetPosition, progress);

            float height = 0.65f * std::sin(PI * progress);
            if (i == 0) {
                objects[i].position.y += height;
            }
            else {
                objects[i].position.y -= height;
            }
        }

        if (progress >= 1.0f) {
            objects[i].position = targetPosition;
        }
    }

    if (progress >= 1.0f) {
        animation = NONE;
    }
}

float Timer() {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    lastTime = now;
    return dt;
}

void InitBuffer() {
    const char* filenames[4] = {"cone.txt", "cube.txt", "sphere.txt", "pyramid.txt"};
    for (int i = 0; i < 4; ++i) {
        ObjData model{};
        read_obj_file(filenames[i], &model);
        meshes[i] = Mesh{};
        // test16처럼 면에 저장한 정점 번호로 좌표를 찾아 그릴 배열에 넣는다.
        for (size_t j = 0; j < model.face_count; ++j) {
            Face face = model.faces[j];
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

void drawScene() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(vao);
    glm::mat4 Rx = glm::rotate(glm::mat4(1), glm::radians(xAngle), glm::vec3(1,0,0));
    glm::mat4 Ry = glm::rotate(glm::mat4(1), glm::radians(yAngle), glm::vec3(0,1,0));
    glm::mat4 projection = glm::ortho(-1.7f, 1.7f, -1.7f, 1.7f, -10.0f, 10.0f);
    glm::mat4 base = projection * Rx * Ry; // 좌표계와 두 객체에 동일하게 적용
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &base[0][0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 6);

    for (int i = 0; i < 2; ++i) {
        const Model& model = objects[i];
        const Mesh& mesh = meshes[model.shape];
        glm::mat4 T = glm::translate(glm::mat4(1), model.position);
        glm::mat4 S = glm::scale(glm::mat4(1), glm::vec3(model.size));
        // 오른쪽부터 크기 → 자전 → 위치 → 공통 30도 기울기 → 투영
        glm::mat4 modelTransform = base * T * model.rotation * S;
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(glm::vec3), mesh.positions.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);   
        glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(glm::vec3), mesh.colors.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.positions.size()));
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

    if (key == GLFW_KEY_1) {
        selected = 1;
        return;
    }
    else if (key == GLFW_KEY_2) {
        selected = 2;
        return;
    }
    else if (key == GLFW_KEY_3) {
        selected = 3;
        return;
    }

    if (key == GLFW_KEY_S) {
        resetScene();
        return;
    }

    if (key == GLFW_KEY_T || key == GLFW_KEY_U || key == GLFW_KEY_V) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_T) {
                startAnimation(THROUGH_ORIGIN);
            }
            else if (key == GLFW_KEY_U) {
                startAnimation(OVER_UNDER);
            }
            else if (key == GLFW_KEY_V) {
                startAnimation(SPIN_ORBIT);
            }
        }
        return;
    }

    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS) {
            animation = NONE;

            if (objects[0].shape == CONE) {
                objects[0].shape = SPHERE;
            }
            else {
                objects[0].shape = CONE;
            }

            if (objects[1].shape == CUBE) {
                objects[1].shape = PYRAMID;
            }
            else {
                objects[1].shape = CUBE;
            }
        }
        return;
    }

    // 수동으로 위치, 회전, 크기를 바꾸는 키인지 확인한다.
    bool isManualTransform = false;
    if (key == GLFW_KEY_X || key == GLFW_KEY_Y || key == GLFW_KEY_R) {
        isManualTransform = true;
    }
    else if (key == GLFW_KEY_A || key == GLFW_KEY_B) {
        isManualTransform = true;
    }
    else if (key == GLFW_KEY_D || key == GLFW_KEY_E) {
        isManualTransform = true;
    }

    if (!isManualTransform) {
        return;
    }
    animation = NONE; // 현재 애니메이션 위치에서 수동 변환을 이어서 적용한다.

    bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
    bool capsLockOn = (mods & GLFW_MOD_CAPS_LOCK) != 0;
    bool upper = false;
    // Shift와 Caps Lock 중 하나만 켜져 있으면 대문자이다.
    if (shiftPressed && !capsLockOn) {
        upper = true;
    }
    else if (!shiftPressed && capsLockOn) {
        upper = true;
    }

    float direction;
    float scaleMultiplier;
    if (upper) {
        direction = -1.0f;
        scaleMultiplier = 1.0f / 1.1f;
    }
    else {
        direction = 1.0f;
        scaleMultiplier = 1.1f;
    }

    for (int i = 0; i < 2; ++i) {
        bool isSelected = false;
        if (selected == 3) {
            isSelected = true;
        }
        else if (selected == i + 1) {
            isSelected = true;
        }

        if (!isSelected) {
            continue;
        }

        // 참조이므로 model을 수정하면 objects[i]도 함께 바뀐다.
        Model& model = objects[i];
        if (key == GLFW_KEY_X) {
            float rotationRadians = glm::radians(5.0f * direction);
            glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
            model.rotation = glm::rotate(model.rotation, rotationRadians, rotationAxis);
        }
        else if (key == GLFW_KEY_Y) {
            float rotationRadians = glm::radians(5.0f * direction);
            glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
            model.rotation = glm::rotate(model.rotation, rotationRadians, rotationAxis);
        }
        else if (key == GLFW_KEY_R) {
            float orbitDegrees = 5.0f * direction;
            orbit(model, orbitDegrees);
        }
        else if (key == GLFW_KEY_A) {
            // 제자리에서 크기만 바꾼다.
            float nextSize = model.size * scaleMultiplier;
            nextSize = glm::clamp(nextSize, 0.05f, 5.0f);
            model.size = nextSize;
        }
        else if (key == GLFW_KEY_B) {
            // 원점 기준 확대/축소이므로 위치도 같은 비율로 바꾼다.
            float nextSize = model.size * scaleMultiplier;
            nextSize = glm::clamp(nextSize, 0.05f, 5.0f);
            float positionMultiplier = nextSize / model.size;
            model.position *= positionMultiplier;
            model.size = nextSize;
        }
        else if (key == GLFW_KEY_D) {
            model.position.x -= 0.05f * direction;
        }
        else if (key == GLFW_KEY_E) {
            model.position.y += 0.05f * direction;
        }
    }
}

int main() {
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 800, "Test 17 - Modeling Transform", nullptr, nullptr);
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
