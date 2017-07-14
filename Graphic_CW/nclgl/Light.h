#pragma once

#include "Vector4.h"
#include "Vector3.h"

class Light
{
public:
	Light(Vector3 position, Vector4 colour, float radius) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}
	~Light(void) {};

	Vector3	GetPosition()	const { return position; }
	void SetPosition(Vector3 val) { this->position = val; }

	float GetRadius()		const { return radius; }
	void SetRadius(float val)	  { this->radius = val; }

	Vector4 GetColour()		const { return colour; }
	void SetColour(Vector4 val)	  { this->colour = val; }

protected:
	Vector3 position;
	Vector4 colour;
	float radius;
};