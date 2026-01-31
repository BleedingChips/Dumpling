module;

export module DumplingMathTransform;

import std;
import Potato;
import DumplingMathConcept;
import DumplingMathVector;
import DumplingMathRotator;

export namespace Dumpling::Math
{
	template<typename Type, std::size_t Dimension>
		requires(
			Potato::TMP::IsOneOf<Type, float, double>::Value
			&& (Dimension == 2 || Dimension == 3)
		)
	struct Transform
	{
		Vector<Type, Dimension> position;
		Rotator<Type, Dimension> rotation;
		Vector<Type, Dimension> scale;
	};
}

export namespace Dumpling
{
}