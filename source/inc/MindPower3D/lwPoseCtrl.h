//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"

LW_BEGIN

// by lsh
// 这个版本的PlayPose和CallBack对PLAY_LOOP标志下的关键帧做过测试（减速，正常，加速）
///////////////////////////

class lwPoseCtrl : public lwIPoseCtrl {

private:
	lwPoseInfo* _pose_seq;
	DWORD _pose_num;
	DWORD _frame_num;

	LW_STD_DECLARATION()

private:
	LW_RESULT _Update(DWORD pose, DWORD play_type, float velocity, float* io_frame, float* o_ret_frame);

public:
	lwPoseCtrl() : _pose_seq(nullptr), _pose_num(0), _frame_num(0xffffffff) {}
	~lwPoseCtrl() { LW_SAFE_DELETE_A(_pose_seq); }

	LW_RESULT Load(const char* file) override;
	LW_RESULT Save(const char* file) const override;
	LW_RESULT Load(FILE* fp);
	LW_RESULT Save(FILE* fp) const;

	LW_RESULT Copy(const lwPoseCtrl* src);
	LW_RESULT Clone(lwIPoseCtrl** obj);

	DWORD GetDataSize() const;

	LW_RESULT InsertPose(DWORD id, const lwPoseInfo* pi, int num) override;
	LW_RESULT ReplacePose(DWORD id, const lwPoseInfo* pi) override;
	LW_RESULT RemovePose(DWORD id) override;
	LW_RESULT RemoveAll() override;
	LW_RESULT GetPoseInfoBuffer(lwPoseInfo** buf);

	void SetFrameNum(int frame) override { _frame_num = frame; }
	DWORD GetPoseNum() const override { return _pose_num; }
	lwPoseInfo* GetPoseInfo(DWORD id) override { return (id >= _pose_num) ? nullptr : &_pose_seq[id]; }

	DWORD GetPoseFrameNum(DWORD id) const override { return (id >= _pose_num) ? 0 : (_pose_seq[id].end - _pose_seq[id].start + 1); }
	int IsPosePlaying(const lwPlayPoseInfo* info) const override;

	LW_RESULT PlayPose(lwPlayPoseInfo* info) override;
	LW_RESULT CallBack(const lwPlayPoseInfo* info);
};

///////////////////////////

LW_RESULT lwPlayPoseSmooth(lwPlayPoseInfo* dst, const lwPlayPoseInfo* src, lwIPoseCtrl* ctrl);

LW_END