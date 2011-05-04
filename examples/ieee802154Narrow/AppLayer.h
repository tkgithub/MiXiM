#ifndef APPLAYER_H_
#define APPLAYER_H_

#include "BaseLayer.h"
#include "ApplPkt_m.h"
#include "SimpleAddress.h"
#include "BaseArp.h"
#include <BaseWorldUtility.h>
#include "BaseConnectionManager.h"
#include "NetwControlInfo.h"
#include <cassert>
#include <Packet.h>
#include <BaseMacLayer.h>
#include <omnetpp.h>

class AppLayer : public BaseLayer
{
public:

	enum TrafficGenMessageKinds{		// All the possible types of messages/tasks to send/schedule

		SEND_SYNC_TIMER_WITH_CSMA = 1,	// Used to schedule the next message
		SYNC_MESSAGE_WITH_CSMA,			// Used by the Mobile Nodes type 3 and 4 to send their broadcasts
		SEND_SYNC_TIMER_WITHOUT_CSMA,	// Used to schedule the next message
		SYNC_MESSAGE_WITHOUT_CSMA,		// Used by the Anchors to send the sync messages
		SEND_REPORT_WITH_CSMA,			// Used to schedule the next message
		REPORT_WITH_CSMA,				// Report from any host with CSMA enabled
		SEND_REPORT_WITHOUT_CSMA,		// Used to schedule the next message
		REPORT_WITHOUT_CSMA,			// Report from any host with CSMA disabled
		CONFIGURE_FULL_PHASE,			// Event at start from every full phase (period) to organize all the necesary events during it
		CONFIGURE_EXTRA_REPORT,			// Event to schedule the extra reports for all the hosts when they are needed
		CALCULATE_POSITION,				// Event to simulate the processing time of the Mobile Node type 2 when it calculates its position
		CHECK_QUEUE,					// Event to check the transmission queues from Anchors (Com Sink 1) and Computer (Com Sink 2)
		WAITING_REQUEST					// Event to simulate the waiting time since we send the request till we receive the answer from the Anchor
	};

	enum PhaseType{						// Phases of the Full Phase or Period

		SYNC_PHASE_1 = 1,
		REPORT_PHASE ,
		VIP_PHASE,
		SYNC_PHASE_2,
		COM_SINK_PHASE_1,
		SYNC_PHASE_3,
		COM_SINK_PHASE_2
	};


protected:

	BaseArp* arp;						// Pointer to the ARP of the Host
	int myNetwAddr;						// Network address of the Host

	BaseConnectionManager* cc;			// Pointer to the Propagation Model module

	int packetLength;					// Standard Packet Length, in some cases when we transmit info we'll have to change it
	simtime_t packetTime;				// We'll have to change the packet time according to the Packet Length

	long destination;					// Broadcast destination, usually -1

	int syncPacketsPerSyncPhase; 		// Determines how many times do we repeat all the slots per sync phase

	simtime_t syncPacketTime;			// Max. duration of a Sync Packet, determines the slot size
	simtime_t fullPhaseTime;			// Duration of the Full Phase or Period
	simtime_t timeComSinkPhase;			// Duration of every Com Sink Phase
	simtime_t timeSyncPhase;			// Duration of every Sync Phase, everyone is formed by syncPacketsPerSyncPhase mini sync phases
	simtime_t timeReportPhase;			// Duration of the Report Phase
	simtime_t timeVIPPhase;				// Duration of the VIP Phase
	double phase2VIPPercentage;			// Percentage of the time Phase Report + Phase VIP that the Phase VIP takes

	simtime_t guardTimeReportPhase;		// Guard time to leave at the end of the Report Phase, so the transmissions don't invade next phase
	simtime_t guardTimeVIPPhase;		// Guard time to leave at the end of the VIP Phase, so the transmissions don't invade next phase
	simtime_t guardTimeComSinkPhase;	// Guard time to leave at the end of every Com Sink Phase, so the transmissions don't invade next phase

	int numTotalSlots;					// Number of Slots in every mini sync Phase, they are calculated by the computer in the initialize method
	int numberOfAnchors;				// Number of Anchors in the Network
	int numberOfNodes;					// Number of Mobile Nodes in the Network

	long nbPacketDroppedReportMN;		// Variable to count the number of packets that were dropped when sending a Report from a Mobile Node and don't getting ACK
	long nbPacketDroppedReportAN;		// Variable to count the number of packets that were dropped when sending a Report from an Anchor or Computer and don't getting ACK
	long nbPacketDroppedBackOffMN;		// Variable to count the number of packets that were dropped when sending a Report from a Mobile Node and reaching maximum BackOff number
	long nbPacketDroppedBackOffAN;		// Variable to count the number of packets that were dropped when sending a Report from an Anchor or Computer and reaching maximum BackOff number
	long nbPacketDroppedNoTimeAN;		// Variable to count the number of packets that were dropped at application layer when there was no more time for an AN to transmit more
	long nbPacketDroppedNoTimeMN;		// Variable to count the number of packets that were dropped at application layer when there was no more time for a MN to transmit more
	long numOfPcktsRcvdInWrongPhase;	// Variable to count the number of packets that arrived to any host in a phase they are not supposed to arrive
	long nbRqstWihoutAnswer;			// Variable to count the number of requests from the Mobile Node that didn't get any answer from an Anchor

	NicEntry* computer;					// Pointer to the NIC of the computer to take general data over the configurations
	NicEntry* host;						// Pointer to a host, we use it to point the module who sent the message to check what type of module is (AN, MN, Comp)

	int maxRetransDroppedReportMN;		// Maximum number of retransmissions from the App Layer when a Mobile Node gets a drop from MAC when it sends a Report
	int maxRetransDroppedReportAN;		// Maximum number of retransmissions from the App Layer when an Anchor gets a drop from MAC when it sends a Report
	int maxRetransDroppedBackOff;		// Maximum number of retransmissions from the App Layer when we get a CAF (Drop because of too many backoffs) from the MAC

public:

	virtual void initialize(int stage);
	virtual int numInitStages() const {return 5;}

protected:

	/* Function to return in which sub-phase from the full phase we are:
			1 -> 1st SyncPhase
			2 -> Report Phase
			3 -> VIP Phase
			4 -> 2nd SyncPhase
			5 -> 1st ComSink
			6 -> 3rd SyncPhase
			7 -> 2nd ComSink
	   In a future this value must be taken when it synchronizes with the AN
	   The values go in seconds */
	PhaseType InWhichPhaseAmI(simtime_t fullPhaseTime, simtime_t timeSyncPhase, simtime_t timeReportPhase, simtime_t timeVIPPhase, simtime_t timeComSinkPhase);
};

#endif