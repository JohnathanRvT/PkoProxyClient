//
#pragma once

#include "lwHeader.h"
#include "lwInterfaceExt.h"
#include "lwStdInc.h"
#include "lwDirectX.h"
#include "lwClassDecl.h"
#include "lwCoordinateSys.h"

LW_BEGIN

class lwSysGraphics : public lwISysGraphics {
public:
	static lwISysGraphics* __sys_graphics;
	static void SetActiveIGraphicsSystem(lwISysGraphics* sys) { __sys_graphics = sys; }
	static lwISysGraphics* GetActiveIGraphicsSystem() { return __sys_graphics; }

private:
	lwSystem* _sys;

	lwIDeviceObject* _dev_obj;
	lwIResourceMgr* _res_mgr;
	lwISceneMgr* _scene_mgr;

	lwCoordinateSys _coord_sys;

	lwOutputLoseDeviceProc _lose_dev_proc;
	lwOutputResetDeviceProc _reset_dev_proc;

	LW_STD_DECLARATION();

public:
	lwSysGraphics(lwSystem* sys);
	~lwSysGraphics();

	lwISystem* GetSystem() override { return (lwISystem*)_sys; }
	lwIDeviceObject* GetDeviceObject() override { return _dev_obj; }
	lwIResourceMgr* GetResourceMgr() override { return _res_mgr; }
	lwISceneMgr* GetSceneMgr() override { return _scene_mgr; }
	lwICoordinateSys* GetCoordinateSys() override { return &_coord_sys; }

	LW_RESULT CreateDeviceObject(lwIDeviceObject** ret_obj) override;
	LW_RESULT CreateResourceManager(lwIResourceMgr** ret_obj) override;
	LW_RESULT CreateSceneManager(lwISceneMgr** ret_obj) override;

	LW_RESULT ToggleFullScreen(D3DPRESENT_PARAMETERS* d3dpp, lwWndInfo* wnd_info) override;
	LW_RESULT TestCooperativeLevel() override;

	void SetOutputLoseDeviceProc(lwOutputLoseDeviceProc proc) override { _lose_dev_proc = proc; }
	void SetOutputResetDeviceProc(lwOutputResetDeviceProc proc) override { _reset_dev_proc = proc; }
};

LW_END