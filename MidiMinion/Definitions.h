// USBDefinitions.h

#pragma once
#include "arduino.h"

namespace Defs {


	// --- MIDI MESSAGE ----------------
	typedef struct MidiMessageData_struct {
		uint8_t type;
		uint8_t cable;
		uint8_t channel;
		uint8_t data1;
		uint8_t data2;
	}MidiMessageData;

	//	USB Device Info Summary & Cable Info
	typedef struct EndpointJackMap_struct {
		uint8_t embeddedJackID;
		uint8_t embeddedJackiString;
		uint8_t externalJackID;
		uint8_t externalJackiString;
	} EndpointJackMap_t;
	typedef struct DeviceInfoSummary_struct {
		bool deviceReady;
		// Device Stuff
		uint16_t USBvendorID;
		uint8_t USBMajorVersion;
		uint8_t USBMinorVersion;
		uint8_t USBMinorSubVersion;
		uint8_t manufacturerStringIndex;
		uint8_t productStringIndex;
		uint8_t serialNumStringIndex;
		uint8_t deviceMajorVersion;
		uint8_t deviceMinorVersion;
		uint8_t deviceMinorSubVersion;
		uint8_t numberOfConfigurations;
		// Config Stuff
		uint8_t numberOfInterfaces;
		uint8_t configurationStringIndex;
		bool selfPowered;
		bool remoteWakeup;
		uint16_t maxBusPower_mA;
		// Midi Streaming
		uint8_t interfaceStringIndex;
		uint8_t midiMajorVersion;
		uint8_t midiMinorVersion;
		uint8_t midiMinorSubVersion;
		//MS Endpoint
		uint8_t endpointInID;
		uint8_t numberOfCablesIn;
		EndpointJackMap_t* jackMapIn;
		uint8_t endpointOutID;
		uint8_t numberOfCablesOut;
		EndpointJackMap_t* jackMapOut;
		//Strings
		uint8_t** iStrings;
	} DeviceInfoSummary_t;


	// ---- USBMidiDriver ---

	//types so we know how to overlay descriptor structs on our data buffer
	enum class USBDescriptorStructType : uint8_t {
		Unknown, Device, Config, Interface, Endpoint
		, AC_Interface_Header
		, MS_Interface_Header, MS_Interface_InJack, MS_Interface_OutJack, MS_Interface_Element
		, MS_Endpoint_In, MS_Endpoint_Out
	};


	// ---- DeviceManager ---

	enum class DeviceEventType { ERROR, EmptyQueue, DeviceConnect, DeviceDisconnect };
	enum class DeviceNumber { ERROR, USB1, USB2, USB3, USB4, MIDI1 };


	typedef struct DeviceEvent_struct { Defs::DeviceNumber deviceNumber; Defs::DeviceEventType eventType; const void* dataPtr; } DeviceEvent;
	typedef struct MidiMessage_struct { Defs::DeviceNumber deviceNumber; Defs::MidiMessageData midiMsg; } MidiMessage;

	// ---- UIMessageQueue ---

	enum class UIMessageType { ERROR, EmptyQueue, DeviceConnect, DeviceDisconnect };
	typedef struct UIMessage_struct { Defs::UIMessageType msgType; const void* dataPtr; } UIMessage;

};

namespace USBDefs {
	// ----- ENUMERATIONS ------------
	enum class USB_DESCRIPTOR_TYPE : uint8_t {
		DEVICE = 0x01,
		CONFIGURATION = 0x02,
		STRING = 0x03,
		INTERFACE = 0x04,
		ENDPOINT = 0x05,
		AUDIOCLASS_INTERFACE = 0x24,
		AUDIOCLASS_ENDPOINT = 0x25
	};
	enum class AUDIO_INTERFACE_SUBCLASS : uint8_t
	{
		UNDEFINED = 0x00,
		AUDIOCONTROL = 0x01,
		AUDIOSTREAMING = 0x02,
		MIDISTREAMING = 0x03
	};
	enum class MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE : uint8_t
	{
		UNDEFINED = 0x00,
		HEADER = 0x01,
		IN_JACK = 0x02,
		OUT_JACK = 0x03,
		ELEMENT = 0x04
	};
	enum class MIDISTREAMING_INOUTJACK_TYPE : uint8_t
	{
		JACK_TYPE_UNDEFINED = 0x00,
		EMBEDDED = 0x01,
		EXTERNAL = 0x02
	};
	enum class MS_EP_DESCRIPTOR_SUBTYPE : uint8_t
	{
		UNDEFINED = 0x00,
		GENERAL = 0x01
	};
	enum class AUDIOCONTROL_INTERFACE_DESCRIPTOR_SUBTYPE : uint8_t
	{
		UNDEFINED = 0x00,
		HEADER = 0x01,
		INPUT_TERMINAL = 0x02,
		OUTPUT_TERMINAL = 0x03,
		MIXER_UNIT = 0x04,
		SELECTOR_UNIT = 0x05,
		FEATURE_UNIT = 0x06,
		PROCESSING_UNIT = 0x07,
		EXTENSION_UNIT = 0x08
	};

