cbuffer UserDefine  {
	float4 PositionOffset;
	float4 ColorOffset;
};

Texture2D<float4> text;
sampler text_sampler;

struct Vertex
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE;
};

struct InputVertex
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 UV : TEXTURE;
};

void Function(float4 V1, float2 UV, out float4 V2, out float4 V3)
{
    V2 = V1;
    V3 = V1 + text.Sample(text_sampler, UV);
}

Vertex VSMain(InputVertex in_vertex)
{
	Vertex vertex;
	vertex.position = float4(in_vertex.Position.x, in_vertex.Position.y, in_vertex.Position.z, 1.0f) + PositionOffset;
	vertex.color = float4(in_vertex.Color.x, in_vertex.Color.y, in_vertex.Color.z, 1.0f);
	vertex.uv = in_vertex.UV;
	return vertex;
};

struct Pixel
{
	float4 Color : SV_TARGET0;
};

Pixel PSMain(Vertex vertex)
{
	Pixel pixel;
    float4 Color1;
    float4 Color2;
    Function(vertex.color, vertex.uv, Color1, Color2);
	pixel.Color = Color1 + vertex.color;
	return pixel;
};