matrix world : register(b0);

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(world,pos);
}