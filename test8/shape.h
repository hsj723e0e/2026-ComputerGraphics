#pragma once
enum ShapeType {
	NONE, POINT, LINE, TRIANGLE, RECTANGLE
};
//--- 구조체 선언
struct Shape {
	ShapeType type = NONE;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float x = 0.0f, y = 0.0f;
	bool isshow = true;
	float size = 0.1;

	Shape(ShapeType type, float inx, float iny, float inred, float ingreen, float inblue) : type{ type }, r{ inred }, g{ ingreen }, b{ inblue }, x{ inx }, y{ iny }
	{}

	Shape() {}

	void move(float dx, float dy) {
		x += dx;
		y += dy;
		if (x >= 1.0f) x -= 2.0f;
		else if (x < -1.0f) x += 2.0f;
		if (y >= 1.0f) y -= 2.0f;
		else if (y < -1.0f) y += 2.0f;
	}
};
