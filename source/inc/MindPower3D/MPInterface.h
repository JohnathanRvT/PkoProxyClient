//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwInterface.h"
#include "lwITypes.h"
#include "lwITypes2.h"
#include "lwMath.h"
#include "lwShaderTypes.h"
#include "lwIFunc.h"
#include "lwIUtil.h"

LW_USING;

// class
using MPAnimCtrlObjTypeInfo = lwAnimCtrlObjTypeInfo;
using MPVector2 = lwVector2;
using MPVector3 = lwVector3;
using MPMatrix44 = lwMatrix44;
using MPBoundingBoxInfo = lwBoundingBoxInfo;
using MPPickInfo = lwPickInfo;
using MPPlayPoseInfo = lwPlayPoseInfo;
using MPPoseInfo = lwPoseInfo;
using MPInterfaceMgr = lwInterfaceMgr;
using MPMeshInfo = lwMeshInfo;
using MPWatchDevVideoMemInfo = lwWatchDevVideoMemInfo;

using MPIAnimCtrl = lwIAnimCtrl;
using MPIAnimCtrlAgent = lwIAnimCtrlAgent;
using MPIAnimCtrlBone = lwIAnimCtrlBone;
using MPIAnimCtrlObjBone = lwIAnimCtrlObjBone;
using MPIBoundingBox = lwIBoundingBox;
using MPIDeviceObject = lwIDeviceObject;
using MPStaticStreamMgrDebugInfo = lwStaticStreamMgrDebugInfo;
using MPD3DCreateParamAdjustInfo = lwD3DCreateParamAdjustInfo;
using MPDwordByte4 = lwDwordByte4;
using MPIHelperObject = lwIHelperObject;
using MPIMesh = lwIMesh;
using MPIPathInfo = lwIPathInfo;
using MPIPhysique = lwIPhysique;
using MPIPoseCtrl = lwIPoseCtrl;
using MPIPrimitive = lwIPrimitive;
using MPIRenderStateAtomSet = lwIRenderStateAtomSet;
using MPIResBufMgr = lwIResBufMgr;
using MPIResourceMgr = lwIResourceMgr;
using MPISceneMgr = lwISceneMgr;
using MPIStaticStreamMgr = lwIStaticStreamMgr;
using MPISysGraphics = lwISysGraphics;
using MPISystemInfo = lwISystemInfo;
using MPISystem = lwISystem;
using MPITex = lwITex;
using MPITimer = lwITimer;
using MPITimerPeriod = lwITimerPeriod;

// macro
#ifndef MP_NEW
#define MP_NEW LW_NEW
#endif

#ifndef MP_DELETE
#define MP_DELETE LW_DELETE
#endif

#ifndef MP_DELETE_A
#define MP_DELETE_A LW_DELETE_A
#endif

// method

#ifndef MPGUIDCreateObject
#define MPGUIDCreateObject lwGUIDCreateObject
#endif

#ifndef MPMatrix44Multiply
#define MPMatrix44Multiply lwMatrix44Multiply
#endif

#ifndef MPRegisterOutputLoseDeviceProc
#define MPRegisterOutputLoseDeviceProc lwRegisterOutputLoseDeviceProc
#endif

#ifndef MPRegisterOutputResetDeviceProc
#define MPRegisterOutputResetDeviceProc lwRegisterOutputResetDeviceProc
#endif

#ifndef MPVec3Mat44Mul
#define MPVec3Mat44Mul lwVec3Mat44Mul
#endif

#ifndef MPVector3Normalize
#define MPVector3Normalize lwVector3Normalize
#endif

#ifndef MPVector3Slerp
#define MPVector3Slerp lwVector3Slerp
#endif

#ifndef MPVector3Sub
#define MPVector3Sub lwVector3Sub
#endif
