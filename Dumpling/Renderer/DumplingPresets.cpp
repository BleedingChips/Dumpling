module;

module DumplingRendererPresets;
import DumplingMathVector;



namespace Dumpling::Renderer
{

	StructLayout::Ptr CreateDefaultPresetVertexLayout()
	{
		static auto vertex_layout = Potato::IR::StructLayout::CreateDynamic(
			u8"preset_default_vertex_layout",
			std::initializer_list<StructLayout::Member>{
				{
					Potato::IR::StructLayout::GetStatic<Math::Float3>(),
					u8"POSITION"
				},
				{
					Potato::IR::StructLayout::GetStatic<Math::Float3>(),
					u8"COLOR"
				},
				{
					Potato::IR::StructLayout::GetStatic<Math::Float3>(),
					u8"NORMAL"
				},
				{
					Potato::IR::StructLayout::GetStatic<Math::Float2>(),
					u8"TEXTURE"
				}
			}
		);
		return vertex_layout;
	}

	StructLayout::Ptr CreateDefaultPresetIndexLayout()
	{
		return Potato::IR::StructLayout::GetStatic<std::uint32_t>();
	}

	struct NativeDefaultPresetVertex
	{
		Math::Float3 position;
		Math::Float3 color;
		Math::Float3 normal;
		Math::Float2 texture;
	};

	

	PresetGeometry PresetGeometry::GetTriangle()
	{
		static std::array<NativeDefaultPresetVertex, 3> vertex = {
			NativeDefaultPresetVertex{
				{-1.0f, -1.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{0.0f, 0.0f, -1.0f},
				{0.0f, 0.0f},
			},
			NativeDefaultPresetVertex{
				{-1.0f, 1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{0.0f, 0.0f, -1.0f},
				{0.0f, 1.0f},
			},
			NativeDefaultPresetVertex{
				{1.0f, -1.0f, 0.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, -1.0f},
				{1.0f, 1.0f},
			},
		};

		static std::array<std::uint32_t, 3> index = { 0, 1, 2 };

		return {
				Potato::IR::StructLayoutObject::CopyConstruct(
					CreateDefaultPresetVertexLayout(),
					3,
					vertex.data(),
					Potato::MemLayout::ArrayLayout{ 3, sizeof(NativeDefaultPresetVertex), sizeof(NativeDefaultPresetVertex) }
				),
				Potato::IR::StructLayoutObject::CopyConstruct(
					CreateDefaultPresetIndexLayout(),
					3,
					index.data(),
					Potato::MemLayout::ArrayLayout{3, sizeof(std::uint32_t), sizeof(std::uint32_t)}
				),
				PrimitiveTopology::TRIANGLE,
				3
		};
	}
}
