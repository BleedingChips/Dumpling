module;

export module DumplingRendererPresets;

import std;
import Potato;
import DumplingRendererDefine;

export namespace Dumpling::Renderer
{
	using Potato::IR::StructLayout;
	using Potato::IR::StructLayoutObject;

	struct PresetGeometry
	{
		Potato::IR::StructLayoutObject::Ptr vertex_data;
		Potato::IR::StructLayoutObject::Ptr index_data;
		PrimitiveTopology topology = PrimitiveTopology::TRIANGLE;
		std::size_t vertex_count = 0;

		static PresetGeometry GetTriangle();
	};

	struct PresetTexture
	{
		Potato::IR::StructLayoutObject::Ptr vertex_data;
	};

}