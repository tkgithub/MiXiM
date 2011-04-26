#include "AppToNetControlInfo.h"
#include "NodeNetLayer.h"
#include "NetwControlInfo.h"
#include "AddressingInterface.h"
#include "NetwPkt_m.h"
#include <NetwToMacControlInfo.h>
#include <MacToNetwControlInfo.h>
#include <cassert>

Define_Module(NodeNetLayer);

void NodeNetLayer::initialize(int stage) {
	BaseNetwLayer::initialize(stage);
	if (stage == 0) {
	}
}

void NodeNetLayer::finish() {
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void NodeNetLayer::handleLowerMsg(cMessage* msg)
{
    NetwPkt *m = static_cast<NetwPkt *>(msg);
    EV << " handling packet from " << m->getSrcAddr() << endl;
    sendUp(decapsMsg(m));
}

/**
 * Redefine this function if you want to process control messages
 * from lower layers.
 **/
void NodeNetLayer::handleLowerControl(cMessage* msg)
{
	sendControlUp(msg);
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void NodeNetLayer::handleUpperMsg(cMessage* msg)
{
	assert(dynamic_cast<cPacket*>(msg));
    sendDown(encapsMsg(static_cast<cPacket*>(msg)));
}

/**
 * Redefine this function if you want to process control messages
 * from upper layers.
 **/
void NodeNetLayer::handleUpperControl(cMessage* msg)
{
	sendControlDown(msg);
}


/**
 * Decapsulates the packet from the received Network packet
 **/
cMessage* NodeNetLayer::decapsMsg(NetwPkt *msg)
{
    cMessage *m = msg->decapsulate();
	//get control info attached by base class decapsMsg method
	//and set its rssi and ber
	assert(dynamic_cast<MacToNetwControlInfo*>(msg->getControlInfo()));
	MacToNetwControlInfo* cInfo = static_cast<MacToNetwControlInfo*>(msg->getControlInfo());

	m->setControlInfo(new NetwControlInfo(msg->getSrcAddr(), cInfo->getBitErrorRate(), cInfo->getRSSI()));
    // delete the netw packet
    delete msg;
    return m;
}


/**
 * Encapsulates the received TransPkt into a NetwPkt and set all needed
 * header fields.
 **/
NetwPkt* NodeNetLayer::encapsMsg(cPacket *appPkt) {
    int macAddr;
    int netwAddr;

    EV <<"in encaps...\n";

    NetwPkt *pkt = new NetwPkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    netwAddr = (static_cast<ApplPkt*>(appPkt))->getDestAddr();
    EV<<"NET received a message from upper layer, name is " << appPkt->getName() <<", dest mac addr="<< netwAddr <<endl;

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    EV << " netw "<< myNetwAddr << " sending packet" <<endl;
    if(netwAddr == L3BROADCAST) {
        EV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
           << " -> set destMac=L2BROADCAST\n";
        macAddr = L2BROADCAST;
    }
    else{
        EV <<"sendDown: get the MAC address\n";
        macAddr = arp->getMacAddr(netwAddr);
    }

    switch( appPkt->getKind() )
    {
    case NodeAppLayer::REPORT_WITH_CSMA:
    case NodeAppLayer::SYNC_MESSAGE_WITH_CSMA:
    	pkt->setControlInfo(new NetwToMacControlInfo(macAddr, true));
    	break;
    default:
    	pkt->setControlInfo(new NetwToMacControlInfo(macAddr, false));
    }

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    EV <<" pkt encapsulated\n";
    return pkt;
}