	// These are the lengths of the structs.  
	// NOTE: some descriptors have a variable amount of additional bytes
	// NOTE2: not an enum class so we dont have to cast to send these to host functions or size arrays
	namespace DESCRIPTOR_LENGTH {
		enum DESCRIPTOR_LENGTHS : uint8_t
		{
			DEVICE = 18,
			CONFIG = 9,
			INTERFACE = 9,
			ENDPOINT = 9,
			AC_INTERFACE_HEADER = 9,
			MS_INTERFACE_HEADER = 7,
			MS_INTERFACE_INJACK = 6,
			MS_INTERFACE_OUTJACK = 8,	//can be 8+ see struct
			MS_INTERFACE_ELEMENT = 7,	//can be 7+ see struct
			MS_ENDPOINT_GENERAL = 5,		//can be 5+ see struct
			ISTRING_MAX = 255

		};
	}




	// --- USB DESCRIPTOR DATA STRUCTURES ------------------------

	typedef struct DescriptorHeader_struct {
		uint8_t bLength;
		USB_DESCRIPTOR_TYPE bDescriptorType;
	} DescriptorHeader_t;

	typedef struct StringDescriptor_struct {
		DescriptorHeader_t header;
		uint8_t bUnicodeLSB;
		uint8_t bUnicodeMSB;
	} StringStruct;
	typedef struct DeviceDescriptor_struct {
		DescriptorHeader_t header;
		uint8_t bcdUSBLSB;
		uint8_t bcdUSBMSB;
		uint8_t bDeviceClass;
		uint8_t bDeviceSubClass;
		uint8_t bDeviceProtocol;
		uint8_t bMaxPacketSize0;
		uint8_t idVendorLSB;
		uint8_t idVendorMSB;
		uint8_t idProductLSB;
		uint8_t idProductMSB;
		uint8_t bcdDeviceLSB;
		uint8_t bcdDeviceMSB;
		uint8_t iManufacturer;
		uint8_t iProduct;
		uint8_t iSerialNumber;
		uint8_t bNumConfigurations;
	} DeviceStruct;
	typedef struct ConfigDescriptor_struct {
		DescriptorHeader_t header;
		uint8_t bTotalLengthLSB;
		uint8_t bTotalLengthMSB;
		uint8_t bNumInterfaces;
		uint8_t bConfigurationValue;
		uint8_t iConfiguration;
		uint8_t bmAttributes;
		uint8_t bMaxPower; // (250mA)
	} ConfigStruct;
	typedef struct InterfaceDescriptor_struct {
		DescriptorHeader_t header;
		uint8_t bInterfaceNumber;
		uint8_t bAlternateSetting;
		uint8_t bNumEndpoints;
		uint8_t bInterfaceClass;
		AUDIO_INTERFACE_SUBCLASS bInterfaceSubClass;
		uint8_t bInterfaceProtocol;
		uint8_t iInterface;
	} InterfaceStruct;
	typedef struct EndpointDescriptor_struct {
		DescriptorHeader_t header;
		uint8_t bEndpointAddress;
		uint8_t bmAttributes;
		uint8_t wMaxPacketSizeLSB;
		uint8_t wMaxPacketSizeMSB;
		uint8_t bInterval;
		uint8_t bRefresh;
		uint8_t bSynchAddress;
	} EndPointStruct;
	typedef struct AudioControlInterfaceHeaderDescriptor_struct {
		DescriptorHeader_t header;
		AUDIOCONTROL_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		uint8_t bcdAudioSpecLSB;
		uint8_t bcdAudioSpecMSB;
		uint8_t bTotalLengthLSB;
		uint8_t bTotalLengthMSB;
		uint8_t bInCollection;
		uint8_t baInterfaceNr;
	} AC_InterfaceHeaderStruct;
	typedef struct MidiStreamingInterfaceHeaderDescriptor_struct {
		DescriptorHeader_t header;
		MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		uint8_t bcdMidiSpecLSB;
		uint8_t bcdMidiSpecMSB;
		uint8_t bTotalLengthLSB;
		uint8_t bTotalLengthMSB;
	} MS_InterfaceHeaderStruct;
	typedef struct MidiStreamingInterfaceInJackDescriptor_struct {
		DescriptorHeader_t header;
		MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		MIDISTREAMING_INOUTJACK_TYPE bJackType;
		uint8_t bJackID;
		uint8_t iJack;
	} MS_InterfaceInJackStruct;
	typedef struct MidiStreamingInterfaceOutJackDescriptor_struct {
		DescriptorHeader_t header;
		MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		MIDISTREAMING_INOUTJACK_TYPE bJackType;
		uint8_t bJackID;
		uint8_t bNrInputPins;
		uint8_t baSourceID1;  // There are 2x bNrInputPins
		uint8_t BaSourcePin1; //   of these so not fixed data length
		
		//uint8_t iJack;		-- unfortunately this is then at an unknown point
		//						   bLength-1 should work ...
	} MS_InterfaceOutJackStruct;
	typedef struct MidiStreamingInterfaceElementDescriptor_struct {
		DescriptorHeader_t header;
		MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		uint8_t bElementID;
		uint8_t bNrInputPins;
		uint8_t baSourceID1;  // There are 2x bNrInputPins
		uint8_t BaSourcePin1; //   of these so not fixed
		
		//uint8_t bNrOutputPins;	-- could get at these via (bLength - 4) etc.
		//uint8_t bInTerminalLink;
		//uint8_t bOutTerminalLink;
		//uint8_t bElCapsSize;	   
	} MS_InterfaceElementStruct;
	typedef struct MidiStreamingEndpointGeneralDescriptor_struct {
		DescriptorHeader_t header;
		MS_EP_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
		uint8_t bNumEmbMIDJack;
		uint8_t baAssocJackID1;	// There are 1 x bNumEmbMIDIJack of these
								//  so not fixed
	} MS_EndpointGeneralStruct;
	typedef struct ClassSpecificMSUnknownDescriptor_struct {
		DescriptorHeader_t header;
		MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE bDescriptorSubtype;
	} MS_UnknownInterfaceStruct;
	typedef struct UnknownDescriptor_struct {
		DescriptorHeader_t header;
	} UnknownStruct;


};

