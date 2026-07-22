//
#include "lwfileutil.h"
#include <assert.h>

/////////////////////////////////////////////

char* lwGetModuleBaseFileName(char* buf) {
	char tmp[256];

	GetModuleFileName(nullptr, tmp, 256);

	return lwGetPathFileNameBase(buf, tmp);
}

char* lwGetModuleFilePath(char* buf) {
	GetModuleFileName(nullptr, buf, 256);

	char* p = buf + strlen(buf);

	for (; *p != '//' && *p != '\\'; --p)
		;
	*p = 0;

	return buf;
}

char* lwGetPathFileName(char* buf, const char* path) {
	const char* TempStr = strrchr(path, '\\') + 1;
	strncpy_s(buf, strlen(TempStr) + 1, TempStr, _TRUNCATE);

	return buf;
}

char* lwGetPathFileNameBase(char* buf, const char* path) {
	int i;

	const char* p = path + strlen(path) - 1;

	for (; *p != '\\'; --p)
		;

	for (i = 0, p += 1; *p != '.'; p++) {
		buf[i++] = *p;
	}

	buf[i] = '\0';

	return buf;
}

char* lwGetPathFileNameExt(char* buf, const char* path) {
	char* p = (char*)strrchr(path, '.');
	if (p == nullptr)
		return nullptr;

	p++;
	strncpy_s(buf, strlen(p) + 1, p, _TRUNCATE);
	return buf;
}

char* lwGetPathFilePath(char* buf, const char* path) {
	strncpy_s(buf, strlen(path) + 1, path, _TRUNCATE);
	char* p = strrchr(buf, '\\');
	*p = '\0';

	return buf;
}

char* lwGetPathSubPath(char* sub_path, const char* path, const char* compare_path) {
	char p;
	char c;
	const char* ptr_p = path;
	const char* ptr_c = compare_path;

	for (; ((*ptr_c) != '\0') && ((*ptr_p) != '\0'); ptr_c++, ptr_p++) {

		c = (*ptr_c);
		p = (*ptr_p);

		if ((c >= 0x41) && (c <= 0x5a)) {
			c += 0x20;
		}
		if ((p >= 0x41) && (p <= 0x5a)) {
			p += 0x20;
		}

		if (c != p)
			return nullptr;
	}

	if (*ptr_p == 0)
		return nullptr;

	if (*ptr_p == '\\') {
		if (++ptr_p == nullptr)
			return nullptr;
	}

	strncpy_s(sub_path, strlen(ptr_p) + 1, ptr_p, _TRUNCATE);

	return sub_path;
}

char* lwGetIndexFileName(char* pri_name, const char* file, int id_len) {
	size_t len = strlen(file) - id_len;
	if (len <= 0)
		return nullptr;

	strncpy_s(pri_name, len, file, _TRUNCATE);
	pri_name[len] = 0;

	return pri_name;
}

//
int lwGetOpenFileName(HWND hwnd, char* buf, int num, const char* dir, const char* title, const char* filter, int flag) {
	OPENFILENAME ofn; // common dialog box structure
	memset(&ofn, 0, sizeof(ofn));

	//char ret_buf[1024]={"\0"};
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = num;
	ofn.lpstrFilter = filter;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = dir;
	ofn.Flags = flag;
	ofn.nFilterIndex = 1;
	// Display the Open dialog box.
	if (GetOpenFileName(&ofn) != TRUE)
		return 0;

	// MultiSelected style , break file and directory
	if (flag & OFN_FILEMUSTEXIST) {
		*(ofn.lpstrFile + ofn.nFileOffset - 1) = '\0';
	}

	//strncpy_s(buf,ofn.lpstrFile,_TRUNCATE);
	return 1;
}

int lwGetSaveFileName(HWND hwnd, char* buf, int num, const char* dir, const char* title, const char* filter, const char* ext, int flag) {
	OPENFILENAME ofn; // common dialog box structure
	memset(&ofn, 0, sizeof(OPENFILENAME));

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = num;
	ofn.lpstrFilter = filter;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = dir;
	ofn.lpstrDefExt = ext;
	ofn.Flags = flag;
	ofn.nFilterIndex = 1;
	// Display the Open dialog box.
	if (GetSaveFileName(&ofn) != TRUE)
		return 0;

	return 1;
}

//
// function not tested
int lwGetFolderName(char* folder, HWND hwnd, const char* title, LPITEMIDLIST pid_root) {
	char buf[MAX_PATH];
	LPITEMIDLIST pid = pid_root;

	if (pid == nullptr)
		::SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &pid);

	BROWSEINFO bi;
	bi.hwndOwner = hwnd;
	bi.pidlRoot = pid;
	bi.pszDisplayName = buf;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_EDITBOX;
	bi.lpfn = nullptr;
	bi.lParam = 0l;
	bi.iImage = 0;

	LPITEMIDLIST item_list = ::SHBrowseForFolder(&bi);

	if (item_list)
		::SHGetPathFromIDList(item_list, folder);

	LPMALLOC pMalloc = nullptr;
	::SHGetMalloc(&pMalloc);
	(pMalloc)->Free(pid);

	return item_list ? 1 : 0;
}

//load image file

HBITMAP LoadBitmapFile(const char* file) {
	HBITMAP bmp = (HBITMAP)LoadImage(GetModuleHandle(nullptr),
									 file,
									 IMAGE_BITMAP, 0, 0,
									 LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	return bmp;
}
OLE_HANDLE LoadImageFile(const char* file) {
	// open file
	HANDLE hFile = CreateFile(file, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	assert(INVALID_HANDLE_VALUE != hFile);

	// get file size
	DWORD dwFileSize = GetFileSize(hFile, nullptr);
	assert(-1 != dwFileSize);

	LPVOID pvData = nullptr;
	// alloc memory based on file size
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
	assert(NULL != hGlobal);

	pvData = GlobalLock(hGlobal);
	assert(NULL != pvData);

	DWORD dwBytesRead = 0;
	// read file and store in global memory
	BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, nullptr);
	assert(FALSE != bRead);
	GlobalUnlock(hGlobal);
	CloseHandle(hFile);

	LPSTREAM pstm = nullptr;
	// create IStream* from global memory
	HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
	assert(SUCCEEDED(hr) && pstm);

	// Create IPicture from image file
	LPPICTURE gpPicture;

	hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID*)&gpPicture);
	assert(SUCCEEDED(hr) && gpPicture);
	pstm->Release();

	OLE_HANDLE m_picHandle = 0;

	gpPicture->get_Handle(&m_picHandle);

	return m_picHandle;
}
