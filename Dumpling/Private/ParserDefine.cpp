#include <array>
#include <tuple>
#include "../Public/ParserDefine.h"
namespace Dumpling::Parser
{
	
	std::map<BuildInType, BuildInTypeProperty> const& BuildInTypeMapping() {
		static std::map<BuildInType, BuildInTypeProperty> Instance{
			{BuildInType::Float, {U"float", true, {4, 4}}},
			{BuildInType::Float2, {U"float2", true, {4, 8}}},
			{BuildInType::Float3, {U"float3", true, {4, 12}}},
			{BuildInType::Float4, {U"float4", true, {4, 16}}},
			{BuildInType::Int, {U"int", true, {4, 4}}},
			{BuildInType::Int2, {U"int2", true, {4, 8}}},
			{BuildInType::Int3, {U"int3", true, {4, 12}}},
			{BuildInType::Int4, {U"int4", true, {4, 16}}},
			{BuildInType::Uint, {U"uint", true, {4, 4}}},
			{BuildInType::Uint2, {U"uint2", true, {4, 8}}},
			{BuildInType::Uint3, {U"uint3", true, {4, 12}}},
			{BuildInType::Uint4, {U"uint4", true, {4, 16}}},
			{BuildInType::Bool, {U"bool", true, {4, 4}}},
			{BuildInType::Matrix, {U"float4x4", true, {4, 64}}},
			{BuildInType::Tex1, {U"Texture1D", false, {4, 16}}},
			{BuildInType::Tex2, {U"Texture2D", false, {4, 16}}},
			{BuildInType::Tex3, {U"Texture3D", false, {4, 16}}},
			{BuildInType::Sampler, {U"SamplerState", false, {4, 8}}},
			{BuildInType::String, {U"String", false, {4, 8}}},
		};
		return Instance;
	}

	std::optional<BuildInTypeProperty> GetBuildInTypeProperty(BuildInType input)
	{
		auto& ref = BuildInTypeMapping();
		auto re = ref.find(input);
		if (re != ref.end())
			return re->second;
		else
			return std::nullopt;
	}

	std::optional<BuildInType> GetBuildInType(std::u32string_view input)
	{
		for (auto& ite : BuildInTypeMapping())
		{
			if(ite.second.name == input)
				return ite.first;
		}
		return std::nullopt;
	}
}