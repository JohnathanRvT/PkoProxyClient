#ifndef SCENEAREA_H
#define SCENEAREA_H

#define SCENE_AREA_FILE_VER100 100

struct SAreaUnit {
	short siType; // 1：普通陆地，2：普通海洋
};

class CSceneArea {
	struct SFileHead {
		char tcsTitle[32]; // "HF Scene Area File!"
		int lVersion;
		long lFileSize;

		int iSceneWidth; // 单位：Tile
		int iSceneHeight;
	};

public:
	CSceneArea();
	~CSceneArea();
	long Init(char* ptcsAreaFile, bool bSilence = true);
	void Free();
	long CreateFile(char* ptcsAreaFile, int iSceneWidth = 4096, int iSceneHeight = 4096);
	long CreateFileFromMap(char* ptcsMapFile, char* ptcsAreaFile);
	long ReadAreaInfo(long lUnitNO, SAreaUnit* pUnitData, long* lpUnitNum);
	long WriteAreaInfo(long lUnitNO, SAreaUnit* pUnitData, long* lpUnitNum);

private:
	bool m_bInitSuccess;
	long m_lUnitNum;
	FILE* m_fRdWr;
	SFileHead m_SFileHead;
};

#endif //SCENEAREA_H
