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
asdasdasd
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
	auto error_messahe = instance.GetErrorMessage(result);
	instance.CastToWCharString(error_messahe, [](std::wstring_view error) {
		volatile int o = 0;
		});

	return 0;
}