module;

export module DumplingMathConcept;

import std;
import Potato;

export namespace Dumpling::Math
{
	template<typename Type, typename OType>
	concept ValueAcceptOperatorMul = requires(Type type, OType otype)
	{
		{ type* otype } -> std::convertible_to<Type>;
	};

	template<typename Type, typename OType>
	concept ValueAcceptOperatorAdd = requires(Type type, OType otype)
	{
		{ type + otype } -> std::convertible_to<Type>;
	};

	template<typename Type, typename OType>
	concept ValueAcceptOperatorSub = requires(Type type, OType otype)
	{
		{ type - otype } -> std::convertible_to<Type>;
	};

	template<typename Type, typename OType>
	concept ValueAcceptOperatorDiv = requires(Type type, OType otype)
	{
		{ type / otype } -> std::convertible_to<Type>;
	};
}