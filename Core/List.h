﻿#ifndef _List_H_
#define _List_H_

typedef int (*IComparer)(const void* v1, const void* v2);

// 变长列表。仅用于存储指针
class List
{
public:
    IComparer	Comparer;	// 比较器

	List();
    List(const List& list);
    List(List&& list);
	~List();

	int Count() const;

	// 添加单个元素
    void Add(void* item);

	// 添加多个元素
    void Add(void** items, uint count);

	// 删除指定位置元素
	void RemoveAt(uint index);

	// 删除指定元素
	int Remove(const void* item);

	void Clear();

	// 查找指定项。不存在时返回-1
	int FindIndex(const void* item) const;

	// 释放所有指针指向的内存
	List& DeleteAll();

    // 重载索引运算符[]，返回指定元素的第一个
    void* operator[](int i) const;
    void*& operator[](int i);

#if DEBUG
	static void Test();
#endif

private:
	void**	_Arr;
	uint	_Count;
	uint	_Capacity;

	void*	Arr[0x04];

	void Init();
	bool CheckCapacity(int count);
};

template<typename T>
class TList : public List
{
	static_assert(sizeof(T) <= 4, "TList only support pointer or int");
public:
	// 添加单个元素
    void Add(T item) { List::Add(item); }

	// 删除指定元素
	int Remove(const T item) { return List::Remove(item); }

	// 查找指定项。不存在时返回-1
	int FindIndex(const T item) const { return List::FindIndex(item); }

    // 重载索引运算符[]，返回指定元素的第一个
    T operator[](int i) const	{ return (T)List::operator[](i); }
    T& operator[](int i)		{ return (T&)List::operator[](i); }
};

#endif
