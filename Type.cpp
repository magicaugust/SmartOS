﻿#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#include "Sys.h"
//#include "Type.h"
//#include "String.h"

/******************************** Object ********************************/

// 输出对象的字符串表示方式
String& Object::ToStr(String& str) const
{
	auto name = typeid(*this).name();
	while(*name >= '0' && *name <= '9') name++;

	str	+= name;

	return str;
}

// 输出对象的字符串表示方式。支持RVO优化。
// 该方法直接返回给另一个String作为初始值，只有一次构造，没有多余构造、拷贝和析构。
String Object::ToString() const
{
	String str;
	ToStr(str);

	return str;
}

void Object::Show(bool newLine) const
{
	// 为了减少堆分配，采用较大的栈缓冲区
	char cs[0x200];
	String str(cs, ArrayLength(cs));
	ToStr(str);
	str.Show(newLine);
}

const Type Object::GetType() const
{
	auto p = (int*)this;

	return Type(&typeid(*this), *(p - 1));
}

/******************************** Type ********************************/

Type::Type(const type_info* ti, int size)
{
	_info	= ti;

	Size	= size;
}

const String Type::Name() const
{
	auto name = _info->name();
	while(*name >= '0' && *name <= '9') name++;

	return String(name);
}

/******************************** Buffer ********************************/

Buffer::Buffer(void* ptr, int len)
{
	//assert_param2(ptr && len > 0, "Buffer构造指针不能为空！");

	_Arr	= (char*)ptr;
	_Length	= len;
}

/*Buffer::Buffer(const Buffer& buf)
{
	_Arr	= buf._Arr;
	_Length	= buf._Length;
}*/

Buffer::Buffer(Buffer&& rval)
{
	move(rval);
}

void Buffer::move(Buffer& rval)
{
	_Arr	= rval._Arr;
	_Length	= rval._Length;

	rval._Arr		= nullptr;
	rval._Length	= 0;
}

Buffer& Buffer::operator = (const Buffer& rhs)
{
	Copy(0, rhs, 0, -1);

	return *this;
}

Buffer& Buffer::operator = (const void* ptr)
{
	if(ptr) Copy(0, ptr, -1);

	return *this;
}

Buffer& Buffer::operator = (Buffer&& rval)
{
	move(rval);

	return *this;
}

// 重载索引运算符[]，返回指定元素的第一个字节
byte Buffer::operator[](int i) const
{
	assert_param2(i >= 0 && i < _Length, "下标越界");

	byte* buf = (byte*)_Arr;

	return buf[i];
}

byte& Buffer::operator[](int i)
{
	assert_param2(i >= 0 && i < _Length, "下标越界");

	byte* buf = (byte*)_Arr;

	return buf[i];
}

// 设置数组长度。容量足够则缩小Length，否则失败。子类可以扩展以实现自动扩容
bool Buffer::SetLength(int len, bool bak)
{
	if(len > _Length) return false;

	_Length = len;

	return true;
}

// 拷贝数据，默认-1长度表示当前长度
int Buffer::Copy(int destIndex, const void* src, int len)
{
	if(!src) return 0;

	int remain	= _Length - destIndex;

	// 如果没有指明长度，则拷贝起始位置之后的剩余部分
	if(len < 0 )
		len	= remain;
	else if(len > remain)
	{
		// 要拷贝进来的数据超过内存大小，给子类尝试扩容，如果扩容失败，则只拷贝没有超长的那一部分
		// 子类可能在这里扩容
		if(!SetLength(destIndex + len, true)) len	= remain;
	}

	// 放到这里判断，前面有可能自动扩容
	if(!_Arr) return 0;

	// 拷贝数据
	if(len) memcpy((byte*)_Arr + destIndex, src, len);

	return len;
}

// 拷贝数据，默认-1长度表示两者最小长度
int Buffer::Copy(int destIndex, const Buffer& src, int srcIndex, int len)
{
	// 允许自身拷贝
	// 源数据的实际长度可能跟要拷贝的长度不一致
	int remain	= src._Length - srcIndex;
	if(len > remain) len	= remain;
	if(len <= 0) return 0;

	return Copy(destIndex, (byte*)src._Arr + srcIndex, len);
}

// 把数据复制到目标缓冲区，默认-1长度表示当前长度
int Buffer::CopyTo(int srcIndex, void* data, int len) const
{
	if(!_Arr || !data) return 0;

	int remain	= _Length - srcIndex;
	if(remain <= 0) return 0;

	if(len < 0 || len > remain) len	= remain;

	// 拷贝数据
	if(len) memcpy(data, (byte*)_Arr + srcIndex, len);

	return len;
}

