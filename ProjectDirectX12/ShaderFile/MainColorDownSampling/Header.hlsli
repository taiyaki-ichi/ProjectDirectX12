
//1�傫���T�C�Y�̃��C���J���[�̃e�N�X�`��
Texture2D<float4> mainColorTexture: register(t0);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};