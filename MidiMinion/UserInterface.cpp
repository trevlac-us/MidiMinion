#include "UserInterface.h"
#include "DBG.h"

//---------------  UI Message Queue  -----------------

bool UIMessageQueue::add(Defs::UIMessageType msgType, void* dataPtr) {
	bool rtn = false;

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
bool UIMessageQueue::remove() {
	bool rtn = false;
	if (Qsize > 0) {
		Qsize--;
		//bump up nextInQ ... loop around to 0 if at end of storage
		nextInQ = (nextInQ + 1 == storageSize) ? 0 : nextInQ + 1;
		rtn = true;
	}
	return rtn;
}

Defs::UIMessageType UIMessageQueue::getNextMsgType() { return (!(Qsize)) ? Defs::UIMessageType::EmptyQueue : queueStorage[nextInQ].msgType; }
void* UIMessageQueue::getNextMsgDataPtr() { return (!(Qsize)) ? nullptr : queueStorage[nextInQ].dataPtr; }
