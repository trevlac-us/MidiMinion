#pragma once
#include "Definitions.h"
#include "DataStorage.h"


//---------------  UIMessageQueue     ------------
class UIMessageQueue {
private:
	static const bool PrintClassDEBUG = false;
	static const bool PrintClassERROR = true;

	static inline Defs::UIMessage queueStorage[50] = {};
	static const uint8_t storageSize = sizeof(queueStorage) / sizeof(queueStorage[0]);
	static inline volatile uint8_t Qsize = 0;
	static inline volatile uint8_t nextInQ = 0;
	static inline volatile uint8_t nextOpenSpot = 0;

public:
	static bool add(Defs::UIMessageType msgType, const void* dataPtr );
	static Defs::UIMessage remove();

	static Defs::UIMessageType peekMsgType();
};
//---------------  UIMessageQueue    -------  END CLASS ------------



//---------------  UIMessageHandler     ------------

//  enum class UIMessageType { ERROR, EmptyQueue, DeviceConnect, DeviceDisconnect };
class UIPrintHandler  {
private:
	static const bool PrintClassDEBUG = true;
	static const bool PrintClassERROR = true;

public:
	static void deviceConnect(const Defs::DeviceInfoSummary_t* pDeviceInfoSumStruct) ;
	static void deviceDisconnect(const Defs::DeviceInfoSummary_t* pDeviceInfoSumStruct) ;

};

class UIMessageRouter {
private:
	static const bool PrintClassDEBUG = false;
	static const bool PrintClassERROR = true;
	static UIPrintHandler handler;


public:
	static bool routeMessage(Defs::UIMessage msg) {
		bool rtn = false;

		switch (msg.msgType) {
			case Defs::UIMessageType::EmptyQueue: {rtn = false;	break;}

			case Defs::UIMessageType::DeviceConnect: {
				handler.deviceConnect(reinterpret_cast<const Defs::DeviceInfoSummary_t*>(msg.dataPtr));
				rtn = true;
				break;
			}

			case Defs::UIMessageType::DeviceDisconnect: {
				handler.deviceDisconnect(reinterpret_cast<const Defs::DeviceInfoSummary_t*>(msg.dataPtr));
				rtn = true;
				break;
			}
			default: {
				xERR::println("ERROR - UIMsgRouter - Unknown Msg Type :", (uint8_t) msg.msgType);
				rtn = true;
				break;
			}
		}
		return rtn;
	} // end function routeMessages
};

//---------------  UIMessageHandler END    ------------
