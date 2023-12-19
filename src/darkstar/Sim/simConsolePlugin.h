#pragma once

#include "darkstar/console/console.h"

class SimManager;

class SimConsolePlugin : public CMDCallback
{
protected:
	// This would normally be the client manager.
	SimManager *manager = nullptr;
	CMDConsole *console = nullptr;

public:
	const char *consoleCallback(CMDConsole*, int id, int argc, const char *argv[]) override
	{
		return nullptr;
	}

	virtual ~SimConsolePlugin()
	{
	}

	virtual void init()
	{
	}

	virtual void setManager(SimManager *mgr)
	{
		manager = mgr;
	}

	virtual void startFrame()
	{
	}

	virtual void endFrame()
	{
	}
};
