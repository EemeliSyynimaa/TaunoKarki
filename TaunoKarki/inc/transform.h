#ifndef TRANSFORM_H__
#define TRANSFORM_H__

#include <string>
#include "component.h"

class Transform : public Component
{
public:
	Transform() : x(0), y(0), z(0) {}
	Transform(float x, float y, float z) : x(x), y(y), z(z) {}
	~Transform();

	void update() { std::cout << "HÖHÖHÖHÖHÖ" << std::endl; }

	static const std::string TAG;

	float x;
	float y;
	float z;

private:
};

const std::string Transform::TAG = "Transform";

#endif