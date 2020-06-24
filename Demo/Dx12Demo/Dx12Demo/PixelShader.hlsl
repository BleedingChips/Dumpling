 
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
Texture2DArray<float4> yui[6];
RWTexture2D<float4> RT2[9][10];

float Test(Texture2D<float2> p)
{
	return 0;
}

float4 main(
	float4 poi : POSITION
) : SV_TARGET
{
	int4 Shift = int4(0, 0, 0, 0);
	float4 p = yui[0].mips[0][int3(0, 0, 0)];
	float4 p2 = yui[5].mips[0][int3(0, 0, 0)];
	return float4(a1.GoGo.x + a1.GoGo2.y, 1.0f, 1.0f, 1.0f) + p + p2;
}