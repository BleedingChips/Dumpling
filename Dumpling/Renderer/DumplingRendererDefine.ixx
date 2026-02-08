module;

export module DumplingRendererDefine;

export namespace Dumpling::Renderer
{
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
}