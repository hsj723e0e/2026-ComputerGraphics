#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vColor;
out vec3 out_Color;
// PDF 24쪽: uniform mat4 모델링 변환 행렬 선언
uniform mat4 modelTransform;
void main(void)
{
    gl_Position = modelTransform * vec4(vPosition, 1.0); // PDF 24쪽: 행렬을 정점에 적용, 7쪽: 점의 동차 좌표 w=1
    out_Color = vColor;
}
