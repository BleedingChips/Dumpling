module;


export module DumplingPipeline;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoIR;

export namespace Dumpling
{
	struct PassProperty
	{
		enum class Category
		{
			FRAME,
			COMPUTE
		};
		Category category;
		std::u8string_view name;
		Potato::IR::StructLayout::Ptr struct_layout;
	};

	struct FastIndex
	{
		std::size_t index1;
		std::size_t index2;
	};

	struct Pass : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Pass>;

		static Ptr Create(PassProperty property, FastIndex index, std::pmr::memory_resource* resource);

		std::u8string_view GetName() const { return property.name; }
		PassProperty GetProperty() const { return property; }

	protected:

		Pass(Potato::IR::MemoryResourceRecord record, PassProperty property, FastIndex index)
			: MemoryResourceRecordIntrusiveInterface(record), property(property), index(index) {}

		PassProperty property;
		std::shared_mutex mutex;
		FastIndex index;
	};

	struct Pipeline
	{

		struct Wrapper
		{
			void AddRef(Pipeline const* ptr) const { ptr->AddPipelineRef(); }
			void SubRef(Pipeline const* ptr) const { ptr->SubPipelineRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Pipeline, Wrapper>;

		struct PassInfo
		{
			std::u8string_view pass_name;
			Potato::Misc::IndexSpan<> dependence_index;
		};

		struct Index
		{
			std::size_t pass_info_index;
			std::size_t dependence_index;
		};

		virtual std::span<PassInfo> GetPassInfo() const = 0;
		virtual std::span<std::size_t> GetDependence() const = 0;
		virtual Potato::IR::StructLayout::Ptr GetStruct() const = 0;

	protected:
		virtual void AddPipelineRef() const = 0;
		virtual void SubPipelineRef() const = 0;
	};

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

	struct PipelineInstance : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<PipelineInstance>;

	protected:
		PipelineInstance(Potato::IR::MemoryResourceRecord record) : MemoryResourceRecordIntrusiveInterface(record) {}
	
		friend struct Potato::IR::MemoryResourceRecord;
	};

	struct PipelineManager
	{

		struct PassRequest
		{
			Pass::Ptr pass;
			Pipeline::Ptr pipeline;
			PipelineRequester::Ptr requester;
			Potato::IR::StructLayoutObject::Ptr object;
		};

		PipelineInstance::Ptr CreatPipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		bool ExecutePipeline(PipelineRequester::Ptr requester, PipelineInstance const& pipeline);
		Pass::Ptr RegisterPass(PassProperty pass_property);
		bool UnregisterPass(Pass const&);

		struct MemorySetting
		{
			std::pmr::memory_resource* self_resource = std::pmr::get_default_resource();
			std::pmr::memory_resource* pass_resource = std::pmr::get_default_resource();
		};

		PipelineManager(MemorySetting setting = {});

		std::optional<PassRequest> PopPassRequest(Pass const&);
		bool PushPassRequest(PassRequest pass);

	protected:

		enum class Status
		{
			WAITING,
			RUNNING,
			DONE,
		};

		struct PassTuple
		{
			std::u8string_view pass_name;
			Pass::Ptr pass;
			bool Available = false;
		};

		std::pmr::vector<PassTuple> passes;

		std::mutex request_mutex;

		std::pmr::vector<PassRequest> requests;

		std::pmr::memory_resource* pass_resource = nullptr;
	};

}