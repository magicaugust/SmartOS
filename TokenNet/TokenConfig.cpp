﻿#include "TokenConfig.h"
#include "Net\Net.h"
#include "Config.h"

TokenConfig* TokenConfig::Current	= NULL;

void TokenConfig::LoadDefault()
{
	// 实际内存大小，减去头部大小
	uint len = sizeof(this[0]) - ((int)&Length - (int)this);
	memset(&Length, 0, len);
	Length		= len;

	SoftVer		= Sys.Version;
	PingTime	= 10;
}

void TokenConfig::Load()
{
	// Flash最后一块作为配置区
	if(!Config::Current) Config::Current = &Config::CreateFlash();

	// 尝试加载配置区设置
	uint len = Length;
	if(!len) len = sizeof(this[0]);
	ByteArray bs(&Length, len);
	if(!Config::Current->GetOrSet("TKCF", bs))
		debug_printf("TokenConfig::Load 首次运行，创建配置区！\r\n");
	else
		debug_printf("TokenConfig::Load 从配置区加载配置\r\n");
}

void TokenConfig::Save()
{
	uint len = Length;
	if(!len) len = sizeof(this[0]);

	debug_printf("TokenConfig::Save \r\n");

	ByteArray bs(&Length, len);
	Config::Current->Set("TKCF", bs);
}

void TokenConfig::Show()
{
#if DEBUG
	debug_printf("TokenConfig::令牌配置：\r\n");

	debug_printf("\t端口: %d \r\n", Port);

	debug_printf("\t远程: ");
	IPEndPoint ep2(IPAddress(ServerIP), ServerPort);
	ep2.Show(true);

	debug_printf("\t服务: %s \r\n", Server);
	debug_printf("\t厂商: %s \r\n", Vendor);
#endif
}

void TokenConfig::Write(Stream& ms) const
{
	uint len = Length;
	if(!len) len = sizeof(this[0]);

	ms.Write(&Length, 0, len);
}

void TokenConfig::Read(Stream& ms)
{
	uint len = Length;
	if(!len) len = sizeof(this[0]);

	ms.Read(&Length, 0, len);
}

TokenConfig* TokenConfig::Init()
{
	static TokenConfig tc;
	TokenConfig::Current = &tc;
	tc.LoadDefault();

	return &tc;
}