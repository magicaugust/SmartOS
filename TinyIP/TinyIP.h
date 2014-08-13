#ifndef _TinyIP_H_
#define _TinyIP_H_

#include "Enc28j60.h"
#include "Net/Ethernet.h"

// 精简IP类
class TinyIP //: protected IEthernetAdapter
{
private:
    Enc28j60* _enc;
	NetPacker* _net;

	byte* Buffer; // 缓冲区

	static void Work(void* param);	// 任务函数
	void OnWork();	// 循环调度的任务

	void ProcessArp(byte* buf, uint len);
	void ProcessICMP(byte* buf, uint len);
	void ProcessTcp(byte* buf, uint len);
	void ProcessUdp(byte* buf, uint len);
	void SendEthernet(byte* buf, uint len);
	void SendIP(byte* buf, uint len);
	void SendTcp(byte* buf, uint len, byte flags);
	void SendUdp(byte* buf, uint len, bool checksum = true);
	void SendDhcp(byte* buf, uint len);

	byte seqnum;

	void TcpHead(uint ackNum, bool mss, bool cp_seq);
	void TcpAck(byte* buf, uint dlen);

	uint CheckSum(byte* buf, uint len, byte type);

	uint dhcp_id;
	void DHCPDiscover();
	void DHCPRequest(byte* buf);
	void PareOption(byte* buf, int len);
	void DHCPConfig(byte* buf);

	/*virtual void Send(IP_TYPE type, uint len);
	// 获取负载数据指针。外部可以直接填充数据
	virtual byte* GetPayload();
	virtual void OnReceive(byte* buf, uint len);*/
public:
    byte IP[4];
    byte Mask[4];
	byte Mac[6];
	ushort Port;

	byte RemoteMac[6];
	byte RemoteIP[4];
	ushort RemotePort;

	ushort BufferSize;	// 缓冲区大小
	bool UseDHCP;
	bool IPIsReady;
	byte DHCPServer[4];
	byte DNSServer[4];
	byte Gateway[4];

    TinyIP(Enc28j60* enc, byte ip[4], byte mac[6]);
    virtual ~TinyIP();

	bool Init();
	static void ShowIP(byte* ip);
	static void ShowMac(byte* mac);

	bool TcpConnect(byte ip[4], ushort port);	// 连接远程
    void TcpSend(byte* packet, uint len);
    void TcpClose(byte* packet, uint maxlen);

	// 收到Ping请求时触发，传递结构体和负载数据长度，负载数据紧跟着结构体
	typedef void (*PingHandler)(TinyIP* tip, ICMP_HEADER* icmp, uint len);
	PingHandler OnPing;

	// 收到Udp数据时触发，传递结构体和负载数据长度，负载数据紧跟着结构体
	typedef void (*UdpHandler)(TinyIP* tip, UDP_HEADER* udp, uint len);
	UdpHandler OnUdpReceived;

	// 收到Tcp数据时触发，传递结构体和负载数据长度，负载数据紧跟着结构体
	typedef void (*TcpHandler)(TinyIP* tip, TCP_HEADER* tcp, uint len);
	TcpHandler OnTcpAccepted;
	TcpHandler OnTcpReceived;
	TcpHandler OnTcpDisconnected;

	void Register(DataHandler handler, void* param = NULL);
private:
	DataHandler _Handler;
	void* _Param;
};

#endif
