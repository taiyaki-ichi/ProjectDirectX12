
static const int SHADOW_MAP_NUM = 3;
static const int MAX_POINT_LIGHT_NUM = 1000;

struct DirectionLight
{
	float3 dir;
	float3 color;
};

struct PointLight
{
	float3 pos;
	float3 posInView;
	float3 color;
	float range;
};

struct LightData
{
	DirectionLight directionLight;
	matrix directionLightViewProj[SHADOW_MAP_NUM];
	
	PointLight pointLight[MAX_POINT_LIGHT_NUM];
	int pointLightNum;

	float specPow;
};