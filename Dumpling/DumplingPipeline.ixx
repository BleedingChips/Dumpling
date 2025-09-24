module;


export module DumplingPipeline;

import std;
import Potato;


export namespace Dumpling
{
	struct PassDistributor;


	struct PassScription
	{
		std::u8string_view pass_name;
		Potato::IR::StructLayoutObject::Ptr property;
		Potato::IR::StructLayoutObject::Ptr default_parameter;
	};

	struct PassRequest
	{
		std::optional<std::size_t> order;
		Potato::IR::StructLayoutObject::Ptr property;
		Potato::IR::StructLayoutObject::Ptr parameter;
		std::optional<std::size_t> parameter_meber_view;
		template<typename Type>
		Type* TryGetParameter();
	};

	using PassIndex = Potato::Misc::VersionIndex;

	struct PassSequencer
	{
		struct Element
		{
			PassIndex pass_index;
			Potato::IR::StructLayoutObject::Ptr parameter;
			std::optional<std::size_t> parameter_member_index;
		};
		std::size_t GetRequireCount() const { return elements.size(); }
	protected:
		std::pmr::vector<Element> elements;
		friend struct PassDistributor;
	};

	struct PassDistributor
	{

		PassIndex RegisterPass(PassScription scription);

		PassIndex GetPassIndex(std::u8string_view pass_name) const;
		Potato::IR::StructLayoutObject::ConstPtr GetParameter(PassIndex pass_index) const;
		bool PopRequest(PassIndex pass_index, PassRequest& output, std::size_t offset = 0) const;
		std::size_t SendRequest(PassSequencer const& sequencer);
		void CleanRequest();
		std::optional<Potato::IR::StructLayoutObject::Ptr> CreatePassRequest(std::u8string_view name, std::span<std::u8string_view const> require_pass, PassSequencer& output_sequence, std::pmr::memory_resource* resource = std::pmr::get_default_resource(), std::pmr::memory_resource* temporary_resource = std::pmr::get_default_resource());

	protected:

		struct Info
		{
			std::size_t version;
			PassScription scription;
			Potato::Misc::IndexSpan<> request;
		};

		struct Request
		{
			std::size_t order;
			Potato::IR::StructLayoutObject::Ptr parameter;
			std::optional<std::size_t> member_view;
		};

		std::pmr::vector<Info> infos;
		std::pmr::vector<Request> pass_request;
	};

	template<typename Type>
	Type* PassRequest::TryGetParameter()
	{
		if (parameter)
		{
			if (parameter_meber_view.has_value())
			{
				return parameter->TryGetMemberDataWithStaticCast<Type>(*parameter_meber_view);
			}
			else {
				return parameter->TryGetDataWithStaticCast<Type>();
			}
		}
		return nullptr;
	}
}