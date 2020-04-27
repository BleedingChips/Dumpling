
#include "msc_parser.h"
using namespace Potato::Parser;
using namespace Potato::Lexical;
using namespace Potato::Syntax;
using range_set = nfa_storage::range_set;
using range = range_set::range;
using EdgeType = nfa_storage::EdgeType;
sbnf const& msc_sbnf_instance(){
	static sbnf instance{
		std::u32string(U"\x27\x28\x27\x27\x29\x27\x27\x2A\x27\x27\x2B\x27\x43\x6F\x64\x65\x49\x64\x4E\x75\x6D\x62\x65\x72\x53\x74\x72\x69\x6E\x67\x5F\x49\x47\x4E\x4F\x52\x45\x3C\x53\x74\x61\x74\x65\x6D\x65\x6E\x74\x3E"),
		{{30, 7},{12, 4},{18, 6},{24, 6},{16, 2},{9, 3},{6, 3},{0, 3},{3, 3},{37, 11},},
		9,
		0,
		4294967294,
		nfa_storage{
			{
				range_set({{41, 42},}),range_set({{40, 41},}),range_set({{42, 43},}),range_set({{43, 44},}),range_set({{65, 91},{95, 96},{97, 123},}),range_set({{34, 35},}),range_set({{45, 46},}),range_set({{49, 58},}),range_set({{64, 65},}),range_set({{1, 33},{127, 128},}),range_set({{48, 58},{65, 91},{95, 96},{97, 123},}),range_set({{92, 93},}),range_set({{10, 11},}),range_set({{1, 10},{11, 34},{35, 92},{93, 4294967295},}),range_set({{34, 35},}),range_set({{49, 58},}),range_set({{48, 58},}),range_set({{46, 47},}),range_set({{123, 124},}),range_set({{1, 33},{127, 128},}),range_set({{1, 11},{11, 125},{126, 4294967295},}),range_set({{125, 126},}),range_set({{1, 11},{11, 125},{126, 4294967295},}),range_set({{125, 126},}),range_set({{1, 11},{11, 64},{65, 125},{126, 4294967295},}),range_set({{125, 126},}),range_set({{64, 65},}),range_set({{48, 58},}),range_set({{46, 47},}),range_set({{48, 58},}),range_set({{48, 58},}),range_set({{92, 93},}),range_set({{10, 11},}),range_set({{1, 10},{11, 92},{93, 4294967295},}),range_set({{34, 35},}),range_set({{92, 93},}),range_set({{10, 11},}),range_set({{1, 10},{11, 34},{35, 92},{93, 4294967295},}),range_set({{34, 35},}),range_set({{92, 93},}),range_set({{10, 11},}),range_set({{1, 10},{11, 34},{35, 92},{93, 4294967295},}),range_set({{34, 35},}),range_set({{48, 58},{65, 91},{95, 96},{97, 123},}),
			},

			{
				{EdgeType::Comsume, 0, 1},{EdgeType::Comsume, 1, 2},{EdgeType::Comsume, 2, 3},{EdgeType::Comsume, 3, 4},{EdgeType::Comsume, 4, 5},{EdgeType::Comsume, 5, 6},{EdgeType::Comsume, 6, 7},{EdgeType::Comsume, 7, 8},{EdgeType::Comsume, 8, 9},{EdgeType::Comsume, 9, 10},{EdgeType::Acception, 8, 32},{EdgeType::Acception, 7, 31},{EdgeType::Acception, 6, 30},{EdgeType::Acception, 5, 29},{EdgeType::Comsume, 10, 27},{EdgeType::Acception, 4, 28},{EdgeType::Comsume, 11, 21},{EdgeType::Comsume, 12, 22},{EdgeType::Comsume, 13, 23},{EdgeType::Comsume, 14, 24},{EdgeType::Comsume, 15, 8},{EdgeType::Comsume, 16, 17},{EdgeType::Comsume, 17, 18},{EdgeType::Acception, 2, 19},{EdgeType::Comsume, 18, 12},{EdgeType::Comsume, 19, 10},{EdgeType::Acception, 0, 11},{EdgeType::Comsume, 20, 13},{EdgeType::Comsume, 21, 14},{EdgeType::Comsume, 22, 13},{EdgeType::Comsume, 23, 14},{EdgeType::Comsume, 24, 13},{EdgeType::Comsume, 25, 14},{EdgeType::Comsume, 26, 15},{EdgeType::Acception, 1, 16},{EdgeType::Comsume, 27, 17},{EdgeType::Comsume, 28, 18},{EdgeType::Acception, 2, 19},{EdgeType::Comsume, 29, 20},{EdgeType::Comsume, 30, 20},{EdgeType::Acception, 2, 19},{EdgeType::Comsume, 31, 21},{EdgeType::Comsume, 32, 22},{EdgeType::Comsume, 33, 23},{EdgeType::Comsume, 34, 25},{EdgeType::Comsume, 35, 21},{EdgeType::Comsume, 36, 22},{EdgeType::Comsume, 37, 23},{EdgeType::Comsume, 38, 25},{EdgeType::Comsume, 39, 21},{EdgeType::Comsume, 40, 22},{EdgeType::Comsume, 41, 23},{EdgeType::Comsume, 42, 25},{EdgeType::Acception, 3, 26},{EdgeType::Acception, 3, 26},{EdgeType::Comsume, 43, 27},{EdgeType::Acception, 4, 28},
			},
			{
				{0, 10},{10, 1},{11, 1},{12, 1},{13, 1},{14, 2},{16, 4},{20, 1},{21, 3},{24, 1},{25, 2},{27, 0},{27, 2},{29, 2},{31, 3},{34, 1},{35, 0},{35, 3},{38, 1},{39, 0},{39, 2},{41, 3},{44, 1},{45, 4},{49, 5},{54, 1},{55, 0},{55, 2},{57, 0},{57, 0},{57, 0},{57, 0},{57, 0},
			}
		},
		lr1_storage{
			{
				{2147483648, 1, 18446744073709551615},{2147483648, 3, 1},{2147483648, 3, 2},{2147483648, 3, 18446744073709551615},{4294967295, 1, 18446744073709551615},
			},
			{
				{2, 1},{7, 2},{2147483648, 3},{5, 0},{6, 0},{2147483647, 0},{2, 8},{7, 9},{2147483648, 10},{2147483647, 4},{5, 4},{6, 5},{2, 1},{7, 2},{2147483648, 7},{2, 1},{7, 2},{2147483648, 6},{5, 2},{6, 2},{2147483647, 2},{5, 4},{6, 5},{5, 1},{2147483647, 1},{5, 4},{6, 5},{5, 0},{6, 0},{8, 0},{2, 8},{7, 9},{2147483648, 16},{5, 11},{6, 12},{8, 13},{2, 8},{7, 9},{2147483648, 15},{2, 8},{7, 9},{2147483648, 14},{5, 3},{6, 3},{2147483647, 3},{5, 2},{6, 2},{8, 2},{5, 11},{6, 12},{5, 1},{8, 1},{5, 11},{6, 12},{5, 11},{6, 12},{8, 17},{5, 3},{6, 3},{8, 3},
			},
			{
				{0, 0, 3},{3, 3, 0},{6, 0, 3},{9, 1, 2},{12, 0, 3},{15, 0, 3},{18, 3, 2},{23, 2, 2},{27, 3, 0},{30, 0, 3},{33, 0, 3},{36, 0, 3},{39, 0, 3},{42, 3, 0},{45, 3, 2},{50, 2, 2},{54, 0, 3},{57, 3, 0},
			}
		}
	};

	return instance;
}

