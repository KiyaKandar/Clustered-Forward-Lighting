#pragma once

#include "../nclgl/Maths/Vector2.h"
#include "../nclgl/Maths/Matrix4.h"

//#define DEMO_PRETTY
//#define DEMO_128_LIGHTS
//#define DEMO_256_LIGHTS
//#define DEMO_512_LIGHTS
#define DEMO_1024_LIGHTS
//#define DEMO_2048_LIGHTS

class GLConfig
{
public:

	enum GBUFFER
	{
		GPOSITION,
		GNORMAL,
		GALBEDO
	};

	enum AMBIENT_TEX
	{
		SSAO_INDEX
	};
	
	static const int BLOOM_STRENGTH = 2;

#if defined DEMO_PRETTY
	static const int NUM_X_AXIS_CLUSTERS = 8;
	static const int NUM_Y_AXIS_CLUSTERS = 8;
	static const int NUM_Z_AXIS_CLUSTERS = 8;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = true;
	static const int NUM_LIGHTS = 100;
#elif defined DEMO_128_LIGHTS
	static const int NUM_X_AXIS_CLUSTERS = 4;
	static const int NUM_Y_AXIS_CLUSTERS = 4;
	static const int NUM_Z_AXIS_CLUSTERS = 4;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = false;
	static const int NUM_LIGHTS = 128;
#elif defined DEMO_256_LIGHTS
	static const int NUM_X_AXIS_CLUSTERS = 8;
	static const int NUM_Y_AXIS_CLUSTERS = 8;
	static const int NUM_Z_AXIS_CLUSTERS = 8;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = false;
	static const int NUM_LIGHTS = 256;
#elif defined DEMO_512_LIGHTS
	static const int NUM_X_AXIS_CLUSTERS = 8;
	static const int NUM_Y_AXIS_CLUSTERS = 8;
	static const int NUM_Z_AXIS_CLUSTERS = 8;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = false;
	static const int NUM_LIGHTS = 512;
#elif defined DEMO_1024_LIGHTS
	static const int NUM_X_AXIS_CLUSTERS = 16;
	static const int NUM_Y_AXIS_CLUSTERS = 16;
	static const int NUM_Z_AXIS_CLUSTERS = 16;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = false;
	static const int NUM_LIGHTS = 1024;
#elif defined DEMO_2048_LIGHTS
	static const int NUM_X_AXIS_CLUSTERS = 8;
	static const int NUM_Y_AXIS_CLUSTERS = 8;
	static const int NUM_Z_AXIS_CLUSTERS = 8;
	static const bool FORCE_GLOBAL_LIGHT_ZERO = false;
	static const int NUM_LIGHTS = 2048;
#endif

	static const int NUM_TILES = NUM_X_AXIS_CLUSTERS * NUM_Y_AXIS_CLUSTERS * NUM_Z_AXIS_CLUSTERS;

	//Declared in cpp file - volatile integral type problem.
	static const Vector2 MIN_NDC_COORDS;
	static const Vector2 MAX_NDC_COORDS;
	static const Vector2 RESOLUTION;

	static const float NEAR_PLANE;
	static const float FAR_PLANE;
	static const Matrix4 SHARED_PROJ_MATRIX;
};

