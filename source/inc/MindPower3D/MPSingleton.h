#ifndef MPSingleton_H
#define MPSingleton_H

/** 模板类用于创建单例实体
*/
template <typename T>
class MPSingleton {
protected:
	static T* m_sSingleton;

public:
	MPSingleton() {
		assert(!m_sSingleton);
#if defined(_MSC_VER) && _MSC_VER < 1200
		int offset = (int)(T*)1 - (int)(MPSingleton<T>*)(T*)1;
		m_sSingleton = (T*)((int)this + offset);
#else
		m_sSingleton = static_cast<T*>(this);
#endif
	}

	~MPSingleton() {
		assert(ms_Singleton);
		m_sSingleton = 0;
	}

	static T& getSingleton() {
		assert(ms_Singleton);
		return (*m_sSingleton);
	}

	static T* getSingletonPtr() {
		return m_sSingleton;
	}
};

#endif