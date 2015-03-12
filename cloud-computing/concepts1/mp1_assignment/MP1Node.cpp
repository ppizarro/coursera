/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = member;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
    // node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long);
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "Message JOINREQ sent to %s SIZE[%li]", joinaddr->getAddress().c_str(), msgsize);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode(){
    return 0;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

void MP1Node::updateMember(int id, short port, long heartbeat)
{
    vector<MemberListEntry>::iterator it = memberNode->memberList.begin();
    for (; it != memberNode->memberList.end(); ++it) {
        if (it->id == id && it->port ==port) {
            if (heartbeat > it->heartbeat) {
#ifdef DEBUGLOG
                log->LOG(&memberNode->addr, "\t\tUpdateMember new heartbeat[%li] timestamp[%li] old heartbeat[%li] timestamp[%li]",
                         heartbeat, par->getcurrtime(), it->heartbeat, it->timestamp);
#endif
                it->setheartbeat(heartbeat);
                it->settimestamp(par->getcurrtime());
            }
            return;
        }
    }

    MemberListEntry memberEntry(id, port, heartbeat, par->getcurrtime());
    memberNode->memberList.push_back(memberEntry);

#ifdef DEBUGLOG
    Address joinaddr;
    memcpy(&joinaddr.addr[0], &id, sizeof(int));
    memcpy(&joinaddr.addr[4], &port, sizeof(short));
    log->logNodeAdd(&memberNode->addr, &joinaddr);
#endif
}

void MP1Node::updateMember(MemberListEntry& member)
{
    updateMember(member.getid(), member.getport(), member.getheartbeat());
}

void MP1Node::sendMemberList(enum MsgTypes msgType, Address * to)
{
    long members = memberNode->memberList.size();
    size_t msgsize = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + members * sizeof(MemberListEntry);
    MessageHdr * msg = (MessageHdr *) malloc(msgsize * sizeof(char));
    char * data = (char*)(msg + 1);

    msg->msgType = msgType;
    memcpy(data, &memberNode->addr.addr, sizeof(memberNode->addr.addr));
    data += sizeof(memberNode->addr.addr);
    memcpy(data, &members, sizeof(long));
    data += sizeof(long);

    for (vector<MemberListEntry>::iterator it = memberNode->memberList.begin() ; it != memberNode->memberList.end();) {
        
        if (it != memberNode->memberList.begin()) {
            if (par->getcurrtime() - it->timestamp > TFAIL) {
#ifdef DEBUGLOG
                log->LOG(&memberNode->addr, "Member FAILED! id[%i] currenttime[%li] timestamp[%li]", it->id, par->getcurrtime(), it->timestamp);
#endif
                ++it;
                continue;
            }
/*
            if (par->getcurrtime() - it->timestamp > TREMOVE) {
                // Member CLEANUP!
#ifdef DEBUGLOG
                Address joinaddr;
                memcpy(&joinaddr.addr[0], &it->id, sizeof(int));
                memcpy(&joinaddr.addr[4], &it->port, sizeof(short));
                log->logNodeRemove(&memberNode->addr, &joinaddr);
#endif         
                it = memberNode->memberList.erase(it);              
                continue;
            }
*/            
        }
        
        logMemberListEntry(*it);

        //data += memcpyMemberListEntry(data, *it);
        memcpy(data, &(*it), sizeof(MemberListEntry));
        data += sizeof(MemberListEntry);
        ++it;
    }

    emulNet->ENsend(&memberNode->addr, to, (char *)msg, msgsize);
    free(msg);
}

bool MP1Node::recvJoinReq(void *env, char *data, int size) {
    if (size < (int)(sizeof(memberNode->addr.addr) + sizeof(long))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message JOINREQ received with size wrong. Ignored.");
#endif
        return false;
    }

    Address joinaddr;
    long heartbeat;

    memcpy(joinaddr.addr, data, sizeof(memberNode->addr.addr));
    memcpy(&heartbeat, data + sizeof(memberNode->addr.addr), sizeof(long));

#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "JOINREQ received from: %s heartbeat: %li", joinaddr.getAddress().c_str(), heartbeat);
#endif

    int id = *(int*)(&joinaddr.addr);
    int port = *(short*)(&joinaddr.addr[4]);

    updateMember(id, port, heartbeat);

    #ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message JOINREP sent to: %s", joinaddr.getAddress().c_str());
    #endif

    sendMemberList(JOINREP, &joinaddr);
    return true;
}

bool MP1Node::recvMemberList(const char * label, void *env, char *data, int size)
{
    if (size < (int)(sizeof(long))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message %s received with size wrong. Ignored.", label);
#endif
        return false;
    }

    long members;
    memcpy(&members, data, sizeof(long));
    data += sizeof(long);
    size -= sizeof(long);

    //log->LOG(&memberNode->addr, "Message %s received: SIZE[%li]", label, members);

    if (size < (int)(members * sizeof(MemberListEntry))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message %s received with size wrong. Ignored.", label);
#endif
        return false;
    }

    MemberListEntry member;
    for (long i = 0; i < members; i++) {
        //log->LOG(&memberNode->addr, "Message %s received: i[%li]", label, i);
        memcpy(&member, data, sizeof(MemberListEntry));
        data += sizeof(MemberListEntry);
        logMemberListEntry(member);
        updateMember(member);
    }
    return true;
}

bool MP1Node::recvJoinRep(void *env, char *data, int size) {
    if (size < (int)(sizeof(memberNode->addr.addr))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message JOINREP received with size wrong. Ignored.");
#endif
        return false;
    }

    Address senderAddr;
    memcpy(senderAddr.addr, data, sizeof(memberNode->addr.addr));
    data += sizeof(memberNode->addr.addr);
    size -= sizeof(memberNode->addr.addr);

#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "Message JOINREP received from %s", senderAddr.getAddress().c_str());
#endif

    if (!recvMemberList("JOINREP", env, data, size)) {
        return false;
    }

    memberNode->inGroup = true;
    return true;
}

bool MP1Node::recvHeartbeatReq(void *env, char *data, int size) {
    if (size < (int)(sizeof(memberNode->addr.addr))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message HEARTBEATREQ received with size wrong. Ignored.");
#endif
        return false;
    }

    Address senderAddr;
    memcpy(senderAddr.addr, data, sizeof(memberNode->addr.addr));
    data += sizeof(memberNode->addr.addr);
    size -= sizeof(memberNode->addr.addr);

#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "Message HEARTBEATREQ received from %s", senderAddr.getAddress().c_str());
#endif

    if (!recvMemberList("HEARTBEATREQ", env, data, size)) {
        return false;
    }

    size_t msgsize = sizeof(MessageHdr) + sizeof(memberNode->addr.addr);
    MessageHdr * msg = (MessageHdr *) malloc(msgsize * sizeof(char));
    msg->msgType = HEARTBEATREP;
    memcpy((char *)(msg + 1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));

    #ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message HEARTBEATREP sent to: %s", senderAddr.getAddress().c_str());
    #endif

    emulNet->ENsend(&memberNode->addr, &senderAddr, (char *)msg, msgsize);
    free(msg);
    return true;
}

bool MP1Node::recvHeartbeatRep(void *env, char *data, int size) {
    if (size < (int)(sizeof(memberNode->addr.addr))) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message HEARTBEATREP received with size wrong. Ignored.");
#endif
        return false;
    }

    Address senderAddr;
    memcpy(senderAddr.addr, data, sizeof(memberNode->addr.addr));

#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "Message HEARTBEATREP received from %s", senderAddr.getAddress().c_str());
#endif
/*
    int id = *(int*)(&senderAddr.addr);
    int port = *(short*)(&senderAddr.addr[4]);
    vector<MemberListEntry>::iterator it = memberNode->memberList.begin();
    //++it;
    for (; it != memberNode->memberList.end(); ++it) {
        //logMemberListEntry(*it);
        if (it->id == id && it->port == port) {
            //FIXME it->heartbeat++;
            //FIXME it->timestamp = par->getcurrtime();
            log->LOG(&memberNode->addr, "\t\tHEARTBEATREP heartbeat: %li timestamp: %li", it->heartbeat, it->timestamp);
            return true;
        }
    }
*/
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message HEARTBEATREP not found in member list.");
#endif
    return false;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {

    if (size < (int)sizeof(MessageHdr)) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message received with size less than MessageHdr. Ignored.");
#endif
        return false;
    }

    MessageHdr * msg = (MessageHdr *)data;

    switch (msg->msgType) {
        case JOINREQ:
            return recvJoinReq(env, data + sizeof(MessageHdr), size - sizeof(MessageHdr));
        case JOINREP:
            return recvJoinRep(env, data + sizeof(MessageHdr), size - sizeof(MessageHdr));
        case HEARTBEATREQ:
            return recvHeartbeatReq(env, data + sizeof(MessageHdr), size - sizeof(MessageHdr));
        case HEARTBEATREP:
            return recvHeartbeatRep(env, data + sizeof(MessageHdr), size - sizeof(MessageHdr));
        case DUMMYLASTMSGTYPE:
            return false;
    }

    return false;
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {
   
    if (par->getcurrtime() > 3 && memberNode->memberList.size() > 1) {

        memberNode->memberList.begin()->heartbeat++;
        memberNode->memberList.begin()->timestamp = par->getcurrtime();

        int pos = rand() % (memberNode->memberList.size() - 1) + 1;   
        MemberListEntry& member = memberNode->memberList[pos];

        if (par->getcurrtime() - member.timestamp > TFAIL) {
            // FIXME escolher outro
            return;
        }

        Address memberAddr;
        memcpy(&memberAddr.addr[0], &member.id, sizeof(int));
        memcpy(&memberAddr.addr[4], &member.port, sizeof(short));

    #ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Message HEARTBEATREQ sent to: %s", memberAddr.getAddress().c_str());
    #endif

        sendMemberList(HEARTBEATREQ, &memberAddr);
    }
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(memberNode->addr.addr));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();

    int id = *(int*)(&memberNode->addr.addr);
    int port = *(short*)(&memberNode->addr.addr[4]);
    MemberListEntry memberEntry(id, port, 0, par->getcurrtime());
    memberNode->memberList.push_back(memberEntry);
    memberNode->myPos = memberNode->memberList.begin();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}

void MP1Node::logMemberListEntry(MemberListEntry& member)
{
#ifdef DEBUGLOG
    log->LOG(&memberNode->addr, "\t\tMember %i:%i: heartbeat[%li] timestamp[%li]", member.id, member.port, member.heartbeat, member.timestamp);
#endif
}
