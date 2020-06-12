#include "parser_lr0.h"
#include "parser_nfa.h"

using namespace Potato::Parser::Lr0;
using namespace Potato;


enum class T
{
	Num = 0,
	Add,
	Mulity,
	A,
	B,
	C,
	D,
};

Symbol operator*(T token) { return Symbol(static_cast<size_t>(token), terminal); }

enum class NT
{
	Expression,
	StepA,
	StepB,
	StepC,
};

Symbol operator*(NT token) { return Symbol(static_cast<size_t>(token), noterminal); }

std::u32string_view Ptr[] = {
	U"[a-zA-Z-][0-9]",
	U"while",
	U"\\s"
};


int main()
{
	auto ref = CreateTable(
		*NT::Expression,
		{
			{{*NT::Expression, *T::Num}, 0},
			//{{*NT::Expression, *NT::Expression, *T::Add, *NT::Expression,}, 1},
			//{{*NT::Expression, *NT::Expression, *T::Mulity, *NT::Expression,}, 2},
			{{*NT::Expression, *NT::Expression, *T::A}, 3},
			{{*NT::Expression, *NT::Expression, *T::A, *T::B,}, 4},
			{{*NT::Expression, *NT::Expression, *T::B}, 5},
		}, { {{*T::Mulity}, Associativity::Right}, {{*T::Add}} }
	);

	Symbol List[] = { *T::Num, *T::A, *T::B };

	float NumberList[] = { 1.0, 0.0f, 2.0, 0.0, 3.0, 0.0, 4.0 };

	auto His = Processer{}(ref, List, 3);

	auto Table = Parser::NFA::CreateTable(Ptr, 2);

	std::u32string_view String = U"abc-asd where while";

	auto Re = Parser::NFA::Consumer{}(Table, String);
	while (true)
	{
		//auto ite = Pro();
		//volatile int i = 0;
	}



	volatile int i = 0;
}