#include "stdafx.h"
#include "PicSet.h"
#include "rng.h"

CPicSet::CPicSet() {
}

CPicSet::~CPicSet() {}

bool CPicSet::init() {
	std::array<char, ID_MAX> ids = {'2', '3', '4', '5', '6', '7', '9', 'A', 'C', 'E', 'F', 'H', 'K', 'L', 'M', 'N', 'P', 'R', 'T', 'U', 'V', 'X', 'Y', 'Z'};
	m_ids = ids;

	for (auto id : ids) {
		std::ostringstream oss;
		oss << "Pic/" << id << id << ".bmp";
		const auto strFileName = GetResPath(oss.str().c_str());
		auto pic = std::make_unique<CPicture>(strFileName);
		if (pic->LoadImg()) {
			pic->SetID(id);
			m_mapList.emplace(id, std::move(pic));
		} else {
			return false;
		}
	}
	return true;
}

char CPicSet::RandGetID() {
	
	int index = rng.uniform(0, ID_MAX - 1);
	return m_ids[index];
}

CPicture* CPicSet::GetPicture(char id) {
	auto it = m_mapList.find(id);
	return it != m_mapList.end() ? it->second.get() : nullptr;
}
