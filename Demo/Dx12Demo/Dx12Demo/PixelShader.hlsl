 
float v1;
float3 v3;
float4 v4;

struct Data
{
	float v1;
	float3 v3;
	float4 v4;
};

Data da;

float4 main(
	float4 poi : POSITION
) : SV_TARGET
{
	return v1 + float4(v3, 0.0) + float4(v4) +da.v4;
}