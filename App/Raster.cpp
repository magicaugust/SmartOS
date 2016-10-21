﻿#include "Device\Port.h"
#include "App\PulsePort.h"

#include "Raster.h"


static Stream*	_Cache	= nullptr;		//实际送的数据
static uint		_task	= 0;
static int		_Ras	= 0;

static PulsePort* Create(Pin pin)
{
	auto pp	= new PulsePort();
	pp->Port = new InputPort(pin);
	pp->Port->HardEvent	= true;
	pp->Min	= 100;
	pp->Max	= 2000;

	return pp;
}

Raster::Raster(Pin pinA, Pin pinB, Pin bz)
{
	Init(bz);

	RasterA	= Create(pinA);
	RasterB	= Create(pinB);

	_Ras++;
}

Raster::~Raster()
{
	delete RasterA->Port;
	delete RasterA;
	delete RasterB->Port;
	delete RasterB;

	if(--_Ras == 0)
	{
		delete _Cache;
		Sys.RemoveTask(_task);
	}
}

void Raster::Init(Pin bz)
{
	RasterA = nullptr;
	RasterB = nullptr;
	Opened = false;

	Buzzer.Set(bz);
	Buzzer.Invert = 1;
	Buzzer.Blink(2, 100);
	Buzzer.Open();
}

bool Raster::Open()
{
	if (Opened) return true;

	if (RasterA != nullptr)
	{
		RasterA->Press.Bind(&Raster::OnHandlerA, this);
		RasterA->Open();
	}

	if (RasterB != nullptr)
	{
		RasterB->Press.Bind(&Raster::OnHandlerB, this);
		RasterB->Open();
	}

	if(!_Cache)
	{
		_Cache	= new MemoryStream();
		_Cache->SetCapacity(512);
	}
	// 光栅一直在轮训是否有数据要发送
	if(!_task) _task	= Sys.AddTask(&Raster::Report, this, 3000, 3000, "光栅发送");

	Opened = true;

	return true;
}

// 检查配对
static bool CheckMatch(FlagData& flag)
{
	if (flag.Count == 0) return false;

	// 超过3秒无效
	if(flag.Start + 3000 < Sys.Ms())
	{
		flag.Count	= 0;
		return false;
	}

	return true;
}

void Raster::OnHandlerA(PulsePort& raster)
{
	Buzzer.Blink(2, 500);
	debug_printf("A路触发，波长%d \r\n", raster.Time);
	FlagA.Start = raster.Start;
	FlagA.Time = raster.Time;
	FlagA.Count++;

	if(CheckMatch(FlagB)) LineReport();
}

void Raster::OnHandlerB(PulsePort& raster)
{
	Buzzer.Blink(2, 500);
	debug_printf("B路触发，波长%d \r\n", raster.Time);
	FlagB.Start = raster.Start;
	FlagB.Time = raster.Time;
	FlagB.Count++;

	if(CheckMatch(FlagA)) LineReport();
}

void Raster::LineReport()
{
	auto size = sizeof(RasTriData);

	// 构造当前数据
	RasTriData data;
	data.Index = Index;
	data.Start = DateTime::Now().TotalMs();
	data.TimeA = FlagA.Time;
	data.TimeB = FlagB.Time;
	data.Count = Count++;

	// 时间差加方向
	if (FlagA.Start > FlagB.Start)
	{
		data.Direction = 0x00;
		data.Time = (ushort)(FlagA.Start - FlagB.Start);
	}
	else
	{
		data.Direction = 0x01;
		data.Time = (ushort)(FlagB.Start - FlagA.Start);
	}

	Buffer bs(&data, size);
	// 如果满了，马上发送
	if(bs.Length() > _Cache->Remain()) Report();
	_Cache->Write(bs);

	//写完数据后标致清零
	FlagA.Count = FlagB.Count = 0;
}

void Raster::Report()
{
	// 没数据或者客户端未登录
	if (_Cache->Length == 0) return;

	OnReport(*_Cache);

	_Cache->SetPosition(0);
	_Cache->Length	= 0;
}
