#pragma once
#include <cstdint>
#include <xstring>

namespace SurgerCane
{
	using Float = float;

	struct Float2
	{
		union
		{
			struct
			{
				float storage[2];
			};

			struct
			{
				float X, Y;
			};

			struct
			{
				float R, G;
			};
		};
		
	};

	struct Float3
	{
		float X,Y,Z;
	};

	struct Float4
	{
		float X,Y,Z,W;
	};

	using Int = int32_t;

	struct Int2
	{
		int32_t X = 0, Y = 0;
	};

	struct Int3
	{
		int32_t X = 0, Y = 0, Z = 0;
	};

	struct Int4
	{
		int32_t x = 0, y = 0, z = 0, w = 0;
	};

	struct Texture
	{
		std::u32string_view path;
	};

	struct Sampler
	{
		std::u32string_view style;
	};
	
}
