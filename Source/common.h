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