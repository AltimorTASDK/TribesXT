#include "plugins/renderXT/renderXT.h"
#include "util/tribes/console.h"
#include <Windows.h>
#include <gl/GL.h>
#include <gl/wglext.h>
#include <gl/glext.h>

static const auto wglSwapIntervalEXT =
	(PFNWGLSWAPINTERVALEXTPROC)
	wglGetProcAddress("wglSwapIntervalEXT");

/*void RenderXTPlugin::hook_glBindTexture(GLenum target, GLuint texture)
{
	get()->hooks.glBindTexture.callOriginal(target, texture);

	//while (glGetError() != GL_NO_ERROR) {
	//}
	glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		Console->printf("gl error %d", error);
	}
}*/

void RenderXTPlugin::hook_glGenTextures(GLsizei n, GLuint *textures)
{
	get()->hooks.glGenTextures.callOriginal(n, textures);

	for (auto i = 0; i < n; i++) {
		glTexParameterf(textures[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glTexParameteri(textures[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

void RenderXTPlugin::init()
{
	if (wglSwapIntervalEXT != nullptr) {
		addVariableXT<"$pref::waitForVSync", [](bool value) {
			wglSwapIntervalEXT(value);
		}>(console);
		wglSwapIntervalEXT(console->getBoolVariable("$pref::waitForVSync"));
	}

	auto maxAnisotropy = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	console->printf(CON_PINK, "maxAnisotropy %f", maxAnisotropy);
}
