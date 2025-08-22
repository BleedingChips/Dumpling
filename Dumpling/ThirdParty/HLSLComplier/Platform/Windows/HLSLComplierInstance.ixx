
module;
#include <wrl/client.h>
#include <d3dcommon.h>

export module DumplingHLSLComplierInstance;

import std;
import Potato;
import DumplingRendererTypes;


export namespace Dumpling::HLSLCompiler
{
	using Microsoft::WRL::ComPtr;

	enum class ShaderTarget
	{
		VS_6_0,
		VS_Lastest,
	};

	enum class ComplierFlag
	{
		None
	};

	struct ShaderComplierArguments
	{
		ShaderComplierArguments(ShaderComplierArguments const&);
		ShaderComplierArguments(ShaderComplierArguments&&);
		ShaderComplierArguments& operator=(ShaderComplierArguments&& arguments);
		ShaderComplierArguments& operator=(ShaderComplierArguments const& arguments);
		ShaderComplierArguments() = default;
		operator bool() const { return argument != nullptr; }
		~ShaderComplierArguments();
	protected:
		void* argument = nullptr;
		friend struct Instance;
	};

	using ShaderBufferPtr = ComPtr<ID3DBlob>;

	struct ShaderComplier
	{
		ShaderComplier(ShaderComplier const&);
		ShaderComplier(ShaderComplier&&);
		ShaderComplier& operator=(ShaderComplier&& in_complier);
		ShaderComplier& operator=(ShaderComplier const& in_complier);
		ShaderComplier() = default;
		operator bool() const { return complier != nullptr; }
		~ShaderComplier();
	protected:
		void* complier = nullptr;
		friend struct Instance;
	};

	struct Instance
	{
		Instance(Instance const&);
		Instance(Instance&&);
		Instance& operator=(Instance&& in_complier);
		Instance& operator=(Instance const& in_complier);
		Instance() = default;
		operator bool() const { return utils != nullptr; }
		~Instance();
		ShaderComplierArguments CreateArguments(ShaderTarget target, wchar_t const* entry_point, wchar_t const* file_path, ComplierFlag flag = ComplierFlag::None);
		ShaderComplier CreateComplier();
		ShaderBufferPtr Complier(ShaderComplier& complier, std::wstring_view code, ShaderComplierArguments const& arguments, Potato::TMP::FunctionRef<void(std::wstring_view)> error = {});
		static Instance Create();
	protected:
		void* utils = nullptr;
	};
}
