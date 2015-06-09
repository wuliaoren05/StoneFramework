/*
 * STCSigOpt.h
 *
 *  Created on: May 24, 2015
 *      Author: root
 */

#ifndef STFRAMEWORK_INCLUDE_NET_STCSIGOPT_H_
#define STFRAMEWORK_INCLUDE_NET_STCSIGOPT_H_

#include <signal.h>

#include "base/STCommonDefine.h"

class CSigOpt {
public:
	int ignalSig();
private:
	static void handle_pipe(int sig);
private:
	struct sigaction m_sigact;
};


#endif /* STFRAMEWORK_INCLUDE_NET_STCSIGOPT_H_ */
