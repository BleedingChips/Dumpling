
float3 Yui;
float2 oi2 : TEXTURE2;

float4 main(
	float4 poi : POSITION
	) : SV_TARGET
{
	return float4(poi.x, 1.0f, 1.0f, 1.0f);
}