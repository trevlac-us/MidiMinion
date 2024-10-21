
#pragma once
#include "Definitions.h"
class USBMidiDriver;	//-- DeviceManager needed for Declare & device callback
#include "DataStorage.h"


	//---------------  DeviceEventQueue     ------------
	class DeviceEventQueue {
	private:
		static inline Defs::DeviceEvent queueStorage[10]={};
		static const uint8_t storageSize = sizeof(queueStorage) / sizeof(queueStorage[0]);
		static inline volatile uint8_t Qsize = 0;
		static inline volatile uint8_t nextInQ = 0;
		static inline volatile uint8_t nextOpenSpot = 0;
		static inline const uint8_t processBeforeYield = 4;
		static inline volatile uint8_t countUntilYield = processBeforeYield;

	public:
		static bool add(Defs::DeviceNumber deviceNumber, Defs::DeviceEventType type, void* dataPtr);

		static bool remove();
		static bool hasMoreWork();

		static Defs::DeviceEventType getNextEventType();
		static Defs::DeviceNumber getNextEventDeviceNum();
		static void* getNextEventDataPtr();
	};
	//---------------  DeviceEventQueue    -------  END CLASS ------------


	//---------------  MidiMsgQueue     ------------
	class MidiMsgQueue {
	private:
		static inline Defs::MidiMessage queueStorage[20] = {};
		static const uint8_t storageSize = sizeof(queueStorage) / sizeof(queueStorage[0]);
		static inline volatile uint8_t Qsize = 0;
		static inline volatile uint8_t nextInQ = 0;
		static inline volatile uint8_t nextOpenSpot = 0;
		static inline const uint8_t processBeforeYield = 10;
		static inline volatile uint8_t countUntilYield = processBeforeYield;

	public:
		static bool add(Defs::DeviceNumber deviceNumber
			, uint8_t msgType = 0, uint8_t msgCable = 0, uint8_t msgChannel = 0, uint8_t msgData1 = 0, uint8_t msgData2 = 0);

		static bool remove();
		static bool hasMoreWork();

		static Defs::DeviceNumber getNextMsgDeviceNum();
		static Defs::MidiMessageData* getNextMsgMidiData();
	};
	//---------------  MidiMsgQueue    -------  END CLASS ------------


	//---------------  DeviceManager     ------------
	class DeviceManager {

	private:
		static const bool PrintClassDEBUG = true;
		static const bool PrintClassERROR = true;

		static DataStorage::USBMidiDriverData usbMidiDevice1Data;
		static USBMidiDriver usbMidiDevice1;

	public:

		static void begin();
		static void task();
		static void readMidiMessages();
		static void USBMidiDriverNotifications(USBMidiDriver* caller, Defs::DeviceEventType type);
		static bool isDeviceReady(Defs::DeviceNumber deviceNumber);
		static bool aDeviceIsReady();
	};
	//---------------  DeviceManager    -------  END CLASS ------------










