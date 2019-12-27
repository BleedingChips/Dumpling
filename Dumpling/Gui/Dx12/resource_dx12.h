#pragma once
#include <d3d12.h>
#include "define_dx12.h"
namespace Dumpling::Dx12
{
	enum class ResourceType
	{
		ConstBuffer,
		StructureBuffer,
		Texture1,
		Texture1A,
		Texture2,
		Texture2A,
		Texture2MS,
		Texture2AMS,
		Texture3,
		TextureCube,
	};

	struct ResourceMapping {
		void AddRef() const noexcept;
		void Release() const noexcept;
	private:
		mutable Potato::Tool::atomic_reference_count m_Ref;
		ResourceMapping();
	};
}