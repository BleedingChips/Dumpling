#include "../Public/Interval.h"
namespace PineApple
{
	namespace Implement
	{
		RelationShip Compress(RelationshipOpe left_left, RelationshipOpe left_right, RelationshipOpe right_left, RelationshipOpe right_right)
		{
			switch (right_left)
			{
			case RelationshipOpe::Less: return RelationShip::Left;
			case RelationshipOpe::Equal: return RelationShip::LeftEqual;
			case RelationshipOpe::Big:
				switch (left_right)
				{
				case RelationshipOpe::Equal: return RelationShip::RightEqual;
				case RelationshipOpe::Big: return RelationShip::Right;
				case RelationshipOpe::Less:
					switch (left_left)
					{
					case RelationshipOpe::Less:
						switch (right_right)
						{
						case RelationshipOpe::Less: return RelationShip::LeftIntersect;
						case RelationshipOpe::Equal: return RelationShip::IncludeRightEqual;
						case RelationshipOpe::Big: return RelationShip::Include;
						default:
							break;
						}
					case RelationshipOpe::Equal:
						switch (right_right)
						{
						case RelationshipOpe::Less: return RelationShip::BeIncludeLeftEqual;
						case RelationshipOpe::Equal: return RelationShip::Equal;
						case RelationshipOpe::Big: return RelationShip::IncludeLeftEqual;
						default:
							break;
						}
					case RelationshipOpe::Big:
						switch (right_right)
						{
						case RelationshipOpe::Less: return RelationShip::BeInclude;
						case RelationshipOpe::Equal: return RelationShip::BeIncludeRightEqual;
						case RelationshipOpe::Big: return RelationShip::RightIntersect;
						default:
							break;
						}
					default:
						break;
					}
				default:
					break;
				}
			default:
				break;
			}
			assert(false);
			return RelationShip::Unknow;
		}
	}
}