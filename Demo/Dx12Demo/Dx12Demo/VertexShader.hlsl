

RWTexture2D<float2> Write;

struct AS
{
	
};

cbuffer TUI
{
	
};

AS as;

namespace POI
{
	cbuffer Prt
	{
		float2 Poi : POSITION;
};
	Texture2D<float4> Write2;
}

float4 main(float2 uv : TEXTURE, float2 PC : TEXTURE2) : SV_POSITION
{
	return float4(POI::Poi, 0.0, 0.0);

}