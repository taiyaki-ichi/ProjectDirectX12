
struct ModelData
{
	matrix world;
	matrix view;
	matrix proj;
};

ModelData modelData : register(b0);

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(mul(modelData.proj, modelData.view), mul(modelData.world,pos));
}