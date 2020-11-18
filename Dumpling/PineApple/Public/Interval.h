#pragma once
#include <vector>
#include <assert.h>
#include <optional>
#include <tuple>
#include <algorithm>
namespace PineApple
{

	enum class RelationShip
	{
		Left,
		LeftIntersect,
		LeftEqual,
		BeInclude,
		BeIncludeLeftEqual,
		BeIncludeRightEqual,
		Include,
		IncludeLeftEqual,
		IncludeRightEqual,
		RightEqual,
		RightIntersect,
		Right,
		Equal,
		Unknow
	};

	template<typename Storage, typename Less = std::less<Storage>>
	struct Point
	{
		Storage storage;
		Point(Storage p) : storage(std::move(p)) {}
		Point(Point&& p) = default;
		Point(Point const&) = default;
		Point() = default;
		Point& operator= (Point const&) = default;
		Point& operator= (Point &&) = default;
		bool operator< (Point const& o) const noexcept{return Less{}(storage, o.storage);}
		bool operator<= (Point const& o) const noexcept { return Less{}(storage, o.storage) || storage == o.storage; }
		Storage& Original() {return storage;}
		Storage const& Original() const { return storage; }
		operator Storage const&() const {return storage;}
		operator Storage& () { return storage; }
	};

	namespace Implement
	{
		enum class RelationshipOpe : int8_t
		{
			Less, Equal, Big
		};

		template<typename PointT>
		RelationshipOpe Compress(PointT const& t1, PointT const& t2)
		{
			if (t1 < t2)
				return RelationshipOpe::Less;
			else if(t2 < t1)
				return RelationshipOpe::Big;
			else
				return RelationshipOpe::Equal;
		}

		RelationShip Compress(RelationshipOpe left_left, RelationshipOpe left_right, RelationshipOpe right_left, RelationshipOpe right_right);
		struct no_detection_t {};
	}

	template<typename PointT, typename Less = std::less<PointT>>
	struct Segment
	{
		using Point = Point<PointT, Less>;

		Segment(Point p1, Point p2) : storage(std::tuple<Point, Point>{std::move(p1), std::move(p2)}){
			auto& [p1c, p2c] = storage;
			if(!(p1c < p2c))
				std::swap(p1c, p2c);
		}
		Segment(Point p1, Point p2, Implement::no_detection_t) : storage(std::tuple<Point, Point>{ std::move(p1), std::move(p2) }) {}
		Segment() = default;
		Segment(Segment const&) = default;
		Segment(Segment &&) = default;
		Segment& operator=(Segment const&) = default;
		Segment& operator=(Segment &&) = default;

		Point const& Start() const { return std::get<0>(storage); }
		Point const& End() const { return std::get<1>(storage); }

		
		RelationShip Relationship(Segment const& p) const { 
			return Implement::Compress(
				Implement::Compress(Start(), p.Start()), 
				Implement::Compress(Start(), p.End()), 
				Implement::Compress(End(), p.Start()),
				Implement::Compress(End(), p.End())
				);
		}

		bool IsInclude(Point const& si) const { return Start() <= si && si < End(); }

		operator bool() const { return Start() != End(); }

		Segment Maximum(Segment const& si, Implement::no_detection_t) const;

		Segment Maximum(Segment const& si) const
		{
			if (*this)
			{
				if (si)
					return Maximum(si, {});
				else
					return *this;
			}
			else
				return si;
		}

		Segment Intersection(Segment const& si, Implement::no_detection_t) const;

		Segment Intersection(Segment const& si) const {
			if (*this && si)
				return Intersection(si, Implement::no_detection_t{});
			return {};
		}

		Segment Union(Segment const& si, Implement::no_detection_t) const;

		Segment Union(Segment const& si) const {
			if (*this)
			{
				if (si)
					return Union(si, Implement::no_detection_t{});
				else
					return *this;
			}
			else
				return si;
		}

		std::tuple<Segment, Segment> Cut(Segment const& si, Implement::no_detection_t) const;

