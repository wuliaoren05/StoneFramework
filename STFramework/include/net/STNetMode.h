#ifndef ST_NETMODE_H
#define ST_NETMODE_H

class STNetMode {
public:
	virtual bool Init() = 0;
	virtual bool Run() = 0;
	virtual bool Destroy() = 0;
};

#endif
