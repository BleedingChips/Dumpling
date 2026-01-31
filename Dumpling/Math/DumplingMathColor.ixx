module;

export module DumplingMathColor;

import std;
import Potato;
import DumplingMathConcept;
import DumplingMathVector;

export namespace Dumpling::Math
{
	
}


export namespace Dumpling::Color
{
	using ColorRGB = Math::Vector<float, 3>;
	using ColorRGBA = Math::Vector<float, 4>;


	constexpr Math::Vector<float, 4> black_rgba{ 0.0f, 0.0f, 0.0f, 1.0f };
}