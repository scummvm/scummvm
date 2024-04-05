#include "StdAfx.h"
#include "ObjectSpreader.h"

#include <algorithm>

ObjectSpreader::ObjectSpreader()
{
	radius_.set (15.0f, 22.0f);
}

ObjectSpreader::Circle ObjectSpreader::adjacentCircle (const Circle& circle, float radius)
{
	float angle = M_PI / 0.25f; // random_.fabsRnd (M_PI * 2.0f);
    Vect2f offset (cosf (angle) * (radius + circle.radius),
                   sinf (angle) * (radius + circle.radius));

    Circle result;
    result.position = circle.position + offset;

    result.radius = radius;
    return result;
}

ObjectSpreader::Circle ObjectSpreader::thirdCircle(const Circle& c1, const Circle& c2, float radius)
{
    float r1 = (c1.radius + radius);
    float r2 = (c2.radius + radius); 
    
    Circle result;

    Vect2f u = c2.position - c1.position;
    Vect2f v (u.y, -u.x);
    float ulen2 = u.x*u.x + u.y*u.y;
    float s = 0.5f * ((r1*r1 - r2*r2) / ulen2 + 1);
	float to_be_rooted = r1*r1/ulen2 - s*s;
	
    float t = 0.0f;
	if (to_be_rooted > 0.0f) {
		t = sqrtf (to_be_rooted);
	} else {
		t = 0.0f;
	}
    result.position = c1.position + u * s + v * t;
    result.radius = radius;
    return result;
}

int ObjectSpreader::addCircle (float radius, int outlineIndex)
{
    return addCircle (radius, outline_.begin() + outlineIndex);
}

int ObjectSpreader::addCircle (float radius, Outline::iterator node)
{
    Circle circle;
	Outline::iterator next = node;
	++next;

	if (node == next) {
        circle = adjacentCircle (getCircle(*node), radius);
    } else {
        circle = thirdCircle (getCircle(*node), getCircle(*next), radius);
    }

	outline_.insert (node, objects_.size());

	objects_.push_back (circle);
	if (outlineLength () > 3)
		repairOutline ();
	return objects_.size() - 1;
}

float ObjectSpreader::angle (Outline::iterator node1, Outline::iterator node2, Outline::iterator node3)
{
	Vect2f v1 = getCircle(*node1).position - getCircle(*node2).position;
	Vect2f v2 = getCircle(*node3).position - getCircle(*node2).position;
	Vect2f a = getCircle(*node2).position - getCircle(*node1).position;
	Vect2f b = getCircle(*node3).position - getCircle(*node2).position;
	float angle = v1.angle(v2);
	if (a % b < 0)
		angle = M_PI * 2 - angle;
	return angle;
}

void ObjectSpreader::eraseNode (Outline::iterator node)
{
	outline_.erase (node);
}

void ObjectSpreader::repairOutline ()
{
	Outline::iterator current = outline_.begin();
    do {
        bool pre_condition = false;
        Circle test = thirdCircle (getCircle(*(current - 1)), getCircle(*current), nextRadius ());
        if (test.intersect (getCircle(*(current + 1))) || test.intersect (getCircle(*(current - 2)))) {
            pre_condition = true;
        } else {
			Circle t2 = thirdCircle (getCircle(*current), getCircle(*(current + 1)), nextRadius ());
			if (t2.intersect (getCircle(*(current - 1))) || t2.intersect (getCircle(*(current + 2))))
				pre_condition = true;
        }
        if (pre_condition) {
			float current_angle = angle (current - 2, current - 1, current) + 
								  angle (current, current + 1, current + 2);
			float new_angle = angle (current - 2, current - 1, current + 1) + 
							  angle (current - 1, current + 1, current + 2);

			if (new_angle > current_angle) {
				Outline::iterator prev = current - 1;
				eraseNode (current);
				current = prev;
			}
        } else {
        }
        ++current;
    } while (current != outline_.begin());
}

struct IsInactive {
	bool operator() (const ObjectSpreader::Circle& circle) {
		return circle.active == false;
	}
};

void ObjectSpreader::eraseInactive ()
{
	objects_.erase (std::remove_if (objects_.begin(), objects_.end(), IsInactive ()), objects_.end ());
}

void ObjectSpreader::clear ()
{
    objects_.clear ();

	outline_.clear ();
}

int ObjectSpreader::getNextCircle ()
{
	Outline::iterator begin = outline_.begin();
	Outline::iterator current = begin;
    float min_dist2 = 10e+15f;
    int result = 0;
    int index = 0;
	if (/*current*/ 1) {
		do {
			float dist2 = ((getCircle(*current).position + getCircle(*(current + 1)).position) * 0.5f).norm2 ();
			if (dist2 < min_dist2) {
				min_dist2 = dist2;
				result = index;
			}
			++index;
			++current;
		} while (current != begin);
	    return result;
	} else {
		return 0;
	} 
}

const ObjectSpreader::Circle& ObjectSpreader::addCircle (float radius)
{
	int count = outlineLength ();

	if (count == 0) {
		Circle c;
		c.position.set (0.0f, 0.0f);
		c.radius = radius;
		objects_.push_back (c);
		outline_.insert (0);
		return getCircle(0);
	} else {
		setNextRadius (radius);

		int offset = getNextCircle ();
		int lastOne = addCircle (newRadius(), outline_.begin() + offset);
		return getCircle(lastOne);
	}
}
