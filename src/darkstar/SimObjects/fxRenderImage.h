#pragma once

#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simRenderGrp.h"
#include "util/meta.h"

class fxRenderImage : public SimRenderImage {
public:
	void faceDirection(const Point3F &direction)
	{
		using func_t = to_static_function_t<decltype(&fxRenderImage::faceDirection)>;
		((func_t)0x558890)(this, direction);
	}
};
