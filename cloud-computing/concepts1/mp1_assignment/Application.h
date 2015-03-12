/**********************************
 * FILE NAME: Application.h
 *
 * DESCRIPTION: Header file of all classes pertaining to the Application Layer
 **********************************/

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "stdincludes.h"
#include "MP1Node.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

/**
 * global variables
 */
int nodeCount = 0;

/*
 * Macros
 */
#define ARGS_COUNT 2
#define TOTAL_RUNNING_TIME 700

/**
 * CLASS NAME: Application
 *
 * DESCRIPTION: Application layer of the distributed system
 */
class Application{
private:
	// Address for introduction to the group
	// Coordinator Node
	char JOINADDR[30];
	EmulNet *en;
	Log *log;
	MP1Node **mp1;
	Params *par;
public:
	Application(char *);
	virtual ~Application();
	Address getjoinaddr();
	int run();
	void mp1Run();
	void fail();
};

#endif /* _APPLICATION_H__ */
