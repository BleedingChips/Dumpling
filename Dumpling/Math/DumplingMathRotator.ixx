module;

export module DumplingMathRotator;

import std;
import Potato;
import DumplingMathConcept;

export namespace Dumpling::Math
{
	template<typename Type, std::size_t Dimension>
	requires(
			Potato::TMP::IsOneOf<Type, float, double>::Value
			&& (Dimension == 2 || Dimension == 3)
		)
	struct Rotator : std::array<Type, Dimension == 2 ? 1 : 3>
	{
	};
}

export namespace Dumpling
{
}