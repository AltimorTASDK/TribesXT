#pragma once

#include "darkstar/console/console.h"
#include "darkstar/Core/persist.h"
#include "darkstar/Core/tVector.h"
#include "darkstar/Sim/simConsolePlugin.h"
#include <cstddef>

class FearDynamicDataPlugin : public SimConsolePlugin, public CMDDataManager {
public:
	using Field = Persistent::AbstractTaggedClass::Field;

	struct DataBlockClass {
		DataBlockClass *nextClass;
	private:
		std::byte padding04[0x08 - 0x04];
	public:
		size_t fieldTableSize;
		void *fieldTable;
		const char *className;
		int dbmClass;
	};

	DataBlockClass *classes;
	DataBlockClass *curClasses;
	Vector<Field> tampFields;
	Vector<char*> objectExportText;
};
