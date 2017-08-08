#pragma once

#include "OGLRenderer.h"

using namespace std;

class ComputeShader
{
public:
	ComputeShader(string compute, bool isVerbose = false);
	~ComputeShader();

	GLuint	GetProgram() { return program; }
	void	UseProgram();
	bool	LinkProgram();

	void Compute(Vector3 workGroups);

protected:
	bool	LoadShaderFile(string from, string & into);
	GLuint	GenerateShader(string from);

	GLuint	object;
	GLuint	program;

	bool	loadFailed;
	bool	verbose; 

	string compute;
};
