﻿#ifndef _JTW8953_H_
#define _JTW8953_H_

#include "I2C.h"
#include "..\Storage\Storage.h"

// EEPROM
class JTW8953 : public CharStorage
{
public:
  
    JTW8953();
    virtual ~JTW8953();

	I2C* IIC;		// I2C通信口

	void Init();
	// 写入键位数据
	bool WriteKey(ushort index, byte data);

	byte Read(ushort addr);
	// 设置配置
	bool SetConfig(const Buffer& bs) const;

	virtual bool Write(uint addr, const Buffer& bs) const;

	virtual bool Read(uint addr, Buffer& bs) const;

	static void JTW8953Test();

private:
	byte Address;	// 设备地址
};

#endif