 
struct Wtf {
	float2 GoGo;
	float2 GoGo2;
	float4 Go234;
	
};

cbuffer WTF
{
	Wtf a1;
	Wtf a2;
	
};

Texture2D G122;

float4 main(
	float4 poi : POSITION
	) : SV_TARGET
{
	return float4(a1.GoGo.x, 1.0f, 1.0f, 1.0f);
}