		std::tuple<Segment, Segment> Cut(Segment const& si) const	{
			if (*this && si)
				return Cut(si, Implement::no_detection_t{});
			return {*this, {}};
		}

		Segment operator&(Segment const& si) const { return Intersection(si); }

		Segment operator|(Segment const& si) const { return Union(si); }
		std::tuple<Segment, Segment> operator-(Segment const& si) const {return Cut(si);}

	public:
		std::tuple<Point, Point> storage;
	};

	template<typename PointT, typename Less>
	auto Segment<PointT, Less>::Intersection(Segment const& si, Implement::no_detection_t) const -> Segment {
		assert(*this && si);
		auto relation = Relationship(si);
		switch (relation)
		{
		case RelationShip::Right:
		case RelationShip::Left:
		case RelationShip::RightEqual:
		case RelationShip::LeftEqual:
			return {};
		case RelationShip::LeftIntersect: 
			return { si.Start(), End(), Implement::no_detection_t{} };
		case RelationShip::BeInclude:
		case RelationShip::BeIncludeLeftEqual:
		case RelationShip::BeIncludeRightEqual:
		case RelationShip::Equal:
			return *this;
		case RelationShip::Include:
		case RelationShip::IncludeLeftEqual:
		case RelationShip::IncludeRightEqual:
			return si;
		case RelationShip::RightIntersect:
			return { si.End(), Start(), Implement::no_detection_t{} };
		default:
			assert(false);
			return {};
		}
	}

	template<typename PointT, typename Less>
	auto Segment<PointT, Less>::Maximum(Segment const& si, Implement::no_detection_t) const -> Segment {
		assert(*this && si);
		auto relation = Relationship(si);
		switch (relation)
		{
		case RelationShip::Left:
		case RelationShip::LeftEqual:
		case RelationShip::LeftIntersect:
			return {Start(), si.End(), Implement::no_detection_t{}};
		case RelationShip::Right:
		case RelationShip::RightEqual:
		case RelationShip::RightIntersect:
			return {si.Start(), End(), Implement::no_detection_t{}};
		case RelationShip::BeInclude:
		case RelationShip::BeIncludeLeftEqual:
		case RelationShip::BeIncludeRightEqual:
		case RelationShip::Equal:
			return *this;
		case RelationShip::Include:
		case RelationShip::IncludeLeftEqual:
		case RelationShip::IncludeRightEqual:
			return si;
		default:
			assert(false);
			return {};
		}
	}

	template<typename PointT, typename Less>
	auto Segment<PointT, Less>::Union(Segment const& si, Implement::no_detection_t) const -> Segment {
		assert(*this && si);
		auto relation = Relationship(si);
		switch (relation)
		{
		case RelationShip::Left:
		case RelationShip::Right:
			return {};
		case RelationShip::LeftIntersect:
		case RelationShip::LeftEqual:
			return { Start(), si.End(), Implement::no_detection_t{} };
		case RelationShip::BeInclude:
		case RelationShip::BeIncludeLeftEqual:
		case RelationShip::BeIncludeRightEqual:
			return si;
		case RelationShip::Include:
		case RelationShip::IncludeLeftEqual:
		case RelationShip::IncludeRightEqual:
		case RelationShip::Equal:
			return *this;
		
		case RelationShip::RightIntersect:
		case RelationShip::RightEqual:
			return { si.Start(), End(), Implement::no_detection_t{} };
		default:
			assert(false);
			return {};
		}
	}

	template<typename PointT, typename Less>
	auto Segment<PointT, Less>::Cut(Segment const& si, Implement::no_detection_t) const -> std::tuple<Segment, Segment> {
		assert(*this && si);
		auto relation = Relationship(si);
		switch (relation)
		{
		case RelationShip::Left:
		case RelationShip::LeftEqual:
		case RelationShip::RightEqual:
		case RelationShip::Right:
			return { *this, {} };
		case RelationShip::BeInclude:
		case RelationShip::BeIncludeLeftEqual:
		case RelationShip::BeIncludeRightEqual:
		case RelationShip::Equal:
			return { {}, {} };
		case RelationShip::Include:
			return { {Start(), si.Start(), Implement::no_detection_t{}}, {si.End(), End(), Implement::no_detection_t{}} };
		case RelationShip::RightIntersect:
		case RelationShip::IncludeLeftEqual:
			return { {si.End(), End(), Implement::no_detection_t{}}, {} };
		case RelationShip::LeftIntersect:
		case RelationShip::IncludeRightEqual:
			return { {Start(), si.Start(), Implement::no_detection_t{}}, {} };
		case RelationShip::Unknow:
		default:
			assert(false);
			return { {}, {} };
		}
	}