// 用指定字节设置初始化一个区域
int Buffer::Set(byte item, int index, int len)
{
	if(!_Arr || len == 0) return 0;

	int remain	= _Length - index;
	if(remain <= 0) return 0;

	if(len < 0 || len > remain) len	= remain;

	if(len) memset((byte*)_Arr + index, item, len);

	return len;
}

void Buffer::Clear(byte item)
{
	Set(item, 0, _Length);
}

// 截取一个子缓冲区
Buffer Buffer::Sub(int index, int len)
{
	assert_param2(index + len <= _Length, "len <= _Length");

	return Buffer((byte*)_Arr + index, len);

	/*// 预留子类自动扩容
	Buffer bs((byte*)_Arr + index, 0);
	// 如果不支持自动扩容，这里会失败
	if(!bs.SetLength(len)) bs._Length	= len;
	return false;*/
}

const Buffer Buffer::Sub(int index, int len) const
{
	assert_param2(index + len <= _Length, "len <= _Length");

	return Buffer((byte*)_Arr + index, len);

	/*// 预留子类自动扩容
	Buffer bs((byte*)_Arr + index, 0);
	// 如果不支持自动扩容，这里会失败
	if(!bs.SetLength(len)) bs._Length	= len;
	return false;*/
}


// 显示十六进制数据，指定分隔字符
String& Buffer::ToHex(String& str, char sep, int newLine) const
{
	auto buf = GetBuffer();

	// 拼接在字符串后面
	for(int i=0; i < Length(); i++, buf++)
	{
		if(i)
		{
			if(newLine > 0 && i % newLine == 0)
				str += "\r\n";
			else if(sep != '\0')
				str += sep;
		}

		str.Concat(*buf, -16);
	}

	return str;
}

// 显示十六进制数据，指定分隔字符
String Buffer::ToHex(char sep, int newLine) const
{
	String str;

	// 优化为使用RVO
	ToHex(str, sep, newLine);

	return str;
}

String& Buffer::ToStr(String& str) const
{
	return ToHex(str, '-', 0x20);
}

bool operator == (const Buffer& bs1, const Buffer& bs2)
{
	if(bs1._Arr == bs2._Arr) return true;
	if(!bs1._Arr || !bs2._Arr) return false;
	if(bs1.Length() != bs2.Length()) return false;

	return memcmp(bs1._Arr, bs2._Arr, bs1.Length()) == 0;
}

bool operator == (const Buffer& bs1, const void* ptr)
{
	if(bs1._Arr == ptr) return true;
	if(!bs1._Arr || !ptr) return false;

	return memcmp(bs1._Arr, ptr, bs1.Length()) == 0;
}

bool operator != (const Buffer& bs1, const Buffer& bs2)
{
	if(bs1._Arr == bs2._Arr) return false;
	if(!bs1._Arr || !bs2._Arr) return true;
	if(bs1.Length() != bs2.Length()) return true;

	return memcmp(bs1._Arr, bs2._Arr, bs1.Length()) != 0;
}

bool operator != (const Buffer& bs1, const void* ptr)
{
	if(bs1._Arr == ptr) return false;
	if(!bs1._Arr || !ptr) return true;

	return memcmp(bs1._Arr, ptr, bs1.Length()) != 0;
}

/******************************** TArray ********************************/

// 数组最大容量。初始化时决定，后面不允许改变
//int Array::Capacity() const { return _Capacity; }

/*int MemLen(const void* data)
{
	if(!data) return 0;

	// 自动计算长度，\0结尾，单字节大小时才允许
	int len = 0;
	const byte* p =(const byte*)data;
	while(*p++) len++;
	return len;
}*/

Array::Array(void* data, int len) : Buffer(data, len)
{
	Init();
}

Array::Array(const void* data, int len) : Buffer((void*)data, len)
{
	Init();

	_canWrite	= false;
}

/*Array::Array(const Buffer& rhs) : Buffer(rhs)
{
	Init();
}

Array::Array(const Array& rhs) : Buffer(rhs)
{
	Init();
}*/

Array::Array(Array&& rval)
{
	//*this	= rval;
	move(rval);
}

void Array::move(Array& rval)
{
	Buffer::move(rval);

	_Capacity	= rval._Capacity;
	_needFree	= rval._needFree;
	_canWrite	= rval._canWrite;

	rval._Capacity	= 0;
	rval._needFree	= 0;
	rval._canWrite	= 0;
}

void Array::Init()
{
	_Size	= 1;

	_Capacity	= _Length;
	_needFree	= false;
	_canWrite	= true;
}

