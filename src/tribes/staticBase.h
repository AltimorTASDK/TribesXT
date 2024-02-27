#pragma once

#include "tribes/shapeBase.h"
#include <cstddef>

class StaticBase : public ShapeBase {
public:
	struct StaticBaseData : ShapeBase::ShapeBaseData {
		std::byte padding68[0x100 - 0x68];
	};
};
