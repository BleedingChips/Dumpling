#include "d3d12shader.h"
import Dumpling;
import std;
using namespace Dumpling;


struct TopHook : public Dumpling::FormEventHook
{
	virtual FormEvent::Respond Hook(FormEvent& event) override
	{
		if (event.IsMessage(FormMessage::DESTORY))
		{
			FormEvent::PostQuitEvent();
		}
		return event.RespondMarkAsSkip();
	}
	virtual void AddFormEventHookRef() const {};
	virtual void SubFormEventHookRef() const {};
} hook;


std::u8string_view shader = u8R"(

struct KKL
{
	float4 i;
};

cbuffer hallo
{
	float Yes = 1.0f;
	float2 iop2[2];
	float4x4 gWorldViewProj;
	float4x3 gWorldViewProj2[2];
	KKL kkl;
	float4 iop = float4(1.0f, 1.0f, 1.0f, 1.0f);
}

cbuffer hallo2
{
	float Yes2 = 1.0f;
}

struct Vertex
{
	float4 position : SV_POSITION;
};

Vertex VSMain()
{
	Vertex vertex;
	vertex.position = float4(0.0f, 0.0f, 0.0f, 1.0f) * Yes * Yes2;
	return vertex;
}

struct Pixel
{
	float3 Color : SV_TARGET0;
};

Pixel PSMain(Vertex vertex)
{
	Pixel pixel;
	pixel.Color = float3(1.0f, 1.0f, 1.0f) * Yes2;
	return pixel;
}

)";



int main()
{

	auto instance = HLSLCompiler::Instance::Create();
	auto compiler = instance.CreateCompiler();

	{
		HLSLCompiler::MaterialShaderOutput shader_output;
		Dx12::ShaderSlot shader_slot;
		bool result = instance.CompileMaterial(
			compiler,
			shader_slot,
			shader_output,
			{
				{shader, u8"VSMain", u8"Test.hlsl"},
				{shader, u8"PSMain", u8"Test.hlsl"}
			},
			{}
		);



		volatile int i = 0;
	}
	



	

	/*
	std::array<Potato::IR::StructLayout::Ptr, 2> output;

	instance.GetConstBufferStructLayoutFromReflection(reflection, output);
	*/


	return 0;
}