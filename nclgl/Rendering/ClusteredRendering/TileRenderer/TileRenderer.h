#pragma once

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

#include "GL/glew.h"
#include "GL/wglew.h"
#include "SOIL.h"

#include "../../../Maths/Matrix4.h"
#include "../../../Maths/Plane.h"

#include "../../ShaderTypes/ComputeShader.h"
#include "GridUtility.h"

#include <vector>
#include <type_traits>
#include "../Game/GraphicsConfiguration/GLConfig.h"

class Light;

const bool THROW_ERROR = true;

struct TileData 
{
	int lightIndexes[GLConfig::NUM_TILES];
	int tileLights[GLConfig::NUM_TILES][GLConfig::NUM_LIGHTS];
};

struct ScreenSpaceData
{
	float indexes[GLConfig::NUM_LIGHTS];
	Vector4 numLightsIn;

	Vector4 data[GLConfig::NUM_LIGHTS];
};

class TileRenderer
{
public:
	TileRenderer(Light** lights, int numLights, int numXTiles, int numYTiles, int numZTiles, 
		Vector2 minScreenCoord, Vector2 maxScreenCoord);
	TileRenderer();

	~TileRenderer() 
	{
		delete compute;
	}

	TileData* GetTileData() const
	{
		return tileData;
	}

	Tile* GetScreenTiles()
	{
		return screenTiles;
	}

	int GetNumTiles()  const
	{
		return numTiles;
	}

	void GenerateGrid();
	void InitGridSSBO();

	void AllocateLightsCPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, GLuint buffer, const Vector3& cameraPos);
	void AllocateLightsGPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, const Vector3& cameraPos) const;
	
	ComputeShader* dataPrep;
	ComputeShader* compute;

private:
	void PrepareDataCPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, const Vector3& cameraPos);
	void PrepareDataGPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, const Vector3& cameraPos) const;

	void FillTilesGPU() const;
	void FillTilesCPU(GLuint buffer);
	void CullLights();

	Light** lights;

	Vector2 minCoord;

	int numLights;
	int numTiles;
	int numLightsInFrustum = 0;

	Vector3 gridSize;
	Vector3 gridDimensions;

	//Data
	Tile screenTiles[GLConfig::NUM_TILES];
	Cube grid[GLConfig::NUM_TILES];
	CubePlanes* gridPlanes;
	
	Cube screenCube;
	TileData* tileData;

	std::vector<int> lightsInFrustrum;

	Vector4 screenLightData[GLConfig::NUM_LIGHTS];
	ScreenSpaceData ssdata;
	Matrix4 lightModelMatrices[GLConfig::NUM_LIGHTS];

	//SSBO Stuff
	GLuint tileDataSSBO;
	GLuint gridPlanesSSBO;
	GLuint screenSpaceDataSSBO;
	GLuint modelMatricesSSBO;
	GLuint screenCubeSSBO;

	GLuint countBuffer;
	GLuint count;
	CubePlanes screenPlanes;

	//Uniform location
	GLuint loc_numZTiles;
	GLuint loc_projMatrix;
	GLuint loc_projView;
	GLuint loc_cameraPos;

	int inds[100];
};

