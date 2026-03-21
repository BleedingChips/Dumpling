import std;
import DumplingMath;
import Potato;
using namespace Dumpling;
using namespace Dumpling::Math;
int main()
{
	std::array<float, 3> k{1.0f, 2.0f, 3.0f};
	Float3 f3{2.0f, 3.0f};
	auto f4 = f3 * 2.0f;
	auto f6 = f4 + 2.0f;
	f6 -= 2;

	Float3 f1(2.34f);

	auto p = Math::Vector(1.0, 2.0);
	auto p3 = Math::Vector(2.0f, 1.0f);
	auto p4 = p + p3;

	auto iup = p4.Length();

	auto k23 = 3.0f + p3;

	auto k233 = p.Slice<6>(0, 1, 1, 1, 1, 0);

	Potato::Log::Log<u"Test", Potato::Log::LogLevel::Log, "Vector=<{}>">(k233);

	static_assert(std::is_convertible_v<double, float>, "123");
	static_assert(std::is_convertible_v<float, double>, "1234");

	auto kop = sizeof(long double);
	auto kop23 = sizeof(double);

	return 0;
}
