#pragma once
#include "Definitions.h"
#include "DataStorage.h"


//---------------  UIMessageQueue     ------------
class UIMessageQueue {
private:
	static inline Defs::UIMessage queueStorage[50] = {};
	static const uint8_t storageSize = sizeof(queueStorage) / sizeof(queueStorage[0]);
	static inline volatile uint8_t Qsize = 0;
	static inline volatile uint8_t nextInQ = 0;
	static inline volatile uint8_t nextOpenSpot = 0;

public:
	static bool add(Defs::UIMessageType msgType, void* dataPtr );
	static bool remove();

	static Defs::UIMessageType getNextMsgType();
	static void* getNextMsgDataPtr();
};
//---------------  DeviceMessageQueue    -------  END CLASS ------------