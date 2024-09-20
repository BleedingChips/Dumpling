#include "d3d12.h"
#include "dxgi1_6.h"



import std;
import Potato;
import DumplingPipeline;

using namespace Dumpling;

struct PipeI : public Dumpling::Pipeline
{

	Require GetRequire() const override
	{
		static auto require = std::array{
			RequirePass{u8"Func1", {0, 1}, {}},
			RequirePass{u8"Func2", {1, 1}, {}},
		};
		static auto direct = std::array{
			std::size_t{1}
		};
		return {
			std::span(require),
			std::span(direct),
			{}
		};
	}

	virtual void AddPipelineRef() const override {}
	virtual void SubPipelineRef() const override {}


	PipeI()  {}
};

int main()
{
	PipeI pipe;

	Dumpling::PassTable table;
	auto in1 = table.CreatePipelineInstance(pipe);
	auto p2 = table.RegisterPass(u8"Func2", {});

	auto in2 = table.CreatePipelineInstance(pipe);

	auto p1 = table.RegisterPass(u8"Func1", {});

	auto in3 = table.CreatePipelineInstance(pipe);

	Dumpling::PipelineRecorder recorder;

	recorder.CommitPipeline(table, *in3, {});
	recorder.CommitPipeline(table, *in3, {});

	recorder.PushFrame();

	recorder.CommitPipeline(table, *in3, {});
	recorder.CommitPipeline(table, *in3, {});

	recorder.PushFrame();

	recorder.PopFrame();

	std::array<Dumpling::PipelineRecorder::PassRequest, 10> storage;

	while(true)
	{
		bool Done = false;
		auto [count, state] = recorder.PopRequest(std::span(storage));

		if(state.running_count == 0 && state.finish_count == state.total_count)
		{
			Done = true;
		}
		for(std::size_t i = 0; i < count; ++i)
		{
			auto k = recorder.FinishRequest(storage[i]);
			volatile int i2 = 0;
		}
		if(Done)
			break;
	}

	recorder.PopFrame();

	return 0;
}