	template<typename PointT, typename Less = std::less<PointT>, template<typename Type> class Allocator = std::allocator>
	struct Interval
	{
		using Segment = Segment<PointT, Less>;
		using Point = typename Segment::Point;

		using StorageType = std::vector<Segment, Allocator<Segment>>;

		Interval() = default;
		Interval(Interval&&) = default;
		Interval(Interval const&) = default;
		Interval& operator=(Interval&&) = default;
		Interval& operator=(Interval const&) = default;

		Interval(Segment const& si) : Interval(si ? Interval({ si }, Implement::no_detection_t{}) : Interval()) {}
		Interval(StorageType st, Implement::no_detection_t) : single_intervals(std::move(st)) {};
		Interval(StorageType st);
		Interval operator|(Segment const& inter) const { return *this | Interval{ inter }; };
		Interval operator|(Interval const& inter) const;
		Interval operator& (Segment const& inter) const { return *this & Interval{ inter }; };
		Interval operator& (Interval const& inter) const;
		Interval operator- (Segment const& inter) const { return *this - Interval{ inter }; };
		Interval operator- (Interval const& inter) const;
		bool IsInclude(Segment const&) const;
		size_t size() const { return single_intervals.size(); }
		operator bool() const { return !single_intervals.empty(); }
		Segment Maximum() const {
			switch (single_intervals.size())
			{
			case 0: return {};
			case 1: return *single_intervals.begin();
			default: return { single_intervals.begin()->Start(), single_intervals.rbegin()->End(), Implement::no_detection_t{} };
			}
		}
		auto begin() {return single_intervals.begin();}
		auto begin() const { return single_intervals.begin(); }
		auto end() { return single_intervals.end(); }
		auto end() const { return single_intervals.end(); }
	private:
		StorageType single_intervals;
	};

	template<typename PointT, typename Less, template<typename Type> class Allocator>
	Interval<PointT, Less, Allocator>::Interval(StorageType st) : single_intervals(std::move(st))
	{
		single_intervals.erase(std::remove_if(single_intervals.begin(), single_intervals.end(), [](Segment const& si) { return !si; }), single_intervals.end());
		std::sort(single_intervals.begin(), single_intervals.end(), [](Segment const& s1, Segment const& s2) { return s1.Start() < s2.Start(); });
		if (single_intervals.size() >= 2)
		{
			auto ite = single_intervals.begin();
			auto last_ite = ite + 1;
			while (last_ite != single_intervals.end())
			{
				assert(*ite);
				auto re = (*ite | *last_ite);
				if (re)
				{
					*ite = re;
					*(last_ite++) = {};
				}
				else {
					ite = last_ite;
					++last_ite;
				}
			}
		}
		single_intervals.erase(std::remove_if(single_intervals.begin(), single_intervals.end(), [](Segment const& si) { return !si; }), single_intervals.end());
	}

	template<typename PointT, typename Less, template<typename Type> class Allocator>
	auto Interval<PointT, Less, Allocator>::operator|(Interval const& inter) const ->Interval
	{
		std::vector<Segment, Allocator<Segment>> result;
		result.reserve(size() + inter.size());
		result.insert(result.end(), single_intervals.begin(), single_intervals.end());
		result.insert(result.end(), inter.single_intervals.begin(), inter.single_intervals.end());
		std::sort(result.begin(), result.end(), [](Segment const& s1, Segment const& s2){ return s1.Start() < s2.Start(); });
		return Interval(result);
	}

