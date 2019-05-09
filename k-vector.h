#pragma once
#include <initializer_list>
#include <utility>
#include <cstddef>
#include <iostream>
#include <algorithm>

template <class T> class KVector
{
private:

	void swap(KVector<T>& right)
	{
		std::swap(_alloc_size, right._alloc_size);
		std::swap(_sz, right._sz);
		std::swap(_array, right._array);
	}

	void shiftRight(T* source, T* dest, size_t sz)
	{
		//source pointer < dest pointer
		//chances of overwrite
		T* d = dest + sz - 1;
		T* s = source + sz - 1;
		size_t n = sz;
		while (n--) {
			*d = std::move(*s); //move semantics for fast copy
			d--; s--;
		}
	}

	void shiftLeft(T* source, T* dest, size_t sz)
	{
		//dest pointer < source pointer
		T* d = dest;
		T* s = source;
		size_t n = sz;
		while (n--) {
			*d = std::move(*s); //move semantics for fast copy
			d++; s++;
		}
	}

	void inflateAlloc(size_t newSz) {
		_alloc_size = newSz;
		T* narray = nullptr; 
		if (narray < _array) {
			narray = new T[_alloc_size]; 
			shiftLeft(_array, narray, _sz);
			delete[] _array;
			_array = narray;
		}
		else if (narray > _array) {
			narray = new T[_alloc_size]; 
			shiftRight(_array, narray, _sz);
			delete[] _array;
			_array = narray;
		}
	}
public:
	/************* CONSTRUCTORS ****************/
	KVector() {
		// Default allocation
		_sz = 0;
		_array = new T[_alloc_size];
	}

	KVector(size_t sz) {
		// Default allocation
		_array = new T[sz + sz / 2 + 1];
		_sz = sz + sz / 2 + 1;
		for (size_t i = 0; i < sz; i++)
			_array[i] = T(); //Template Object default constructor
	}

	//Brace Initializer list constructor
	KVector(std::initializer_list<T> lst)
	{
		size_t i = lst.size();
		_alloc_size = i + i/2 + 1;
		_array = new T[_alloc_size];
		_sz = 0; // start from 0
		// use std::move semantics for fast copy (no copy constructor)
		for (const T& n : lst)
			_array[_sz++] = std::move(n); 
	}

	// move constructor
	KVector(KVector<T>&& r) : _sz(0), _alloc_size(0), _array(nullptr) {
		_sz = r._sz;
		_alloc_size = r._alloc_size;
		_array = r._array;

		r._sz = 0;
		r._alloc_size = 0;
		r._array = nullptr;
	}
	

	//Copy CTORS - lvalue versions
	KVector(const KVector & right)
	{
		_sz = right._sz;
		_alloc_size = right._alloc_size;
		_array = new T[_alloc_size];
		for (size_t i = 0; i < _sz; i++)
			_array[i] = right._array[i];
	}

	/*************** INTERFACES *******************/

	//emplaceback using parameter pack
	template<class ...Args>
	T& EmplaceBack(Args && ...args) {
		if (_alloc_size == _sz)
		{
			//_alloc_size += _alloc_size/2 + 1; //choose default size increase
			inflateAlloc(_sz + _sz / 2 + 1);
		}
		//Assuming move has base case implementation
		return _array[_sz++] = std::move(T(std::forward<Args>(args) ...));
	}

	//lvalue argument
	void PushBack(const T & lval) {
		EmplaceBack(lval);
	}

	//rvalue argument
	void PushBack(T && rval) {
		EmplaceBack(std::forward<T>(rval));
	}

	template<class ...Args>
	size_t Emplace(T * iter, Args && ... args) {
		size_t position = iter - _array;
		T* tempiter = iter;
		//check space availability
		if (_alloc_size == _sz)
		{
			inflateAlloc(_sz + _sz / 2 + 1);
		}
		shiftRight(tempiter, tempiter + 1, _sz - position);
		_sz++;
		*tempiter = std::move(T(std::forward<Args>(args) ...));
		return position;
	}

	size_t InsertAt(size_t pos, const T & lval) {
		T* iter = &_array[pos];
		return Emplace(iter, lval);
	}

	size_t InsertAt(size_t pos, T && rval) {
		T* iter = &_array[pos];
		return Emplace(iter, std::forward<T>(rval));
	}

	size_t EraseAt(size_t pos) {
		T* ptr = &_array[pos];
		ptr->~T();
		//Call shiftLeft on the rest of arrays towards the right
		shiftLeft(ptr + 1, ptr, _sz - pos);
		_sz -= 1;
		return pos;
	}

	~KVector() { 
		if (_array != nullptr) delete[] _array;
	}

	void swap(KVector<T> & left, KVector<T> & right)
	{
		left.swap(right);
	}

	/************************ ACCESSORS *************************/
	T& Front()
	{
		return _array[0];
	}

	const T& Front() const
	{
		return _array[0];
	}

	T& Back()
	{
		return _array[_sz - 1];
	}

	const T& Back() const
	{
		return _array[_sz - 1];
	}

	/*********************** OPERATORS **************************/

	//Assignment operator
	//lvalue operand
	KVector<T>& operator=(const KVector<T> & lvect)
	{
		swap(KVector<T>(lvect));
		return *this;
	}

	//rvalue operand
	KVector<T>& operator=(KVector<T> && rvect)
	{
		swap(rvect);
		return *this;
	}

	KVector<T>& operator=(std::initializer_list<T> lst)
	{
		//cast to KVector using initializer list CTOR version
		swap(KVector<T>(lst));
		return *this;
	}

	//Subscript operator
	T& operator[](size_t idx) {
		return _array[idx];
	}

	const T& operator[](size_t idx) const { 
		return _array[idx];
	}

	//TODO
	bool IsEmpty() { return _sz == 0; }
	size_t Size() { return _sz; }
	void Clear() {
		for (size_t i = 0; i < _sz; i++) {
			_array[i].~T(); // call element DTOR
		}
		_sz = 0; // zeroize KVector size
	}

	/******************* NON-MEMBER OPERATOR DECLARATIONS **************************/
	template<class U> friend bool operator==(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend bool operator!=(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend bool operator<(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend bool operator<=(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend bool operator>(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend bool operator>=(const KVector<U> & l, const KVector<U> & r);
	template<class U> friend void swap(KVector<U> & l, KVector<U> & r);//TODO: exist?

	//IOSTREAM operator
	//template<class U> friend std::ostream& operator<< (std::ostream & os, const KVector<U> & obj);


private:
	using list_iter = typename std::initializer_list<T>::iterator;
	//new size to be reserved each time
	size_t _alloc_size = 20;
	//actual size of Kvector at anytime
	size_t _sz;
	//Array pointer to internal memory
	T* _array;
};

//All friends are declared inline if compiler begs to differ
template<class T>
inline bool operator==(const KVector<T> & l, const KVector<T> & r)
{
	if (l._sz != r._sz) return false;
	//Check member wise for inequality
	for (size_t i = 0; i < l._sz; i++) {
		if (l._array[i] != r._array[i])
			return false;
	}
	return true;
}

template<class T>
inline bool operator!=(const KVector<T> & l, const KVector<T> & r)
{
	return !(l == r); //invokes friend !operator==()
}

template<class T>
inline bool operator<(const KVector<T> & l, const KVector<T> & r)
{
	size_t n;
	//whichever is less
	if (l._sz < r._sz) n = l._sz;
	else n = r._sz;

	//give preference to member size
	for (size_t i = 0; i < n; i++) {
		if (l._array[i] != r._array[i])
			return l._array[i] < r._array[i];
	}
	//else KVector size
	return l._sz < r._sz;
}

template<class T>
inline bool operator>(const KVector<T> & l, const KVector<T> & r)
{
	size_t n;
	//whichever is less
	if (l._sz < r._sz) n = l._sz;
	else n = r._sz;

	//give preference to member size
	for (size_t i = 0; i < n; i++) {
		if (l._array[i] != r._array[i])
			return l._array[i] > r._array[i];
	}
	//else KVector size
	return l._sz > r._sz;
}

template<class T>
inline bool operator<=(const KVector<T> & l, const KVector<T> & r)
{
	return !(l > r);
}

template<class T>
inline bool operator>=(const KVector<T> & l, const KVector<T> & r)
{
	return !(l < r);
}

template<class T>
inline void swap(KVector<T> & l, KVector<T> & r)
{
	l.swap(r);
}

/*
template<class T>
inline std::ostream& operator<< (std::ostream & os, const KVector<T> & obj) {
	os << "_sz: " << obj._sz << std::endl;
	for (size_t i = 0; i < obj._sz; i++)
		os << obj._array[i] << " ";
	os << std::endl;
	return os;
}
*/