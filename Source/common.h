#pragma once
#include "ringbuffer.h"
#include <iostream>
#include <fstream>
#include "CPortSetting.h"
#include "CThreadConfig.h"
#include "threads.h"





extern vector<CPortSetting> portSettings;
extern vector<CThreadConfig*> threadConfigs;


#define TCP_PORT_LENGTH 6

#define ASYNC_COM_TCP_LOG
#define ASYNC_COM_TCP_ERR
//Print all packets(COM-potr, TCP)
#define ASYNC_COM_TCP_DBG

#ifdef ASYNC_COM_TCP_LOG
#define LOG(M) { cout << "INF: " << M; cout << "\n"; }
#else
#define LOG(M) 
#endif

#ifdef ASYNC_COM_TCP_ERR
#define ERR(M) { cout << "ERR: " << M; cout << "\n"; }
#else
#define ERR(M) 
#endif

#ifdef ASYNC_COM_TCP_DBG
#define DBG(M) { cout << "DBG: " << M; cout << "\n"; }
#else
#define DBG(M) 
#endif