	template<typename PointT, typename Less, template<typename Type> class Allocator>
	auto Interval<PointT, Less, Allocator>::operator&(Interval const& inter) const ->Interval
	{
		auto i1 = single_intervals.begin();
		auto i1e = single_intervals.end();
		auto i2 = inter.single_intervals.begin();
		auto i2e = inter.single_intervals.end();
		StorageType result;
		while (i1 != i1e && i2 != i2e)
		{
			auto relation = i1->Relationship(*i2);
			switch (relation)
			{
			case RelationShip::Left:
			case RelationShip::LeftEqual:
				++i1; break;
			case RelationShip::RightEqual:
			case RelationShip::Right:
				++i2; break;
			case RelationShip::BeInclude:
			case RelationShip::BeIncludeLeftEqual:
				result.push_back(*i1); ++i1; break;
			case RelationShip::BeIncludeRightEqual:
			case RelationShip::Equal:
				result.push_back(*i1); ++i2; ++i1; break;
			case RelationShip::Include:
				result.push_back(*i2); ++i2; break;
			case RelationShip::RightIntersect:
				result.push_back({i1->Start(), i2->End(), Implement::no_detection_t{}}); ++i2; break;
			case RelationShip::IncludeLeftEqual:
				result.push_back(*i2); ++i2; break;
			case RelationShip::LeftIntersect:
				result.push_back({i2->Start(), i1->End(), Implement::no_detection_t{}}); ++i1; break;
			case RelationShip::IncludeRightEqual:
				result.push_back(*i2); ++i2; ++i1; break;
			case RelationShip::Unknow:
			default:
				assert(false);
				break;
			}
		}
		return Interval(result, Implement::no_detection_t{});
	}

	template<typename PointT, typename Less, template<typename Type> class Allocator>
	auto Interval<PointT, Less, Allocator>::operator-(Interval const& inter) const ->Interval
	{
		auto i1 = single_intervals.begin();
		auto i1e = single_intervals.end();
		auto i2 = inter.single_intervals.begin();
		auto i2e = inter.single_intervals.end();

		StorageType result;
		std::optional<Segment> Tar;
		while (i1 != i1e && i2 != i2e)
		{
			if(!Tar)
				Tar = *(i1++);
			auto [r1, r2] = (*Tar - *i2);
			if (r2)
			{
				result.push_back(std::move(r1));
				Tar = r2;
				++i2;
			}
			else if(r1){
				if(r1.Start() < i2->Start())
				{
					result.push_back(std::move(r1));
					Tar = {};
				}
				else {
					Tar = r1;
					++i2;
				}
			}
			else {
				Tar = {};
				++i2;
			}
		}
		if(Tar)
			result.push_back(std::move(*Tar));
		result.insert(result.end(), i1, i1e);
		return Interval(result, Implement::no_detection_t{});
	}

