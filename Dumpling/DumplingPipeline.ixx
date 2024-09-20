module;


export module DumplingPipeline;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoIR;

export namespace Dumpling
{
	struct PipelineRequester
	{
		struct Wrapper
		{
			void AddRef(PipelineRequester const* ptr) const { ptr->AddRendererRequesterRef(); }
			void SubRef(PipelineRequester const* ptr) const { ptr->SubRendererRequesterRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<PipelineRequester, Wrapper>;

	protected:

		virtual void AddRendererRequesterRef() const = 0;
		virtual void SubRendererRequesterRef() const = 0;
	};

	struct PassProperty
	{
		enum class Category
		{
			FRAME,
			COMPUTE
		};
		Category category;
		Potato::IR::StructLayout::Ptr struct_layout;
	};

	struct PassIndex
	{
		std::size_t index = std::numeric_limits<std::size_t>::max();
		operator bool() const { return index != std::numeric_limits<std::size_t>::max(); }
	};

	struct Pipeline
	{

		struct Wrapper
		{
			void AddRef(Pipeline const* ptr) const { ptr->AddPipelineRef(); }
			void SubRef(Pipeline const* ptr) const { ptr->SubPipelineRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Pipeline, Wrapper>;
		using PtrConst = Potato::Pointer::IntrusivePtr<Pipeline const, Wrapper>;

		struct ValueMapping
		{
			std::size_t index;
			std::u8string_view mapping_to;
		};

		struct RequirePass
		{
			std::u8string_view pass_name;
			Potato::Misc::IndexSpan<> direct_to;
			Potato::Misc::IndexSpan<> mapping_value;
		};

		struct Require
		{
			std::span<RequirePass> require_pass;
			std::span<std::size_t> direct_to;
			std::span<ValueMapping> mapping_value;
		};

		virtual Require GetRequire() const = 0;
		virtual Potato::IR::StructLayout::Ptr GetStructLayout() const { return {}; }
		virtual void Execute(Potato::IR::StructLayoutObject& executor) const {}

	protected:

		virtual void AddPipelineRef() const = 0;
		virtual void SubPipelineRef() const = 0;
	};

	struct PipelineInstance
	{
		struct Wrapper
		{
			void AddRef(PipelineInstance const* ptr) { ptr->AddPipelineInstanceRef(); }
			void SubRef(PipelineInstance const* ptr) { ptr->SubPipelineInstanceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<PipelineInstance, Wrapper>;

		struct ValueMapping
		{
			std::size_t source;
			std::size_t target;
		};

		struct PassReference
		{
			PassIndex index;
			Potato::Misc::IndexSpan<> direct_to;
			Potato::Misc::IndexSpan<> value_mapping;
			std::size_t indegree;
		};

		struct Require
		{
			std::span<PassReference> require_index;
			std::span<std::size_t> direct_to;
			std::span<ValueMapping> value_mapping;
		};

		Require GetRequiresReference() const { return {require_index, direct_to, value_mapping}; }

	protected:

		PipelineInstance(
			std::span<PassReference> require_index,
			std::span<std::size_t> direct_to,
			std::span<ValueMapping> value_mapping,
			Potato::IR::StructLayoutObject::Ptr struct_object
		) : require_index(require_index), direct_to(direct_to), value_mapping(value_mapping), struct_object(std::move(struct_object))
		{
			
		}

		virtual void AddPipelineInstanceRef() const = 0;
		virtual void SubPipelineInstanceRef() const = 0;
	
		std::span<PassReference> require_index;
		std::span<std::size_t> direct_to;
		std::span<ValueMapping> value_mapping;
		Potato::IR::StructLayoutObject::Ptr struct_object;
	};

	struct PassTable
	{

		struct Element
		{
			Potato::Misc::IndexSpan<> pass_name;
			PassProperty property;
		};

		PassIndex LocatePass(std::u8string_view name) const;
		PassIndex RegisterPass(std::u8string_view name, PassProperty pass_property);
		PipelineInstance::Ptr CreatePipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) const;

		std::size_t GetPassSize() const { return passes.size(); }

		PassTable(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: passes(resource), pass_name(resource) {}

	protected:

		std::pmr::vector<Element> passes;
		std::pmr::u8string pass_name;
	};

	struct PipelineRecorder
	{

		enum class State
		{
			Ready,
			Running,
			Done,
		};

		struct Request
		{
			PassIndex pass_index;
			PipelineRequester::Ptr requester;
			Potato::IR::StructLayoutObject::Ptr object;
			Potato::Misc::IndexSpan<> direct_to;
			State state = State::Ready;
			std::size_t indegree = 0;
		};

		PipelineRecorder(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: requests(resource), direct_to(resource), frame_count(resource) {}

		bool CommitPipeline(PassTable const& table, PipelineInstance const& pipeline, PipelineRequester::Ptr, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		struct PassRequest
		{
			PassIndex pass_index;
			PipelineRequester::Ptr requester;
			Potato::IR::StructLayoutObject::Ptr object;
			std::size_t reference_index;
		};

		struct RequestState
		{
			std::size_t running_count = 0;
			std::size_t finish_count = 0;
			std::size_t total_count = 0;
		};

		std::tuple<std::size_t, RequestState> PopRequest(std::span<PassRequest> output);
		RequestState FinishRequest(PassRequest& request);

		bool PushFrame();
		bool PopFrame();

	protected:

		struct FrameCount
		{
			std::size_t request_count = 0;
			std::size_t direct_to_count = 0;
		};

		FrameCount writing_frame;
		FrameCount reading_frame;

		RequestState current_state;

		std::pmr::vector<Request> requests;
		std::pmr::vector<std::size_t> direct_to;
		std::pmr::deque<FrameCount> frame_count;
	};

	/*
	export struct PipelineManager
	{

		

		PipelineInstance::Ptr CreatePipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		
		//bool CommitPipeline(PipelineRequester::Ptr requester, PipelineInstance const& pipeline);
		PassIndex RegisterPass(std::u8string_view name, PassProperty pass_property);

		struct MemorySetting
		{
			std::pmr::memory_resource* self_resource = std::pmr::get_default_resource();
			std::pmr::memory_resource* pass_resource = std::pmr::get_default_resource();
		};

		PipelineManager(MemorySetting setting = {});

		std::size_t FinishAndPopPass(PassIndex finished_index, std::span<PassRequest> request);

	protected:

		PassIndex RegisterPass_AssumedLocked(std::u8string_view name, PassProperty pass_property);

		enum class Status
		{
			WAITING,
			RUNNING,
			DONE,
		};

		

		std::shared_mutex pass_mutex;
		

		std::mutex request_mutex;
		

		std::pmr::memory_resource* pass_resource = nullptr;
	};
	*/

}