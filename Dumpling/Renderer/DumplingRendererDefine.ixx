module;

export module DumplingRendererDefine;
import Potato;

export namespace Dumpling::Renderer
{
	using Potato::IR::StructLayout;
	using Potato::IR::StructLayoutObject;

	enum class ShaderType
	{
		VS,
		PS,
	};

	enum class ShaderResourceType
	{
		CONST_BUFFER,
		TEXTURE,
		UNORDER_ACCED,
		SAMPLER,
		UNKNOW
	};

	enum class PrimitiveTopology
	{
		TRIANGLE,
	};

	struct Primitive
	{
		StructLayoutObject::Ptr vertex;
		StructLayoutObject::Ptr index;
		PrimitiveTopology topology;
	};
}