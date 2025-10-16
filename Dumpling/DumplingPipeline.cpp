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

	PassIndex PassDistributor::GetPassIndex(std::u8string_view pass_name) const
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

	Potato::IR::StructLayoutObject::ConstPtr PassDistributor::GetParameter(PassIndex pass_index) const
	{
		if (pass_index.index < infos.size())
		{
			auto& ref = infos[pass_index.index];
			if (ref.version == pass_index.version && ref.scription.default_parameter)
			{
				return ref.scription.default_parameter;
			}
		}
		return {};
	}

	std::optional<Potato::IR::StructLayoutObject::Ptr> PassDistributor::CreatePassRequest(std::u8string_view name, std::span<std::u8string_view const> require_pass, PassSequencer& output_sequence, std::pmr::memory_resource* resource, std::pmr::memory_resource* temporary_resource)
	{
		std::size_t old_require_size = output_sequence.elements.size();
		if (require_pass.size() >= 1)
		{
			std::pmr::vector<Potato::IR::StructLayout::Member> members{ temporary_resource };
			std::pmr::vector<Potato::IR::StructLayout::CustomConstruct> member_construct{ temporary_resource };
			members.reserve(require_pass.size());
			member_construct.reserve(require_pass.size());
			std::size_t index = 0;
			for (auto pass_name : require_pass)
			{
				auto pass_index = GetPassIndex(pass_name);
				if (pass_index)
				{
					auto parameter = GetParameter(pass_index);
					if (parameter)
					{
						output_sequence.elements.emplace_back(PassSequencer::Element{ pass_index, {}, index });
						Potato::IR::StructLayout::Member new_meber;
						new_meber.name = pass_name;
						new_meber.struct_layout = parameter->GetStructLayout();
						new_meber.array_count = 0;
						members.emplace_back(std::move(new_meber));
						Potato::IR::StructLayout::CustomConstruct new_construct;
						new_construct.construct_operator = decltype(new_construct.construct_operator)::Copy;
						new_construct.paramter_object.construct_parameter_const_object = parameter->GetBuffer();
						member_construct.emplace_back(std::move(new_construct));
						++index;
					}
					else {
						output_sequence.elements.emplace_back(PassSequencer::Element{ pass_index, {}, std::nullopt });
					}
				}
				else {
					output_sequence.elements.resize(old_require_size);
					return std::nullopt;
				}
			}

			auto new_struct_layout = Potato::IR::StructLayout::CreateDynamic(name, std::span(members.data(), members.size()), {}, resource);

			if (new_struct_layout)
			{
				auto new_parameter = Potato::IR::StructLayoutObject::CustomConstruction(
					std::move(new_struct_layout),
					std::span(member_construct.data(), member_construct.size()),
					resource
				);

				if (new_parameter)
				{
					auto new_add = std::span(output_sequence.elements.data(), output_sequence.elements.size()).subspan(old_require_size);
					for (auto& ite : new_add)
					{
						if (ite.parameter_member_index.has_value())
						{
							ite.parameter = new_parameter;
						}
					}
					return new_parameter;
				}
			}
		}
		output_sequence.elements.resize(old_require_size);
		return std::nullopt;
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
				output.parameter_meber_view = tar.member_view;
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
					Request request{ pass_request.size(), ite.parameter, ite.parameter_member_index };
					pass_request.insert(pass_request.begin() + ref.request.Begin(), std::move(request));
					ref.request.BackwardEnd(1);
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