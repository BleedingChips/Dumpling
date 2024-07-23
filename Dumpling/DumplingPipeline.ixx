module;


export module DumplingPipeline;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;
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

	struct Pipeline : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Pipeline>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		//virtual Potato::IR::StructLayout::Ptr GetStructLayout() const = 0;
		Pipeline(Potato::IR::MemoryResourceRecord record)
			: MemoryResourceRecordIntrusiveInterface(record)
		{
			
		}
	protected:

	};

	struct PipelineRequester
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererRequesterRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererRequesterRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<PipelineRequester, Wrapper>;

	protected:

		virtual void AddRendererRequesterRef() const = 0;
		virtual void SubRendererRequesterRef() const = 0;
	};

}