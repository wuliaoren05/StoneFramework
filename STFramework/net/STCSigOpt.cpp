/*
 * STCSigOpt.cpp
 *
 *  Created on: May 24, 2015
 *      Author: root
 */


#include "net/STCSigOpt.h"


int CSigOpt::ignalSig() {
	m_sigact.sa_handler = handle_pipe;
	sigemptyset(&m_sigact.sa_mask);
	m_sigact.sa_flags = 0;
	sigaction(SIGPIPE, &m_sigact, NULL);
	return RETOK;
}

void CSigOpt::handle_pipe(int sig) {
	//nothing to do
}


