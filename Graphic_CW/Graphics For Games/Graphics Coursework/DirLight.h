#pragma once

#include "../../nclgl/Vector3.h"
#include "../../nclgl/Vector4.h"

class DirectionLight
{
public:
	DirectionLight(Vector3 direction, Vector4 colour)
	{
		this->lightDir = direction;
		this->lightColour = colour;
	}
	~DirectionLight() {};

	Vector3 GetDirection() { return lightDir; }
	void SetDirection(Vector3 val) { this->lightDir = val; }

	Vector4 GetColour() { return lightColour; }
	void SetColour(Vector4 val) { this->lightColour = val; }

protected:
	Vector3 lightDir;
	Vector4 lightColour;
};