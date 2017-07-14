#pragma once
#include "OGLRenderer.h"

enum MeshBuffer {
	VERTEX_BUFFER, COLOUR_BUFFER, TEXTURE_BUFFER, NORMAL_BUFFER, TANGENT_BUFFER, INDEX_BUFFER, MAX_BUFFER
};

class Mesh {
public:
	Mesh(void);
	~Mesh(void);

	void SetTexture1(GLuint tex) { this->texture = tex; }
	GLuint GetTexture() { return texture; }
	
	void SetBumpMap(GLuint tex) { this->bumpTexture = tex; }
	GLuint GetBumpMap() { return bumpTexture; }

	virtual void Draw();
	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();

protected:
	void BufferData();
	void GenerateNormals();
	void GenerateTangents();
	Vector3 GenerateTangent(const Vector3 &a, const Vector3 &b,
							 const Vector3 &c, const Vector2 &ta,
							 const Vector2 &tb, const Vector2 &tc);

	GLuint	VAOId;
	GLuint	VBOId[MAX_BUFFER];
	GLuint	numVertices;
	GLuint	type;
	GLuint	texture;
	GLuint	texture2;
	GLuint	numIndices;
	GLuint	bumpTexture;

	unsigned int* indices;

	Vector2*	textureCoords;
	Vector3*	vertices;
	Vector3*	normals;
	Vector3*	tangents;
	Vector4*	colours;
};