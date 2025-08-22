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
cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
}

void main(float3 iPoL : POSITION, float4 iColor : COLOR,
	out float4 oPosH : SV_POSITION,
	out float4 oColor : COLOR
)
{
	oPosH = mul(float4(iPoL, 1.0f), gWorldViewProj);
	oColor = iColor;
}
)";



int main()
{
	auto instance = HLSLCompiler::Instance::Create();

	//instance->Compile(shader, {});
	

	return 0;
}