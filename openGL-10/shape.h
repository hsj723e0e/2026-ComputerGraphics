#pragma once
enum ShapeType {
	NONE, POINT, LINE, TRIANGLE, RECTANGLE,
    //----------------------------------------------------------------------------------임시 아래 
    LINE_RIGHT_TRIANGLE, LINE_TRIANGLE, LINE_RECTANGLE, RIGHT_TRIANGLE, RECTANGLE_LIKEWINDOW
     //----------------------------------------------------------------------------------임시 위

};
//--- 구조체 선언
struct Shape {
	ShapeType type = NONE;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float x = 0.0f, y = 0.0f;
	bool isshow = true;
	float size = 0.1;
	float speedX = 0.9;
	float speedY = 0.9;
    int rotationSteps = 0; // 회전 인덱스 90도는 1, 180도는 2
    bool outline = false; // 테두리를 그릴거냐?
    bool draggable = false;
    //----------------------------------------------------------------------------------임시 아래

    float verticalDistance = 0.0f;
    bool verticalMoving = false;
    float verticalDirection = -1.0f; // -1: 아래, 1: 위
    float angle = 0.0f;
    float radius = 0.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;
    //----------------------------------------------------------------------------------임시 위로

	Shape(ShapeType type, float inx, float iny, float inred, float ingreen, float inblue) : type{ type }, r{ inred }, g{ ingreen }, b{ inblue }, x{ inx }, y{ iny }
	{}

	float vertices[6][3] = {}; // 최대 사각형까지 담는 배열인데 위치를 담는
    float colors[6][3] = {}; // 최대 사각형까지 담는 배열인데 색깔을 담는
	int vertexCount = 0; // 정점 갯수가 몇개인가? 삼각형은 3 사각형은 6 점은 1
	Shape() {}

	void move(float dx, float dy) {
		x += dx;
		y += dy;
		if (x >= 1.0f) x -= 2.0f;
		else if (x < -1.0f) x += 2.0f;
		if (y >= 1.0f) y -= 2.0f;
		else if (y < -1.0f) y += 2.0f;
	}
    void setVertex(int i, float vx, float vy) { // 좌표 하나를 저장하는 함수

        vertices[i][0] = vx; // x
        vertices[i][1] = vy; // y
        vertices[i][2] = 0.0f; // z
    }
    void updateVertices() { // 도형 하나의 정점 좌표를 계산하는 역할


        if (type == POINT) {
            vertexCount = 1;
            setVertex(0, x, y);
        }
        else if (type == TRIANGLE) {
            vertexCount = 3;
           

            setVertex(0, x - size, y + size);
            setVertex(1, x - size, y - size);
            setVertex(2, x + size , y );
        }
        else if (type == RIGHT_TRIANGLE) {
            vertexCount = 3;


            setVertex(0, x - size, y + 2 * size);
            setVertex(1, x - size, y - size);
            setVertex(2, x + size, y - size);
        }
        else if (type == RECTANGLE) {
            vertexCount = 6;

            // 현재 사각형 코드의 크기는 0.1f로 고정

            // 첫 번째 삼각형
            setVertex(0, x - size, y - size);
            setVertex(1, x + size, y - size);
            setVertex(2, x + size, y + size);

            // 두 번째 삼각형
            setVertex(3, x - size, y - size);
            setVertex(4, x + size, y + size);
            setVertex(5, x - size, y + size);
        }
        //----------------------------------------------------------------------------------임시 여기 아래로 LINE으로 테두리 그리는
        else if (type == LINE_TRIANGLE) {
            // 현재 LINE은 삼각형 테두리를 의미
            vertexCount = 6;
            size = 0.1;
            // 아래쪽 변
            setVertex(0, x - size, y - size);
            setVertex(1, x + size, y - size);

            // 오른쪽 변
            setVertex(2, x + size, y - size);
            setVertex(3, x, y + size );

            // 왼쪽 변
            setVertex(4, x, y + size );
            setVertex(5, x - size, y - size);
        }
        else if (type == LINE_RIGHT_TRIANGLE) {
            vertexCount = 6;
            size = 0.1;
            // 왼쪽 세로 변
            setVertex(0, x - size, y + 2 * size);
            setVertex(1, x - size, y - size);

            // 아래쪽 가로 변
            setVertex(2, x - size, y - size);
            setVertex(3, x + size, y - size);

            // 빗변
            setVertex(4, x + size, y - size);
            setVertex(5, x - size, y + 2 * size);
        }
        else if (type == RECTANGLE_LIKEWINDOW) {
            vertexCount = 6;

            // 현재 사각형 코드의 크기는 0.1f로 고정

            // 첫 번째 삼각형
            setVertex(0, x - size, y - size * 1.5f);
            setVertex(1, x + size, y - size * 1.5f);
            setVertex(2, x + size, y + size * 1.5f);

            // 두 번째 삼각형
            setVertex(3, x - size, y - size * 1.5f);
            setVertex(4, x + size, y + size * 1.5f);
            setVertex(5, x - size, y + size * 1.5f);
        }
        //----------------------------------------------------------------------------------임시 여기 위부터
        else {
            vertexCount = 0;
        }
        //----------------------------------------------------------------------------------임시 여기 아래부터
        for (int step = 0; step < rotationSteps; step++) {
            for (int i = 0; i < vertexCount; i++) {
                float dx = vertices[i][0] - x;
                float dy = vertices[i][1] - y;

                vertices[i][0] = x - dy;
                vertices[i][1] = y + dx;
            }
        }
        
        //---------------------------------------------------------------------------------- 여기 위로

    }

    void updateColors() {
        for (int i = 0; i < vertexCount; i++) {
            colors[i][0] = r;
            colors[i][1] = g;
            colors[i][2] = b;
        }
    }
    float minX() { // 정점 중에서 x값의 최소값을 찾는다 
        updateVertices();
        if (vertexCount == 0) return x;

        float result = vertices[0][0];
        for (int i = 1; i < vertexCount; i++) {
            if (vertices[i][0] < result)
                result = vertices[i][0];
        }
        return result;
    }

    float maxX() { // 정점 중에서 x값의 최대값을 찾는다.
        updateVertices();
        if (vertexCount == 0) return x;

        float result = vertices[0][0];
        for (int i = 1; i < vertexCount; i++) {
            if (vertices[i][0] > result)
                result = vertices[i][0];
        }
        return result;
    }

    float minY() { // 정점 중에서 y값의 최소값을 찾는다.
        updateVertices();
        if (vertexCount == 0) return y;

        float result = vertices[0][1];
        for (int i = 1; i < vertexCount; i++) {
            if (vertices[i][1] < result)
                result = vertices[i][1];
        }
        return result;
    }

    float maxY() { // 정점 중에서 y값의 최대값을 찾는다.
        updateVertices();
        if (vertexCount == 0) return y;

        float result = vertices[0][1];
        for (int i = 1; i < vertexCount; i++) {
            if (vertices[i][1] > result)
                result = vertices[i][1];
        }
        return result;
    }

    void rotate90() {
        rotationSteps = (rotationSteps + 1) % 4;
        updateVertices();
    }
};
