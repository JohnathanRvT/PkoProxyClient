#include "pi_Alloc.h"

#include <iostream>
#include <fstream>

static int s_numBlocks = 0;
static size_t s_numBytes = 0;

static void oneMoreBlock(size_t bytes) {
	++s_numBlocks;
	s_numBytes += bytes;
}

static void oneLessBlock(size_t bytes) {
	--s_numBlocks;
	s_numBytes -= bytes;
}

int pi_Alloc::numBlocks() {
	return s_numBlocks;
}

int pi_Alloc::numBytes() {
	return (int)s_numBytes;
}

union Align {
	struct
	{
		int d_magic;
		int d_bytes;
	} d_data;
	long int d_longInt;
	long double d_longDouble;
	char* d_pointer;
};

enum {
	ALLOCAED_MEMORY = 0xA110CAED,
	DEALLOCATED_MEMORY = ~ALLOCAED_MEMORY
};

#ifdef LOG_POSITION
void* pi_Alloc::allocate(size_t size, pi_Alloc*, const char* fileName, int lineNo)
#else
void* pi_Alloc::allocate(size_t size, pi_Alloc*)
#endif
{
	Align* align = (Align*)new char[sizeof(Align) + size];
	align->d_data.d_magic = ALLOCAED_MEMORY;
	align->d_data.d_bytes = (int)size;

	void* addr = ++align;
	oneMoreBlock(size);

#ifdef LOG_POSITION
	pi_leakReporter.addItem(addr, fileName, lineNo, size);
#endif
	return addr;
}

static void report(const void* addr, const char* gerund) {
	std::cerr << "po: PROGRAMMING ERROR -- " << gerund << " ";
	if (!addr)
		std::cerr << "null memory address";
	else {
		Align* align = (Align*)addr;
		if (ALLOCAED_MEMORY == align->d_data.d_magic)
			std::cerr << "previously deallocated object";
		else
			std::cerr << "unallocated memory address";
	}

	std::cerr << std::endl;
}

void pi_Alloc::assertValid(void* addr) {
	if (!isAllocated(addr))
		report(addr, "using");
}

void pi_Alloc::deallocate(void* addr) {
	if (!isAllocated(addr)) {
		report(addr, "deallocating");
		return;
	}

	Align* align = (Align*)addr;
	int size = align[-1].d_data.d_bytes;
	oneLessBlock(size);
	align[-1].d_data.d_magic = DEALLOCATED_MEMORY;

#ifdef LOG_POSITION
	pi_leakReporter.removeItem(addr);
#endif

	delete[](char*)-- align;
}

int pi_Alloc::isAllocated(void* addr) {
	if (!addr)
		return 0;

	Align* align = (Align*)addr;
	return (ALLOCAED_MEMORY == align[-1].d_data.d_magic);
}

pi_LeakReporter::pi_LeakReporter(const char* fileName) {
	pi_fileName = fileName;
}

pi_LeakReporter::~pi_LeakReporter() {
	int b = pi_Alloc::numBlocks();
	int y = pi_Alloc::numBytes();

	if (b > 0 || y > 0) {
		std::cerr << "pi :MEMORY LEAK --"
			 << b << " block" << (b != 1 ? "s" : "") << ","
			 << y << " byte" << (y != 1 ? "s" : "") << std::endl;
	}

	dumpReport();
}

void pi_LeakReporter::addItem(void* addr, const char* fileName, int lineNo, size_t size) {
	pi_LeakItem item;
	item.addr = reinterpret_cast<intptr_t>(addr); // C4311 64-bit portability issues

	// CRT 7.1
#if (_MSC_VER >= 1400)
	strncpy_s(item.fileName, sizeof(item.fileName), fileName, _TRUNCATE);
#else
	strncpy(item.fileName, fileName, sizeof(item.fileName));
#endif
	item.d_lineNo = lineNo;
	item.d_size = size;

	pi_leakList.push_back(item);
}

void pi_LeakReporter::removeItem(void* addr) {
	for (auto it = pi_leakList.begin(); it != pi_leakList.end(); it++) {
		if (it->addr == reinterpret_cast<intptr_t>(addr)) // C4311 64-bit portability issues)
		{
			pi_leakList.erase(it);
			break;
		}
	}
}

bool pi_LeakReporter::isLeak() {
	return (!pi_leakReporter.pi_leakList.empty());
}

void pi_LeakReporter::dumpReport() {
	if (!pi_leakReporter.pi_leakList.empty()) {
#ifdef LOG_POSITION
		std::cout << "memory leak begin" << std::endl;

		int i = 0;

		for (auto it = pi_leakReporter.pi_leakList.begin(); it != pi_leakReporter.pi_leakList.end(); it++, i++) {
			std::cout << "\t" << (i + 1) << ":" << it->fileName << " size=" << ((it->d_size > 1024) ? (it->d_size / 1024.0) : it->d_size)
				 << ((it->d_size > 1024) ? " kbytes (" : " bytes(") << it->d_lineNo << ")" << std::endl;
		}

		std::cout << "memory leak end" << std::endl;
#else

#endif

		//cout << "save to file (c:\\memoryleak.log)?(Y/N)" << endl;
		//char key;
		//cin >> key;

		//if(key == 'y' || key == 'Y')
		{
			try {
				std::fstream outputFile;

				outputFile.open(pi_fileName.c_str(), std::ios::out);
				outputFile << "memory leak begin" << std::endl;

				int i = 0;

				for (auto it = pi_leakReporter.pi_leakList.begin(); it != pi_leakReporter.pi_leakList.end(); it++, i++) {
					outputFile << "\t" << (i + 1) << ":" << it->fileName << " size=" << ((it->d_size > 1024) ? (it->d_size / 1024.0) : it->d_size)
							   << ((it->d_size > 1024) ? " kbytes (" : " bytes(") << it->d_lineNo << ")" << std::endl;
				}

				outputFile << "memory leak end" << std::endl;

				outputFile.close();
			} catch (...) {
			}
		}
	}
}