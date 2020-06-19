#include "parser_lr0.h"
namespace Potato::Parser::NFA
{
	enum class T
	{
		Char = 0,
		Min, // -
		SquareBracketsLeft, //[
		SquareBracketsRight, // ]
		ParenthesesLeft, //(
		ParenthesesRight, //)
		Mulity, //*
		Question, // ?
		Or, // |
		Add, // +
		Not, // ^
	};

	constexpr Lr0::Symbol operator*(T sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::TerminalT{} }; };

	enum class NT
	{
		Statement,
		ExpressionStatement,
		LeftOrStatement,
		RightOrStatement,
		OrStatement,
		CharList,
		CharListNotable,
		Expression,
		NoGreedAppend,
	};

	constexpr Lr0::Symbol operator*(NT sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::NoTerminalT{} }; };


}
