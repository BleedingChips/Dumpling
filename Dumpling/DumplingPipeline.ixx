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
		std::optional<std::size_t> parameter_meber_view;
		template<typename Type>
		Type* TryGetParameter()
		{
			if (parameter)
			{
				if (parameter_meber_view.has_value())
				{
					if (auto mv = parameter->GetStructLayout()->FindMemberView(*parameter_meber_view); mv.has_value())
					{
						if (mv->struct_layout->IsStatic<Type>())
						{
							return parameter->GetStaticCastMemberData<Type>(*mv);
						}
					}
				}
				else {
					if (parameter->GetStructLayout()->IsStatic<Type>())
					{
						return parameter->GetStaticCastData<Type>();
					}
				}
			}
			return nullptr;
		}
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

		std::pmr::vector<Element> elements;
	};

	struct PassDistributor
	{

		PassIndex RegisterPass(PassScription scription);

		PassIndex GetPassIndex(std::wstring_view pass_name) const;
		Potato::IR::StructLayoutObject::ConstPtr GetParameter(PassIndex pass_index) const;
		bool PopRequest(PassIndex pass_index, PassRequest& output, std::size_t offset = 0) const;
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