// 析构。释放资源
Array::~Array()
{
	if(_needFree && _Arr) delete (byte*)_Arr;
}

Array& Array::operator = (const Buffer& rhs)
{
	Buffer::operator=(rhs);

	return *this;
}

Array& Array::operator = (const void* p)
{
	Buffer::operator=(p);

	return *this;
}

Array& Array::operator = (Array&& rval)
{
	//Buffer::operator=(rval);
	move(rval);

	return *this;
}

// 设置数组长度。容量足够则缩小Length，否则扩容以确保数组容量足够大避免多次分配内存
bool Array::SetLength(int length, bool bak)
{
	if(length <= _Capacity)
	{
		_Length = length;
	}
	else
	{
		if(!CheckCapacity(length, bak ? _Length : 0)) return false;
		// 扩大长度
		if(length > _Length) _Length = length;
	}
	return true;
}

// 设置数组元素为指定值，自动扩容
bool Array::SetItem(const void* data, int index, int count)
{
	assert_param2(_canWrite, "禁止SetItem修改");
	assert_param2(data, "Array::SetItem data Error");

	// count<=0 表示设置全部元素
	if(count <= 0) count = _Length - index;
	assert_param2(count > 0, "Array::SetItem count Error");

	// 检查长度是否足够
	int len2 = index + count;
	CheckCapacity(len2, index);

	byte* buf = (byte*)GetBuffer();
	// 如果元素类型大小为1，那么可以直接调用内存设置函数
	if(_Size == 1)
		memset(&buf[index], *(byte*)data, count);
	else
	{
		while(count-- > 0)
		{
			memcpy(buf, data, _Size);
			buf += _Size;
		}
	}

	// 扩大长度
	if(len2 > _Length) _Length = len2;

	return true;
}

// 设置数组。直接使用指针，不拷贝数据
bool Array::Set(void* data, int len)
{
	if(!Set((const void*)data, len)) return false;

	_canWrite	= true;

	return true;
}

// 设置数组。直接使用指针，不拷贝数据
bool Array::Set(const void* data, int len)
{
	//if(len < 0) len = MemLen(data);

	// 销毁旧的
	if(_needFree && _Arr && _Arr != data) delete (byte*)_Arr;

	_Arr		= (char*)data;
	_Length		= len;
	_Capacity	= len;
	_needFree	= false;
	_canWrite	= false;

	return true;
}

// 清空已存储数据。
void Array::Clear()
{
	assert_param2(_canWrite, "禁止Clear修改");
	assert_param2(_Arr, "Clear数据不能为空指针");

	memset(_Arr, 0, _Size * _Length);
}

// 设置指定位置的值，不足时自动扩容
void Array::SetItemAt(int i, const void* item)
{
	assert_param2(_canWrite, "禁止SetItemAt修改");

	// 检查长度，不足时扩容
	CheckCapacity(i + 1, _Length);

	if(i >= _Length) _Length = i + 1;

	memcpy((byte*)_Arr + _Size * i, item, _Size);
}

// 重载索引运算符[]，返回指定元素的第一个字节
byte Array::operator[](int i) const
{
	assert_param2(_Arr && i >= 0 && i < _Length, "下标越界");

	byte* buf = (byte*)_Arr;
	if(_Size > 1) i *= _Size;

	return buf[i];
}

byte& Array::operator[](int i)
{
	assert_param2(_Arr && i >= 0 && i < _Length, "下标越界");

	byte* buf = (byte*)_Arr;
	if(_Size > 1) i *= _Size;

	return buf[i];
}

// 检查容量。如果不足则扩大，并备份指定长度的数据
bool Array::CheckCapacity(int len, int bak)
{
	// 是否超出容量
	if(len <= _Capacity) return true;

	// 自动计算合适的容量
	int k = 0x40;
	while(k < len) k <<= 1;

	void* p = Alloc(k);
	if(!p) return false;

	// 是否需要备份数据
	if(bak > _Length) bak = _Length;
	if(bak > 0 && _Arr) memcpy(p, _Arr, bak);

	if(_needFree && _Arr && _Arr != p) delete (char*)_Arr;

	_Capacity	= k;
	_Arr		= (char*)p;
	_needFree	= true;

	return true;
}

void* Array::Alloc(int len) { return new byte[_Size * len];}

bool operator==(const Array& bs1, const Array& bs2)
{
	if(bs1.Length() != bs2.Length()) return false;

	return memcmp(bs1._Arr, bs2._Arr, bs1.Length() * bs1._Size) == 0;
}

