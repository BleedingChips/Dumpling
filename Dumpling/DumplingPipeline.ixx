module;


export module DumplingPipeline;

import std;
import Potato;


export namespace Dumpling
{
	struct PassScription
	{
		std::wstring_view pass_name;
		Potato::IR::StructLayoutObject::Ptr property;
		Potato::IR::StructLayoutObject::Ptr default_parameter;
	};

	struct PassRequest
	{
		std::optional<std::size_t> order;
		Potato::IR::StructLayoutObject::Ptr property;
		Potato::IR::StructLayoutObject::Ptr parameter;
	};

	using PassIndex = Potato::Misc::VersionIndex;

	struct PassSequencer
	{
		struct Element
		{
			PassIndex pass_index;
			Potato::IR::StructLayoutObject::Ptr parameter;
		};

		std::pmr::vector<Element> elements;
	};

	struct PassDistributor
	{

		PassIndex RegisterPass(PassScription scription);

		PassIndex GetPassIndex(std::wstring_view pass_name);
		Potato::IR::StructLayoutObject::Ptr CopyParameter(PassIndex pass_index) const;
		std::optional<std::size_t> PopRequest(PassIndex pass_index, std::span<PassRequest> output, std::size_t offset = 0) const;
		std::size_t SendRequest(PassSequencer const& sequencer);
		void CleanRequest();

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
		};

		std::pmr::vector<Info> infos;
		std::pmr::vector<Request> pass_request;
	};
}