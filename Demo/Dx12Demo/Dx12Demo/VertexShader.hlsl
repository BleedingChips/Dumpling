float2 Poi : POSITION;

RWTexture2D<float2> Write;

float4 main(float2 uv : TEXTURE, float2 PC : TEXTURE2) : SV_POSITION
{
	return float4(Poi, 0.0, 0.0);
}