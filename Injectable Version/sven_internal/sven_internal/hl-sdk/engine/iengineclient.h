#ifndef IENGINECLIENT_H
#define IENGINECLIENT_H

#include "../common/netadr.h"

#define SC_ENGINE_CLIENT_INTERFACE_VERSION "SCEngineClient001"

class RenderTarget;
struct ref_params_s;

class IEngineClient
{
public:
	virtual ~IEngineClient() {}

	virtual void Con_DnPrintf(int pos, const char *pszMessage, ...) = 0;

	virtual int DrawCharacterOpq(int x, int y, int number, int r, int g, int b, int unknown) = 0;

	virtual int VGUI2_DrawCharacterOpq(int x, int y, int number, int r, int g, int b, unsigned int unknown) = 0;

	virtual void *LoadClientModel(const char *pszModelName) = 0;

	virtual netadr_t *GetServerAddress(netadr_t *) = 0;

	virtual void ChangeSkymap(const char *pszSkyboxName) = 0;

	virtual void *ChangeSkycolor(float r, float g, float b) = 0;

	virtual int QueueDemoSound(float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7) = 0;

	virtual void PushView(RenderTarget *a1, bool a2, bool a3) = 0;

	virtual void PopView(void) = 0;

	virtual void **RenderView(ref_params_s *pparams, bool a2, bool a3, int a4) = 0;
};

#endif // IENGINECLIENT_H