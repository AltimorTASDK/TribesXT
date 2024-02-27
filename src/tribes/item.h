#pragma once

#include "tribes/staticBase.h"
#include <cstddef>

class Item {
public:
	struct ItemData : public StaticBase::StaticBaseData {
		int imageId;
		std::byte padding104[0x140 - 0x104];
	};

	static_assert(sizeof(ItemData) == 0x140);
};
