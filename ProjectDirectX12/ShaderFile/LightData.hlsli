
//�|�C���g���C���Ƃ̃��C�g�J�����O�̍ۂ̃X���b�h�̐�
//�^�C���̑傫���Ɠ���
#define TILE_WIDTH 16
#define TILE_HEIGHT 16

//�^�C���̑���
#define TILE_NUM (TILE_WIDTH*TILE_HEIGHT)

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