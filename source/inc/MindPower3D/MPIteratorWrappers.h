#ifndef MPIteratorWrappers_H
#define MPIteratorWrappers_H

template <class T>
class MPMapIterator {
private:
	typename T::iterator mCurrent;
	typename T::iterator mEnd;
	/// 私有构造函数，因为仅必须提供参数
	MapIterator(){};

public:
	using MappedType = typename T::mapped_type;
	using KeyType = typename T::key_type;

	/** 构造函数.
	@remarks
		提供start和end迭代器用于初始化.
	*/
	MapIterator(typename T::iterator start, typename T::iterator end)
		: mCurrent(start), mEnd(end) {
	}

	bool hasMoreElements() const {
		return mCurrent != mEnd;
	}

	typename T::mapped_type getNext() {
		return (mCurrent++)->second;
	}
	typename T::mapped_type peekNextValue() {
		return mCurrent->second;
	}
	typename T::key_type peekNextKey() {
		return mCurrent->first;
	}
	MapIterator<T>& operator=(MapIterator<T>& rhs) {
		mCurrent = rhs.mCurrent;
		mEnd = rhs.mEnd;
		return *this;
	}
	typename T::pointer peekNextValuePtr() {
		return &(mCurrent->second);
	}
	void moveNext() {
		mCurrent++;
	}
};

#endif