//Audio Interface Subclass Codes(audio 1.0, page 99)
//Table A - 2: Audio Interface Subclass Codes
//SUBCLASS_UNDEFINED	0x00
//AUDIOCONTROL	0x01
//AUDIOSTREAMING	0x02
//MIDISTREAMING	0x03
// Audio Class-Specific Descriptor Types (audio 1.0, page 99)
//   CS_UNDEFINED     0x20
//   CS_DEVICE        0x21
//   CS_CONFIGURATION 0x22
//   CS_STRING        0x23
//   CS_INTERFACE     0x24
//   CS_ENDPOINT      0x25
// MS Class-Specific Interface Descriptor Subtypes (midi 1.0, page 36)
//   MS_DESCRIPTOR_UNDEFINED 0x00
//   MS_HEADER               0x01
//   MIDI_IN_JACK            0x02
//   MIDI_OUT_JACK           0x03
//   ELEMENT                 0x04
// MS Class-Specific Endpoint Descriptor Subtypes (midi 1.0, page 36)
//   DESCRIPTOR_UNDEFINED 0x00
//   MS_GENERAL           0x01
// MS MIDI IN and OUT Jack types (midi 1.0, page 36)
//   JACK_TYPE_UNDEFINED 0x00
//   EMBEDDED            0x01
//   EXTERNAL            0x02
// Endpoint Control Selectors (midi 1.0, page 36)
//   EP_CONTROL_UNDEFINED 0x00
//   ASSOCIATION_CONTROL  0x01
//A.5 Audio Class - Specific AudioControl Interface Descriptor Subtypes (audio 1.0, page 100)
//Table A - 5: Audio Class - Specific AC Interface Descriptor Subtypes
//	AC_DESCRIPTOR_UNDEFINED	0x00
//	HEADER	0x01
//	INPUT_TERMINAL	0x02
//	OUTPUT_TERMINAL	0x03
//	MIXER_UNIT	0x04
//	SELECTOR_UNIT	0x05
//	FEATURE_UNIT	0x06
//	PROCESSING_UNIT	0x07
//	EXTENSION_UNIT	0x08