bool operator!=(const Array& bs1, const Array& bs2)
{
	if(bs1.Length() != bs2.Length()) return true;

	return memcmp(bs1._Arr, bs2._Arr, bs1.Length() * bs1._Size) != 0;
}

/******************************** ByteArray ********************************/

ByteArray::ByteArray(const void* data, int length, bool copy) : Array(Arr, ArrayLength(Arr))
{
	if(copy)
		Copy(0, data, length);
	else
		Set(data, length);
}

ByteArray::ByteArray(void* data, int length, bool copy) : Array(Arr, ArrayLength(Arr))
{
	if(copy)
		Copy(0, data, length);
	else
		Set(data, length);
}

ByteArray::ByteArray(const Buffer& arr) : Array(Arr, arr.Length())
{
	Copy(0, arr, 0, -1);
}

ByteArray::ByteArray(const ByteArray& arr) : Array(Arr, arr.Length())
{
	Copy(0, arr, 0, -1);
}

ByteArray::ByteArray(ByteArray&& rval) : Array((const void*)nullptr, 0)
{
	move(rval);
}

// 字符串转为字节数组
ByteArray::ByteArray(String& str) : Array(Arr, ArrayLength(Arr))
{
	char* p = str.GetBuffer();
	Set((byte*)p, str.Length());
}

// 不允许修改，拷贝
ByteArray::ByteArray(const String& str) : Array(Arr, ArrayLength(Arr))
{
	const char* p = str.GetBuffer();
	//Copy((const byte*)p, str.Length());
	Set((const byte*)p, str.Length());
}

void ByteArray::move(ByteArray& rval)
{
	Array::move(rval);

	// 如果指向自己的缓冲区，那么拷贝一下数据
	if(rval._Arr == (char*)rval.Arr && rval._Length > 0)
	{
		Copy(0, rval._Arr, rval._Length);
	}
}

ByteArray& ByteArray::operator = (const Buffer& rhs)
{
	Array::operator=(rhs);

	return *this;
}

ByteArray& ByteArray::operator = (const ByteArray& rhs)
{
	Array::operator=(rhs);

	return *this;
}

ByteArray& ByteArray::operator = (const void* p)
{
	Array::operator=(p);

	return *this;
}

ByteArray& ByteArray::operator = (ByteArray&& rval)
{
	move(rval);

	return *this;
}

// 保存到普通字节数组，首字节为长度
int ByteArray::Load(const void* data, int maxsize)
{
	const byte* p = (const byte*)data;
	_Length = p[0] <= maxsize ? p[0] : maxsize;

	return Copy(0, p + 1, _Length);
}

// 从普通字节数据组加载，首字节为长度
int ByteArray::Save(void* data, int maxsize) const
{
	assert_param2(_Length <= 0xFF, "_Length <= 0xFF");

	byte* p = (byte*)data;
	int len = _Length <= maxsize ? _Length : maxsize;
	p[0] = len;

	return CopyTo(0, p + 1, len);
}

ushort	ByteArray::ToUInt16() const
{
	auto p = GetBuffer();
	// 字节对齐时才能之前转为目标整数
	if(((int)p & 0x01) == 0) return *(ushort*)p;

	return p[0] | (p[1] << 8);
}

uint	ByteArray::ToUInt32() const
{
	auto p = GetBuffer();
	// 字节对齐时才能之前转为目标整数
	if(((int)p & 0x03) == 0) return *(uint*)p;

	return p[0] | (p[1] << 8) | (p[2] << 0x10) | (p[3] << 0x18);
}

UInt64	ByteArray::ToUInt64() const
{
	auto p = GetBuffer();
	// 字节对齐时才能之前转为目标整数
	if(((int)p & 0x07) == 0) return *(UInt64*)p;

	uint n1 = p[0] | (p[1] << 8) | (p[2] << 0x10) | (p[3] << 0x18);
	p += 4;
	uint n2 = p[0] | (p[1] << 8) | (p[2] << 0x10) | (p[3] << 0x18);

	return n1 | ((UInt64)n2 << 0x20);
}

void ByteArray::Write(ushort value, int index)
{
	Copy(index, (byte*)&value, sizeof(ushort));
}

void ByteArray::Write(short value, int index)
{
	Copy(index, (byte*)&value, sizeof(short));
}

void ByteArray::Write(uint value, int index)
{
	Copy(index, (byte*)&value, sizeof(uint));
}

void ByteArray::Write(int value, int index)
{
	Copy(index, (byte*)&value, sizeof(int));
}

void ByteArray::Write(UInt64 value, int index)
{
	Copy(index, (byte*)&value, sizeof(UInt64));
}
