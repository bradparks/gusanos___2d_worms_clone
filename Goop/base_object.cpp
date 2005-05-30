#include "base_object.h"
#include "vec.h"

BaseObject::BaseObject()
{
	deleteMe = false;
}

BaseObject::~BaseObject()
{
}

Vec BaseObject::getPos()
{
	return pos;
}

Vec BaseObject::getRenderPos()
{
	return pos;
}

Vec BaseObject::getSpd()
{
	return spd;
}

float BaseObject::getAngle()
{
	return 0;
}

void BaseObject::remove()
{
	deleteMe = true;
}

bool BaseObject::isCollidingWith( const Vec& point, float radius )
{
	return (pos - point).lengthSqr() < radius*radius;
}
