
#pragma once

#include <USBHost_t36.h>				//-- need MIDIDevice & USBHost
class DeviceManager;					//-- ptr for connect/disconnect callback
#include "DataStorage.h"
#include "Definitions.h"

//---------------  USBMidiDriver    -------  CLASS START ------------
class USBMidiDriver : public MIDIDevice {

// Use xDBG::print() for debug prints in a class
private: static const bool PrintClassDEBUG = true;
private: static const bool PrintClassERROR = true;

//---------------  USBMidiDriver    -------  DATA STOARGE START ------------
private:
	// buffers for descriptor requests and storage of results
	//
	setup_t USBsetupPacket __attribute__((aligned(16)));
	DataStorage::USBMidiDriverData* myData;

public: 
	USBMidiDriver(DataStorage::USBMidiDriverData* data) : MIDIDevice(*(static_cast<USBHost*>(this))) , myData(data) { init(); };
	bool deviceIsReady() const { return myData->deviceReady; }

// Parent "USBHost_t36::MIDIDevice" methods we are using ... 
protected:
	virtual bool claim(Device_t* dev, int type, const uint8_t* descriptors, uint32_t len);
	virtual void control(const Transfer_t* transfer);
	virtual void disconnect();


private:
	void fireEvent(Defs::DeviceEventType e);
	bool moreStringsToRequest(Device_t* dev);
	void requestDeviceDescriptor(Device_t* dev);
	void requestConfigDescriptor(Device_t* dev, uint16_t lengthToRequest);
	void requestStringDescriptor(Device_t* dev, uint8_t stringIndex);

	enum controlCallbackType { ERROR, DEVICE, CONFIG_HEADER, CONFIG_FULL, STRING };
	controlCallbackType findControlCallbackType(const Transfer_t* transfer);

};
//---------------  USBMidiDriver    -------  CLASS END ------------

//---------------  DescriptorHelper    -------  CLASS START ------------
class DescriptorHelper {
private:

	// Use xDBG::print() for debug prints in a class
	static const bool PrintClassDEBUG = true;
	static const bool PrintClassERROR = true;

	static Defs::USBDescriptorStructType getDescriptorStructType(USBDefs::UnknownStruct* ukD
		, USBDefs::InterfaceStruct* iD
		, USBDefs::EndPointStruct* epD);

	static void validateIString(DataStorage::USBMidiDriverData* data, uint8_t* idx);


public:
	static void parseConfigData(DataStorage::USBMidiDriverData*);
	static void buildStackOfStringsToProcess(DataStorage::USBMidiDriverData*);
	static void saveStringDescriptor(DataStorage::USBMidiDriverData* data, uint8_t stringIndex);


};
//---------------  DescriptorHelper    -------  CLASS END ------------








