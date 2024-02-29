#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include <Windows.h>
#include <gl/GL.h>

class RenderXTPlugin : public SimConsolePlugin {
	static inline RenderXTPlugin *instance;

public:
	static RenderXTPlugin *get()
	{
		return instance;
	}

private:
	//static void __stdcall hook_glBindTexture(GLenum target, GLuint texture);
	static void __stdcall hook_glGenTextures(GLsizei n, GLuint *textures);

	struct {
		// Set GL_TEXTURE_MAX_ANISOTROPY_EXT
		//StaticJmpHook<glBindTexture, hook_glBindTexture> glBindTexture;
		StaticJmpHook<glGenTextures, hook_glGenTextures> glGenTextures;
	} hooks;

public:
	RenderXTPlugin()
	{
		instance = this;
	}

	~RenderXTPlugin()
	{
		instance = nullptr;
	}

	void init() override;
};
