#include <cxxtest/TestSuite.h>

#include <math.h>
#include "graphics/transform_tools.h"

class TransformToolsSuite : public CxxTest::TestSuite {
public:

	void test_transformPoint(){

		FloatPoint point = FloatPoint();
		Common::Point cPt = Common::Point(1,1);

		FloatPoint test = TransformToolls::transformPoint(point, 1.1, *Common::Point(1,1), false, true);
		
			float x = point.x;
			float y = point.y;
			x = (x * zoom.x) / kDefaultZoomX;
			y = (y * zoom.y) / kDefaultZoomY;
			x = x * cos(rotateRad) - y * sin(rotateRad);
			y = x * sin(rotateRad) + y * cos(rotateRad);
	
			
			y *= -1;

			// X
			TS_ASSERT_EQUALS(test.x,x);

			// Y
			TS_ASSERT_EQUALS(test.x,x);
	}

};
