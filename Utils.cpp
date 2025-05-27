#include "Utils.h"
#include <math.h>
#include <algorithm>

float srandom() {
	float number = float(rand())/float(RAND_MAX);//0.0~1.0f
	number *= 2.0f;//0.0~2.0
	number -= 1.0f;//-1.0f~1.0f;
	return number;
}