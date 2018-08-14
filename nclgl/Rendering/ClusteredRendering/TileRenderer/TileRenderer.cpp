#include "TileRenderer.h"

#include "../../../Utility/Light.h"
#include "../Game/Utility/Util.h"
#include "../Game/GraphicsConfiguration/GLUtil.h"

const int INTERSECTING = 1;
const int EMPTY = 0;

TileRenderer::TileRenderer(Light** lights, int numLights, int numXTiles, int numYTiles, int numZTiles,
	Vector2 minScreenCoord, Vector2 maxScreenCoord)
{
	this->numLights = numLights;
	this->minCoord = minScreenCoord;
	this->lights = lights;

	//Check number of tiles is greater than 0
	Util::AssertGreaterThan<int>(numXTiles, 0, false, THROW_ERROR);
	Util::AssertGreaterThan<int>(numYTiles, 0, false, THROW_ERROR);
	Util::AssertGreaterThan<int>(numZTiles, 0, false, THROW_ERROR);

	gridSize = Vector3(numXTiles, numYTiles, numZTiles);
	gridDimensions = Vector3(
		std::abs(minScreenCoord.x - maxScreenCoord.x) / static_cast<float>(gridSize.x),
		std::abs(minScreenCoord.y - maxScreenCoord.y) / static_cast<float>(gridSize.y),
		15000.0f / static_cast<float>(gridSize.z));

	numTiles = gridSize.x * gridSize.y * gridSize.z;

	gridPlanes = new CubePlanes[numTiles];

	compute = new ComputeShader(SHADERDIR"/Compute/compute.glsl", true);
	compute->LinkProgram();
	loc_numZTiles = glGetUniformLocation(compute->GetProgram(), "numZTiles");

	dataPrep = new ComputeShader(SHADERDIR"/Compute/dataPrep.glsl", true);
	dataPrep->LinkProgram();
	loc_projMatrix = glGetUniformLocation(dataPrep->GetProgram(), "projectionMatrix");
	loc_projView = glGetUniformLocation(dataPrep->GetProgram(), "projView");
	loc_cameraPos = glGetUniformLocation(dataPrep->GetProgram(), "cameraPos");

	tileData = new TileData();
	GenerateGrid();
	InitGridSSBO();
}

TileRenderer::TileRenderer()
{
	numLights = 0;

	minCoord = Vector2();
	gridSize = Vector3();
	gridDimensions = Vector3();

	numTiles = 0;

	compute = new ComputeShader(SHADERDIR"/Compute/compute.glsl", true);
	compute->LinkProgram();

	dataPrep = new ComputeShader(SHADERDIR"/Compute/dataPrep.glsl", true);
	dataPrep->LinkProgram();

	InitGridSSBO();
}

void TileRenderer::GenerateGrid()
{
	const GridData gridData(grid, gridPlanes, screenTiles, minCoord);
	GridUtility::Generate3DGrid(gridData, gridDimensions, gridSize);
}

void TileRenderer::InitGridSSBO()
{
	gridPlanesSSBO = GLUtil::InitSSBO(1, 4, gridPlanesSSBO,
		sizeof(CubePlanes) * numTiles, gridPlanes, GL_STATIC_COPY);

	screenSpaceDataSSBO = GLUtil::InitSSBO(1, 5, screenSpaceDataSSBO,
		sizeof(ScreenSpaceData), &ssdata, GL_STATIC_COPY);

	glGenBuffers(1, &countBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, countBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, countBuffer);

	//TEMP
	glGenBuffers(1, &intersectionCountBuffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, intersectionCountBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void TileRenderer::AllocateLightsGPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, 
	const Vector3& cameraPos) const
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, countBuffer);
	glInvalidateBufferData(countBuffer);
	GLuint zero = 0;
	glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	//TEMP
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, intersectionCountBuffer);
	glInvalidateBufferData(intersectionCountBuffer);
	GLuint zero1 = 0;
	glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero1);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	PrepareDataGPU(projectionMatrix, viewMatrix, cameraPos);
	FillTilesGPU(projectionMatrix, viewMatrix);
}

void TileRenderer::FillTilesGPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix) const
{
	//Writes to the shared buffer used in lighting pass
	compute->UseProgram();

	glUniform1i(loc_numZTiles, gridSize.z);
	glUniformMatrix4fv(glGetUniformLocation(compute->GetProgram(), "projMatrix"), 1, false, (float*)&projectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(compute->GetProgram(), "viewMatrix"), 1, false, (float*)&viewMatrix);

	compute->Compute(Vector3(1, 1, 1));

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	//////TEMP
	//glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, intersectionCountBuffer);
	//GLuint* ptr = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);

	//if (ptr)
	//{
	//	std::cout << ptr[0] << std::endl;
	//}

	//glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void TileRenderer::PrepareDataGPU(const Matrix4& projectionMatrix, const Matrix4& viewMatrix, 
	const Vector3& cameraPos) const 
{
	Matrix4 projView = projectionMatrix * viewMatrix;
	dataPrep->UseProgram();

	glUniformMatrix4fv(loc_projMatrix, 1, false, (float*)&projectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(dataPrep->GetProgram(), "viewMatrix"), 1, false, (float*)&viewMatrix);
	glUniformMatrix4fv(loc_projView, 1, false, (float*)&projView);

	dataPrep->Compute(**dataPrepWorkGroups);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
}