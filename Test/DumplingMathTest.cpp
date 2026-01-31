import std;
import DumplingMath;
using namespace Dumpling;
int main()
{
	std::array<float, 3> k{1.0f, 2.0f, 3.0f};
	Float3 f3{2.0f, 3.0f};
	auto f4 = f3 * 2.0f;
	auto f6 = f4 + 2.0f;
	f6 -= 2;

	return 0;
}
