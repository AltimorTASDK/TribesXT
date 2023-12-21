#pragma once

#include "util/struct.h"

class SimObject {
protected:
	FIELD(0x44, uint32_t, id);

public:
	uint32_t getId() const
	{
		return id;
	}
};
