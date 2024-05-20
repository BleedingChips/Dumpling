import std;
import PotatoTaskSystem;

import Dumpling;
import Noodles;
import std;

import DumplingImGui;

using namespace Dumpling;

int main()
{

	{
		Sub S1;
		Potato::Task::TaskContext context;
		Noodles::Context noodles_context;

		auto collector = noodles_context.CreateSingleton<FormEventCollector>();

		auto man = Dumpling::CreateManager();
		collector->manager = man;

		noodles_context.CreateTickSystemAuto(
			{}, {u8"event_responder", u8"event"},
			[](Noodles::ExecuteContext& cont, Noodles::SingletonFilter<FormEventCollector>& filter)
			{
				auto ref = filter.Get(cont);
				if(ref)
				{
					std::lock_guard lg(ref->mutex);
					for(auto& ite : ref->events)
					{
						if(ite.events.message == FormEventEnum::DESTORYED)
						{
							ref->manager->CloseMessageLoop();
							cont.noodles_context.Quit();
						}
					}
					ref->events.clear();
				}
			}
		);

		noodles_context.FlushStats();

		auto dv = Dumpling::CreateRendererHardDevice();

		auto render = dv->CreateRenderer();

		auto form_rt = render->CreateFormRenderTarget(std::nullopt);

		man->Commite(context, std::this_thread::get_id(), {});

		auto form = man->CreateForm({}, collector.GetPointer(), form_rt);
		noodles_context.Commit(context);
		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}