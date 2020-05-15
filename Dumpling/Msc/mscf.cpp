#include "mscf.h"
#include "mscf_parser.h"
namespace Dumpling::Msc
{
	using namespace Potato::Parser;


	mscf translate(std::u32string const& code)
	{
		auto& mscf_sbnf = msc_sbnf_instance();
		sbnf_processer sp(mscf_sbnf);
		sp.analyze(code, [](sbnf_processer::travel tra) {

		});
	}
}