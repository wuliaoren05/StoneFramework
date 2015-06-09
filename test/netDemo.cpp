#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <string>

#include "net/STNetMode.h"
#include "net/STNetEpoll.h"

void Heartbeat() {
	printf("send Heartbeat flag\n");
	return;
}
static void* funDo(int32_t fd, void* pData){

	return NULL;
};

int main(int argc, void* argv[]) {
	int32_t iRet = 0;
	std::string strCfgPath = "NetEpoll.ini";
	printf("cfg file PATH:%s\n", strCfgPath.c_str());
	CNetEpoll netEpoll;
	iRet = netEpoll.init(strCfgPath);
	if (iRet < 0) {
		return -1;
	}
	netEpoll.set_cb(funDo,funDo,funDo);
	iRet = netEpoll.run();
	if (iRet < 0) {
		printf("iRet = netEpoll.run(); error \n");
		return -1;
	}
	while (1) {
		sleep(1);
		Heartbeat();
	}
	netEpoll.destroy();
	printf("exit the progress\n");
	sleep(30);
	return 0;
}