	/*
	template<typename Storage, typename Less = std::less<Storage>, template<typename Type> class Allocator = std::allocator>
	struct Set
	{
		struct Range
		{

			static bool small(const Storage& i1, const Storage& i2) { return Less{}(i1, i2); }
			static bool small_equal(const Storage& i1, const Storage& i2) { return !big(i1, i2); }
			static bool big(const Storage& i1, const Storage& i2) { return small(i2, i1); }
			static bool big_equal(const Storage& i1, const Storage& i2) { return !small(i1, i2); }
			
			Storage left;
			Storage right;
			bool operator<(const Range& input) { return Less{}(left, input.left); }
			std::tuple<RelationShip, std::optional<Range>> union_set(const Range& input) const
			{
				if (small(right, input.left)) // right < input.left
					return { RelationShip::Left, std::nullopt };
				else if (big(left, input.right)) // left > input.right
					return { RelationShip::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if(big(right, input.right)) // right > input.right
							return { RelationShip::Include, Range{left, right} };
						else if(small(right, input.right)) // right < input.right
							return { RelationShip::LeftIntersect, Range{left, input.right} };
						else // right == input.right
							return { RelationShip::IncludeRightEqual, Range{left, right} };
					}
					else if (big(left, input.left)) // left > input.left
					{
						if (small(right, input.right)) // right < input.right;
							return { RelationShip::BeInclude, Range{input.left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { RelationShip::RightIntersect, Range{input.left, right} };
						else // right == input.right
							return { RelationShip::BeIncludeRightEqual, Range{input.left, input.right} };
					}
					else { // left == input.left
						if (small(right, input.right)) // right < input.right
							return { RelationShip::BeIncludeLeftEqual, Range{left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { RelationShip::IncludeLeftEqual, Range{left, right} };
						else // right == input.right
							return { RelationShip::Equal, Range{left, input.right} };
					}
				}
			}
			auto operator|(const Range& include) const { return union_set(include); }

			std::tuple<RelationShip, std::optional<Range>> intersection_set(const Range& input) const
			{
				if (small_equal(right, input.left)) // right <= input.left
					return { RelationShip::Left, std::nullopt };
				else if (big_equal(left, input.right)) // left >= input.right
					return { RelationShip::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if (big(right, input.right)) // right > input.right
							return { RelationShip::Include, Range{input.left, input.right} };
						else if(small(right, input.right))// right < input.right
							return { RelationShip::LeftIntersect, Range{input.left, right} };
						else // =
							return { RelationShip::IncludeRightEqual, Range{input.left, input.right} };
					}
					else if(big(left, input.left)){ // left > input.left
						if (small(right, input.right)) // right <= input.right;
							return { RelationShip::BeInclude, Range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { RelationShip::RightIntersect, Range{left, input.right} };
						else // =
							return { RelationShip::BeIncludeRightEqual, Range{left, right} };
					}
					else { // left  == input.left
						if (small(right, input.right)) // right < input.right
							return { RelationShip::BeIncludeLeftEqual, Range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { RelationShip::IncludeLeftEqual, Range{left, input.right} };
						else // right == input.right
							return { RelationShip::Equal, Range{left, input.right} };
					}
				}
			}

			bool inside(Storage Input) const noexcept { return big_equal(Input, left) && small(Input, right); }

			auto operator&(const Range& include) const { return intersection_set(include); }
		};

		Set(const Set&) = default;
		Set(std::initializer_list<Range> ul) : m_set(ul) {}
		Set(Set&&) = default;
		Set& operator=(const Set&) = default;
		Set& operator=(Set&&) = default;
		Set() = default;
		Set(const Range& input) : m_set({ input }) { assert(Less{}(input.left, input.right)); }
		Set(const Storage& input) : m_set({ Range{input, input + 1}}) {}
		size_t size() const noexcept { return m_set.size(); }
		bool empty() const noexcept { return m_set.empty(); }
		Range operator[](size_t index) const noexcept { return m_set[index]; }

		auto begin() const noexcept { return m_set.begin(); }
		auto end() const noexcept { return m_set.end(); }

		Set operator|(const Set& input) const;
		Set& operator|=(const Set& input) { auto tem = std::move(*this); *this = (tem | input); return *this; }
		Set operator&(const Set&) const;
		Set operator&=(const Set& input) { auto tem = std::move(*this); *this = (tem & input); return *this; }
		Set operator-(const Set& input) const {
			auto cur = *this;
			cur -= input;
			return std::move(cur);
		};
		Set& operator-=(const Set& input) {
			auto input_c = input;
			auto re = intersection_cull(input_c);
			return *this;
		}
		Set intersection_cull(Set& input);
		std::vector<Range, Allocator<Range>>& storage() { return m_set; }
		std::vector<Range, Allocator<Range>> const& storage() const { return m_set; }
		bool intersection_find(const Range& input) const;
	private:
		std::vector<Range, Allocator<Range>> m_set;
	};

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::intersection_find(const Range& input) const-> bool
	{
		for (auto ite = m_set.begin(); ite != m_set.end();)
		{
			auto [lo, re] = *ite & input;
			switch (lo)
			{
			case RelationShip::Left: ++ite; break;
			case RelationShip::BeInclude:
			case RelationShip::BeIncludeLeftEqual:
			case RelationShip::BeIncludeRightEqual:
			case RelationShip::LeftIntersect:
			case RelationShip::Right:
			case RelationShip::RightIntersect:
				return false;
			case RelationShip::Include:
			case RelationShip::IncludeLeftEqual:
			case RelationShip::IncludeRightEqual:
			case RelationShip::Equal:
				return true;
			default:
				break;
			}
		}
		return false;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::operator|(const Set& input) const->Set
	{
		Set Temporary_result;
		auto& ref = Temporary_result.m_set;
		ref.reserve(m_set.size() + input.m_set.size());
		auto ite = m_set.begin();
		auto ite2 = input.m_set.begin();
		std::optional<Range> result;
		decltype(ite) search_target;
		bool LastDetect = false;
		while (ite != m_set.end() && ite2 != input.m_set.end())
		{
			if (result)
			{
				bool Change = false;
				auto [lo, r1] = *result | *ite;
				if (r1) {
					Change = true; ++ite;
					result = *r1;
				}
				auto [lo2, r2] = *result | *ite2;
				if (r2) {
					Change = true; ++ite2;
					result = *r2;
				}
				if (!Change)
				{
					ref.push_back(*result);
					result = std::nullopt;
				}
			}
			else {
				auto [lo, re] = *ite | *ite2;
				if (re)
				{
					result = *re;
					++ite; ++ite2;
				}
				else {
					switch (lo)
					{
					case RelationShip::Left: ref.push_back(*ite); ++ite; break;
					case RelationShip::Right: ref.push_back(*ite2); ++ite2; break;
					default: assert(false); break;
					}
				}
			}
		}
		if (result)
			ref.push_back(*result);
		ref.insert(ref.end(), ite, m_set.end());
		ref.insert(ref.end(), ite2, input.m_set.end());
		return Temporary_result;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::operator&(const Set& input) const -> Set
	{
		Set result;
		auto& ref = result.m_set();
		auto i1 = m_set.begin();
		auto i2 = input.m_set.begin();
		while (i1 != m_set.end() && i2 != input.m_set.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case RelationShip::Left: 
				++i1; break;
			case RelationShip::Right: ++i2; break;
			case RelationShip::BeInclude: 
			case RelationShip::LeftIntersect: 
				ref.push_back(*re); ++i1; break;
			case RelationShip::RightIntersect:
			case RelationShip::Include: 
				ref.push_back(*re); ++i2; break;
			case RelationShip::Equal:
				ref.push_back(*re); ++i2; ++i1; break;
			default:
				break;
			}
		}
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::intersection_cull(Set& input) -> Set
	{
		Set result;
		auto& ref = result.m_set;
		auto set1 = std::move(m_set);
		auto set2 = std::move(input.m_set);
		auto i1 = set1.begin();
		auto i2 = set2.begin();
		while (i1 != set1.end() && i2 != set2.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case RelationShip::Left: m_set.push_back(*i1); ++i1; break;
			case RelationShip::Right: input.m_set.push_back(*i2); ++i2; break;
			case RelationShip::BeInclude:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case RelationShip::BeIncludeLeftEqual:
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case RelationShip::BeIncludeRightEqual:
				input.m_set.push_back({ i2->left, i1->left });
				ref.push_back(*re);
				++i2;
				++i1;
				break;
			case RelationShip::LeftIntersect:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back({ i2->left, i1->right }); 
				i2->left = i1->right;
				++i1;
				break;
			case RelationShip::RightIntersect:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RelationShip::Include:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RelationShip::IncludeLeftEqual:
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RelationShip::IncludeRightEqual:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				++i1;
				++i2;
				break;
			case RelationShip::Equal:
				ref.push_back(*i1); ++i2; ++i1; break;
			default: assert(false); break;
			}
		}
		m_set.insert(m_set.end(), i1, set1.end());
		input.m_set.insert(input.m_set.end(), i2, set2.end());
		return std::move(result);
	}
	*/

}
