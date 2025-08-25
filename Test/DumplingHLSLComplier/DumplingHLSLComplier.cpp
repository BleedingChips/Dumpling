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


std::wstring_view shader = LR"(
cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
}

void main(float3 iPoL : POSITION, float4 iColor : COLOR,
	out float4 oPosH : SV_POSITION,
	out float4 oColor : COLOR
)
{
sdasdasd
	oPosH = mul(float4(iPoL, 1.0f), gWorldViewProj);
	oColor = iColor;
}
)";



int main()
{
	auto instance = HLSLCompiler::Instance::Create();
	auto code = instance.EncodeShader(shader);
	auto argues = instance.CreateArguments(HLSLCompiler::ShaderTarget::VS_Lastest, L"main", L"Funck.ixx");
	auto compiler = instance.CreateCompiler();
	auto result = instance.Compile(compiler, code, argues);
	auto error_messahe = instance.GetErrorMessage(result, [](std::u8string_view str_view) {
		volatile int oi = 0;
	});

	auto shader_object = instance.GetShaderObject(result);
	auto reflection = instance.GetReflection(shader_object);
	if (reflection)
	{
		D3D12_SHADER_DESC desc;
		reflection->GetDesc(
			&desc
		);

		volatile int i = 0;
	}
	return 0;
}