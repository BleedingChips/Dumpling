import std;
import PotatoTaskSystem;

import Dumpling;
import Noodles;
import std;

using namespace Dumpling;


struct Sub : public FormEventResponder
{
	virtual void AddFormEventResponderRef() const override {}
	virtual void SubFormEventResponderRef() const override {}
	FormManager::Ptr manager;
	std::optional<FormEventRespond> Respond(FormInterface const& interface, FormEvent event) override
	{
		if(event.message == FormEventEnum::DESTORYED)
		{
			if(manager)
			{
				manager->CloseMessageLoop();
			}
		}
		return {};
	}
};


int main()
{

	{
		Sub S1;
		Potato::Task::TaskContext context;
		Noodles::Context noodles_context;

		auto man = Dumpling::CreateManager();

		auto dv = Dumpling::CreateRendererHardDevice();

		auto render = dv->CreateRenderer();

		S1.manager = man;

		auto form_rt = render->CreateFormRenderTarget(std::nullopt);

		man->Commite(context, std::this_thread::get_id(), {});

		auto form = man->CreateForm({}, &S1, form_rt);

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}