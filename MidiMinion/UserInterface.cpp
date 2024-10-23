#include "UserInterface.h"
#include "DBG.h"

//---------------  UI Message Queue  -----------------

bool UIMessageQueue::add(Defs::UIMessageType msgType, const void* dataPtr) {
	bool rtn = false;
	xDBG::println("UI Q - Add - Type=", (uint8_t)msgType);
	if (Qsize < storageSize) {

		queueStorage[nextOpenSpot].msgType = msgType;
		queueStorage[nextOpenSpot].dataPtr = dataPtr;

		//move to next spot ... loop around to [0] if we are at end of storage
		nextOpenSpot = (nextOpenSpot + 1 >= storageSize) ? 0 : nextOpenSpot + 1;

		//up que size ... if its full nextOpen will be next in Q's "chair"
		Qsize++;
		if (Qsize >= storageSize) {
			nextOpenSpot = nextInQ;
			Qsize = storageSize; // just in case Qsize got bigger ?
		}
		rtn = true;
	}
	return rtn;
}
Defs::UIMessage UIMessageQueue::remove() {

	Defs::UIMessage rtn = {.msgType = Defs::UIMessageType::EmptyQueue, .dataPtr = nullptr };
	if (Qsize > 0) {
		rtn = queueStorage[nextInQ];
		Qsize--;
		//bump up nextInQ ... loop around to 0 if at end of storage
		nextInQ = (nextInQ + 1 == storageSize) ? 0 : nextInQ + 1;
	}
	return rtn;
}

Defs::UIMessageType UIMessageQueue::peekMsgType() { return (!(Qsize)) ? Defs::UIMessageType::EmptyQueue : queueStorage[nextInQ].msgType; }



UIPrintHandler UIMessageRouter::handler;

void UIPrintHandler::deviceConnect(const Defs::DeviceInfoSummary_t* pDeviceInfoSumStruct) {
	if (pDeviceInfoSumStruct == nullptr) {
		xDBG::println("Device Connected");
		return;
	}
		uint8_t idx = pDeviceInfoSumStruct->productStringIndex;
	if (idx == 0) {
		xDBG::println("Device Connected - USBVendorID = ",pDeviceInfoSumStruct->USBvendorID);
	}
	else {
		xDBG::println("Device Connected - ", (const char*)pDeviceInfoSumStruct->iStrings[idx]);
	}
}
void UIPrintHandler::deviceDisconnect(const Defs::DeviceInfoSummary_t* pDeviceInfoSumStruct) {
	if (pDeviceInfoSumStruct == nullptr) {
		xDBG::println("Device Disconnected");
		return;
	}
	uint8_t idx = pDeviceInfoSumStruct->productStringIndex;
	if (idx == 0) {
		xDBG::println("Device Disconnected - USBVendorID = ", pDeviceInfoSumStruct->USBvendorID);
	}
	else {
		xDBG::println("Device Disconnected - ", (const char*)pDeviceInfoSumStruct->iStrings[idx]);
	}
}


