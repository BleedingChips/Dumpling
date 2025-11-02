#include <imgui.h>

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

struct DemoWeight : public IGWidget
{
	virtual void Draw(PassRenderer& render) override
	{
		ImGui::ShowDemoWindow();
	}

protected:

	virtual void AddIGWidgetRef() const {}
	virtual void SubIGWidgetRef() const {}
}demo;

int main()
{
	Device::InitDebugLayer();

	auto device = Device::Create();

	Form::Config config;
	config.title = u8"FuckYou";
	config.event_hook = &hook;

	auto form = Form::Create(config);

	auto form_renderer = device->CreateFrameRenderer();

	auto output = device->CreateFormWrapper(form, *form_renderer);

	

	auto hud = IGHeadUpDisplay::Create(form, *form_renderer, &demo);

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	bool need_loop = true;
	while(need_loop)
	{

		while (true)
		{
			auto re = Form::PeekMessageEventOnce([](FormEvent& event) ->FormEvent::Respond {
				return IGHeadUpDisplay::FormEventHook(event);
				});
			if (re.has_value())
			{
				if (!*re)
					break;
			}else
			{
				need_loop = false;
			}
		}

		PassRenderer ren;
		PassRequest request;

		if (form_renderer->PopPassRenderer(ren, request))
		{
			RenderTargetSet sets;
			sets.AddRenderTarget(*output);
			ren.SetRenderTargets(sets);
			ren.ClearRendererTarget(0, { R, G, B, 1.0f });
			hud->Draw(ren);
			form_renderer->FinishPassRenderer(ren);
		}
		auto tar = form_renderer->CommitFrame();
		form_renderer->FlushToLastFrame();

		output->LogicPresent();
		output->Present();
		

		R += 0.03f;
		G += 0.06f;
		B += 0.09f;

		if(R >= 1.0f)
			R -= 1.0f;
		if(G >= 1.0f)
			G -= 1.0f;
		if(B >= 1.0f)
			B -= 1.0f;
	}
	

	return 0;
}