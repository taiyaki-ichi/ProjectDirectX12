
cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
};

matrix world : register(b1);

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(mul(proj, view), mul(world,pos));
}