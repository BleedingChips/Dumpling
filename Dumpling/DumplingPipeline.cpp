module;

#include <cassert>
module DumplingPipeline;


namespace Dumpling
{
	PassIndex PassDistributor::RegisterPass(PassScription scription)
	{
		infos.emplace_back(1, std::move(scription));
		return { infos.size() - 1, 1 };
	}

	PassIndex PassDistributor::GetPassIndex(std::wstring_view pass_name) const
	{
		for (std::size_t index = 0; index < infos.size(); ++index)
		{
			if (infos[index].scription.pass_name == pass_name)
			{
				return { index, infos[index].version };
			}
		}
		return {};
	}

	Potato::IR::StructLayoutObject::Ptr PassDistributor::CopyParameter(PassIndex pass_index) const
	{
		if (pass_index.index < infos.size())
		{
			auto& ref = infos[pass_index.index];
			if (ref.version == pass_index.version && ref.scription.default_parameter)
			{
				return Potato::IR::StructLayoutObject::CopyConstruct(
					ref.scription.default_parameter->GetStructLayout(),
					*ref.scription.default_parameter
				);
			}
		}
		return {};
	}

	bool PassDistributor::PopRequest(PassIndex pass_index, PassRequest& output, std::size_t offset) const
	{
		if (pass_index.index < infos.size())
		{
			auto& ref = infos[pass_index.index];
			if (ref.version == pass_index.version && offset < ref.request.Size())
			{
				auto& tar = pass_request[ref.request.Begin() + offset];
				output.order = tar.order;
				output.property = ref.scription.property;
				output.parameter = tar.parameter;
				return true;
			}
		}
		return false;
	}

	std::size_t PassDistributor::SendRequest(PassSequencer const& sequencer)
	{
		std::size_t count = 0;
		for (auto& ite : sequencer.elements)
		{
			if (ite.pass_index.index < infos.size())
			{
				auto& ref = infos[ite.pass_index.index];
				if (ref.version == ite.pass_index.version)
				{
					count += 1;
					Request request{ pass_request.size(), ite.parameter };
					pass_request.insert(pass_request.begin() + ref.request.Begin(), std::move(request));
					for (std::size_t i = ite.pass_index.index + 1; i < infos.size(); ++i)
					{
						infos[i].request.WholeOffset(1);
					}
				}
			}
		}
		return count;
	}

	void PassDistributor::CleanRequest()
	{
		pass_request.clear();
		for (auto& ite : infos)
		{
			ite.request = {0, 0};
		}
	}
}