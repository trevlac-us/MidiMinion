
#include "DevicesInterface.h"
#include "USBMidiDriver.h"
#include "DBG.h"


//---------------  Device Manager -------------------

//Static Defs
DataStorage::USBMidiDriverData DeviceManager::usbMidiDevice1Data;
USBMidiDriver DeviceManager::usbMidiDevice1(&DeviceManager::usbMidiDevice1Data);

void DeviceManager::begin() {
	USBHost::begin();
	xDBG::println("DeviceManager Begin");
}
void DeviceManager::task() {
	// in case any drivers need a bit of cpu
	USBHost::Task();
}
void DeviceManager::readMidiMessages() {

	// look for midi messages and add them to the queue
	if (usbMidiDevice1.read()) {
		DeviceMessageQueue::add(Defs::DeviceNumber::USB1, Defs::DeviceEventType::MidiMessage, usbMidiDevice1.getType(), usbMidiDevice1.getCable()
			, usbMidiDevice1.getChannel(), usbMidiDevice1.getData1(), usbMidiDevice1.getData2());
	}
}
void DeviceManager::USBMidiDriverNotifications(USBMidiDriver* caller, Defs::DeviceEventType notificationType) {
	
	Defs::DeviceNumber deviceNumber = Defs::DeviceNumber::ERROR;
	//uint8_t midiType = 0;
	//uint8_t midiCable = 0;
	//uint8_t midiChannel = 0;
	//uint8_t midiData1 = 0;
	//uint8_t midiData2 = 0;

	if (caller == &usbMidiDevice1) {deviceNumber = Defs::DeviceNumber::USB1;}
	//if (caller == &usbMidiDevice2) { deviceNumber = DeviceNumber::USB2; }
	//if (caller == &usbMidiDevice3) { deviceNumber = DeviceNumber::USB3; }
	//if (caller == &usbMidiDevice4) { deviceNumber = DeviceNumber::USB4; }
	//if (caller == &midiDevice1) { deviceNumber = DeviceNumber::MIDI1; }

	DeviceMessageQueue::add( deviceNumber, notificationType);


}
bool DeviceManager::isDeviceReady(Defs::DeviceNumber deviceNumber) {
	switch (deviceNumber) {
	case Defs::DeviceNumber::ERROR: { return false; }
	case Defs::DeviceNumber::USB1: { return usbMidiDevice1.deviceIsReady(); }
	case Defs::DeviceNumber::USB2: { return false; }
	case Defs::DeviceNumber::USB3: { return false; }
	case Defs::DeviceNumber::USB4: { return false; }
	case Defs::DeviceNumber::MIDI1: { return false; }
	default: { return false; }
	}
}
bool DeviceManager::aDeviceIsReady() {
	if (usbMidiDevice1.deviceIsReady()) {return true;	}
	//if (usbMidiDevice2.deviceIsReady()) { rtn++; }
	//if (usbMidiDevice3.deviceIsReady()) { rtn++; }
	//if (usbMidiDevice4.deviceIsReady()) { rtn++; }
	//if (MidiDevice1.deviceIsReady()) { rtn++; }

}

//---------------  Device Manager   END  -----------




//---------------  Device Message Queue  -----------------

bool DeviceMessageQueue::add(Defs::DeviceNumber deviceNumber, Defs::DeviceEventType type
	, uint8_t msgType, uint8_t msgCable, uint8_t msgChannel, uint8_t msgData1, uint8_t msgData2) {
	bool rtn = false;
	__disable_irq();


	if (Qsize < storageSize) {

		queueStorage[nextOpenSpot].deviceNumber = deviceNumber;
		queueStorage[nextOpenSpot].eventType = type;
		queueStorage[nextOpenSpot].midiMsg.type = msgType;
		queueStorage[nextOpenSpot].midiMsg.cable = msgCable;
		queueStorage[nextOpenSpot].midiMsg.channel = msgChannel;
		queueStorage[nextOpenSpot].midiMsg.data1 = msgData1;
		queueStorage[nextOpenSpot].midiMsg.data2 = msgData2;


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
	__enable_irq();
	return rtn;
}
bool DeviceMessageQueue::remove() {
	bool rtn = false;
	__disable_irq();
	if (Qsize > 0) {
		Qsize--;
		//bump up nextInQ ... loop around to 0 if at end of storage
		nextInQ = (nextInQ + 1 == storageSize) ? 0 : nextInQ + 1;
		rtn = true;
	}
	__enable_irq();
	return rtn;
}
bool DeviceMessageQueue::hasMoreWork() {
	if (Qsize <= 0 || countUntilYield <= 0) { countUntilYield = processBeforeYield; return false; }

	countUntilYield--;
	return true;
}

Defs::DeviceEventType DeviceMessageQueue::getNextMsgType() { return (!(Qsize)) ? Defs::DeviceEventType::EmptyQueue : queueStorage[nextInQ].eventType; }
Defs::DeviceNumber DeviceMessageQueue::getNextMsgDeviceNum() { return (!(Qsize)) ? Defs::DeviceNumber::ERROR : queueStorage[nextInQ].deviceNumber; }
Defs::MidiMessageData* DeviceMessageQueue::getNextMsgMidiData() { return (!(Qsize)) ? nullptr : &(queueStorage[nextInQ].midiMsg); }

//---------------  Device Message Queue END  -----------
