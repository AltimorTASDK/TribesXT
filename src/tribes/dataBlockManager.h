#pragma once

#include "darkstar/Sim/simNetObject.h"
#include "tribes/gameBase.h"
#include "util/struct.h"

class DataBlockManager : public SimNetObject {
public:
	enum {
		ItemDataType      = 5,
		ItemImageDataType = 6,
		BulletDataType    = 14,
		GrenadeDataType   = 15,
		RocketDataType    = 16,
	};

	ARRAY_FIELD(0x106C, Vector<GameBase::GameBaseData*>[31], dataBlocks);

	GameBase::GameBaseData *lookupDataBlock(int blockId, int groupId)
	{
		if (blockId >= 0 && blockId < dataBlocks[groupId].size()) {
			const auto &group = dataBlocks[groupId];
			return group[blockId];
		}

		return nullptr;
	}

	const char *lookupBlockName(int blockId, int groupId)
	{
		if (const auto *data = lookupDataBlock(blockId, groupId); data != nullptr)
			return data->dbmDatFileName;

		return nullptr;
	}
};
