#pragma once
#include "Definitions.h"

// Change this to print to Serial, USBSerial1, maybe a log stream ...
#define DBGSerial Serial

// Use " xDBG::print() " for debug prints in a class
// above resolves to --> if(PrintClassDEBUG) DBG::print()
// class must include DBG.h and define --> private: const uint8_t PrintClassDEBUG = 1;
// 
// Use " xDBGS(MidiMinion)::print() " in global loop() ...  
// above resolves to --> if(MidiMinion::PrintClassDEBUG) DBG::print()

#define xDBG if(PrintClassDEBUG) DBG
#define xDBGS(cls) if(cls::PrintClassDEBUG) DBG
#define xERR if(PrintClassERROR) DBG
#define xERRS(cls) if(cls::PrintClassERROR) DBG


class DBG {

public:

	static void print_hexbytes(const void* ptr, uint32_t len)
	{
		if (ptr == NULL || len == 0) return;
		const uint8_t* p = (const uint8_t*)ptr;
		do {
			if (*p < 16) DBGSerial.print('0');
			DBGSerial.print(*p++, HEX);
			DBGSerial.print(' ');
		} while (--len);
		DBGSerial.println();
	}
	static void printDescriptorHeader(const char* lbl, USBDefs::DescriptorHeader_t* pHeader) {
		DBGSerial.print("--- "); DBGSerial.print(lbl); DBGSerial.println(" Descriptor ---");
		DBG::print_hexbytes(pHeader, pHeader->bLength);
		DBG::println("   bLength         =\t", pHeader->bLength);
		DBG::println("   bDescriptorType =\t0x", pHeader->bDescriptorType, HEX);
	}
	static void print(USBDefs::UnknownStruct* pDescriptor) {
		DBG::printDescriptorHeader("Unknown", &(pDescriptor->header));
	}
	static void print(USBDefs::EndPointStruct* pEndpointDescriptor) {
		uint16_t maxPacketSize = pEndpointDescriptor->wMaxPacketSizeMSB << 8 | pEndpointDescriptor->wMaxPacketSizeLSB;

		DBG::printDescriptorHeader("Endpoint", &(pEndpointDescriptor->header));
		DBG::println("   bEndpointAddress =\t0x", pEndpointDescriptor->bEndpointAddress, HEX);
		DBG::println("   bmAttributes     =\t0b", pEndpointDescriptor->bmAttributes, BIN);
		DBG::println("   PipeMaxPacketSz  =\t", maxPacketSize);
		DBG::println("   bInterval        =\t0x", pEndpointDescriptor->bInterval, HEX);
		DBG::println("   bRefresh         =\t0x", pEndpointDescriptor->bRefresh, HEX);
		DBG::println("   bSynchAddress    =\t0x", pEndpointDescriptor->bSynchAddress, HEX);
	}
	static void print(USBDefs::MS_InterfaceHeaderStruct* pDescriptor) {
		uint16_t totalLen = pDescriptor->bTotalLengthMSB << 8 | pDescriptor->bTotalLengthLSB;

		DBG::printDescriptorHeader("Midi Streaming Interface Header", &(pDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pDescriptor->bDescriptorSubtype, HEX);
		DBG::print("   bcdMidiSpecVersion =\t", pDescriptor->bcdMidiSpecMSB, HEX); DBG::println(".", pDescriptor->bcdMidiSpecLSB, HEX);
		DBG::println("   totalLength        =\t", totalLen);
	}
	static void print(USBDefs::MS_InterfaceInJackStruct* pDescriptor, uint8_t* const iStrings[]) {
		DBG::printDescriptorHeader("Midi Streaming Interface In Jack", &(pDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pDescriptor->bDescriptorSubtype, HEX);
		DBG::println("   bJackType          =\t0x", pDescriptor->bJackType, HEX);
		DBG::println("   bJackID            =\t0x", pDescriptor->bJackID, HEX);
		DBG::print("   iJack              =\t[", pDescriptor->iJack); DBGSerial.println("] "); DBG::println((const char*)iStrings[pDescriptor->iJack]);
	}
	static void print(USBDefs::MS_InterfaceOutJackStruct* pDescriptor, uint8_t* const iStrings[]) {

		// Get at last byte
		uint8_t* bytePtr = (uint8_t*)pDescriptor;
		uint8_t iJack = bytePtr[(pDescriptor->header.bLength) - 1];

		DBG::printDescriptorHeader("Midi Streaming Interface Out Jack", &(pDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pDescriptor->bDescriptorSubtype, HEX);
		DBG::println("   bJackType          =\t0x", pDescriptor->bJackType, HEX);
		DBG::println("   bJackID            =\t0x", pDescriptor->bJackID, HEX);
		DBG::println("   bNrInputPins       =\t0x", pDescriptor->bNrInputPins, HEX);
		DBG::println("   baSourceID1        =\t0x", pDescriptor->baSourceID1, HEX);
		DBG::println("   BaSourcePin1       =\t0x", pDescriptor->BaSourcePin1, HEX);
		DBG::print("   iJack              =\t[", iJack, DEC); DBGSerial.println("] "); DBG::println((const char*)iStrings[iJack]);

	}
	static void print(USBDefs::MS_InterfaceElementStruct* pDescriptor) {
		DBG::printDescriptorHeader("Midi Streaming Interface Element", &(pDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pDescriptor->bDescriptorSubtype, HEX);
		DBG::println("   bElementID          =\t0x", pDescriptor->bElementID, HEX);
		DBG::println("   bNrInputPins       =\t0x", pDescriptor->bNrInputPins, HEX);
		DBG::println("   baSourceID1        =\t0x", pDescriptor->baSourceID1, HEX);
		DBG::println("   BaSourcePin1       =\t0x", pDescriptor->BaSourcePin1, HEX);
	}
	static void print(USBDefs::MS_EndpointGeneralStruct* pDescriptor) {
		DBG::printDescriptorHeader("Midi Streaming Endpoint General", &(pDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pDescriptor->bDescriptorSubtype, HEX);
		DBG::println("   bNumEmbMIDJack     =\t0x", pDescriptor->bNumEmbMIDJack, HEX);
		DBG::println("   baAssocJackID1     =\t0x", pDescriptor->baAssocJackID1, HEX);
	}

	static void print(USBDefs::DeviceStruct* pDeviceDescriptor, uint8_t* const iStrings[]) {
		//, const char* pManufacturer, const char* pProduct, const char* pSerialNum) {
		uint16_t idVendor = pDeviceDescriptor->idVendorMSB << 8 | pDeviceDescriptor->idVendorLSB;
		uint16_t idProduct = pDeviceDescriptor->idProductMSB << 8 | pDeviceDescriptor->idProductLSB;

		DBG::printDescriptorHeader("Device", &(pDeviceDescriptor->header));
		DBG::print("   USBVersion      =\t", pDeviceDescriptor->bcdUSBMSB, HEX); DBG::println(".", pDeviceDescriptor->bcdUSBLSB, HEX);
		DBG::println("   bDeviceClass    =\t0x", pDeviceDescriptor->bDeviceClass, HEX);
		DBG::println("   bDeviceSubClass =\t0x", pDeviceDescriptor->bDeviceSubClass, HEX);
		DBG::println("   bDeviceProtocol =\t0x", pDeviceDescriptor->bDeviceProtocol, HEX);
		DBG::println("   CtlPipeMaxPacketSz =\t", pDeviceDescriptor->bMaxPacketSize0);
		DBG::println("   idVendor        =\t", idVendor);
		DBG::println("   idProduct       =\t", idProduct);
		DBG::print("   DeviceVersion   =\t", pDeviceDescriptor->bcdDeviceMSB, HEX); DBG::println(".", pDeviceDescriptor->bcdDeviceLSB, HEX);
		DBG::print("   iManufacturerIndx =\t[", pDeviceDescriptor->iManufacturer); DBGSerial.print("] "); DBG::println((const char*) iStrings[pDeviceDescriptor->iManufacturer]);
		DBG::print("   iProductIndex   =\t[", pDeviceDescriptor->iProduct);        DBGSerial.print("] "); DBG::println((const char*) iStrings[pDeviceDescriptor->iProduct]);
		DBG::print("   iSerialNumberIndx =\t[", pDeviceDescriptor->iSerialNumber); DBGSerial.print("] "); DBG::println((const char*) iStrings[pDeviceDescriptor->iSerialNumber]);
		DBG::println("   bNumConfigurations =\t", pDeviceDescriptor->bNumConfigurations);

	}
	static void print(USBDefs::ConfigStruct* pConfigDescriptor, uint8_t* const iStrings[]) {
		uint16_t totalLen = pConfigDescriptor->bTotalLengthMSB << 8 | pConfigDescriptor->bTotalLengthLSB;
		uint16_t maxPower = pConfigDescriptor->bMaxPower << 1;

		DBG::printDescriptorHeader("Config", &(pConfigDescriptor->header));
		DBG::println("   totalLength     =\t", totalLen);
		DBG::println("   bNumInterfaces  =\t", pConfigDescriptor->bNumInterfaces);
		DBG::println("   bConfigurationValue =\t", pConfigDescriptor->bConfigurationValue);
		DBG::print("   iConfigurationIndx =\t[", pConfigDescriptor->iConfiguration); DBGSerial.print("] "); DBG::println((const char*)iStrings[pConfigDescriptor->iConfiguration]);
		DBG::println("   bmAttributes    =\t0b", pConfigDescriptor->bmAttributes, BIN);
		if (((pConfigDescriptor->bmAttributes & 0b10000000) >> 7)) DBG::println("   \t0b1xxxxxxx = BUS Powered");
		if (((pConfigDescriptor->bmAttributes & 0b01000000) >> 6)) DBG::println("   \t0bx1xxxxxx = Self Powered");
		if (((pConfigDescriptor->bmAttributes & 0b00100000) >> 5)) DBG::println("   \t0bxx1xxxxx = Remote Wakeup");
		DBG::print("   MaxPower       =\t", maxPower); DBGSerial.println("mA");


	}
	static void print(USBDefs::InterfaceStruct* pInterfaceDescriptor, uint8_t* const iStrings[]) {

		DBG::printDescriptorHeader("Interface", &(pInterfaceDescriptor->header));
		DBG::println("   bInterfaceNumber =\t", pInterfaceDescriptor->bInterfaceNumber);
		DBG::println("   bAlternateSetting =\t", pInterfaceDescriptor->bAlternateSetting);
		DBG::println("   bNumEndpoints   =\t", pInterfaceDescriptor->bNumEndpoints);
		DBG::println("   bInterfaceClass =\t0x", pInterfaceDescriptor->bInterfaceClass, HEX);
		DBG::println("   bInterfaceSubClass =\t0x", pInterfaceDescriptor->bInterfaceSubClass, HEX);
		DBG::println("   bInterfaceProtocol =\t0x", pInterfaceDescriptor->bInterfaceProtocol, HEX);
		DBG::print("   iInterfaceIndex =\t[", pInterfaceDescriptor->iInterface);  DBGSerial.println("] "); DBG::println((const char*)iStrings[pInterfaceDescriptor->iInterface]);
	}
	static void print(USBDefs::AC_InterfaceHeaderStruct* pInterfaceDescriptor) {
		uint16_t totalLen = pInterfaceDescriptor->bTotalLengthMSB << 8 | pInterfaceDescriptor->bTotalLengthLSB;

		DBG::printDescriptorHeader("Audio Control Interface Header", &(pInterfaceDescriptor->header));
		DBG::println("   bDescriptorSubtype =\t0x", pInterfaceDescriptor->bDescriptorSubtype, HEX);
		DBG::print("   bcdAudioSpecVersion =\t", pInterfaceDescriptor->bcdAudioSpecMSB, HEX); DBG::println(".", pInterfaceDescriptor->bcdAudioSpecLSB, HEX);
		DBG::println("   totalLength     =\t", totalLen);
		DBG::println("   num_ASandMS_interfaces =\t", pInterfaceDescriptor->bInCollection);
		DBG::println("   firstIfaceNum   =\t", pInterfaceDescriptor->baInterfaceNr);

	}

	static void printDeviceInfoSummary(Defs::DeviceInfoSummary_t* pSumStruct) {
		DBG::println("****** Device Summary ******");
		DBG::println("   USB-IF Vendor ID:   ", pSumStruct->USBvendorID);
		DBG::print(  "   USB Version:        ", pSumStruct->USBMajorVersion); 
								DBG::print(  ".", pSumStruct->USBMinorVersion);
								DBG::println(".", pSumStruct->USBMinorSubVersion);
		DBG::print("   Manufacturer:       "); (pSumStruct->manufacturerStringIndex) 
									? DBG::println((const char*)pSumStruct->iStrings[pSumStruct->manufacturerStringIndex]) 
									: DBG::println("<Not Available>");
		DBG::print("   Product:            "); (pSumStruct->productStringIndex)
									? DBG::println((const char*)pSumStruct->iStrings[pSumStruct->productStringIndex])
									: DBG::println("<Not Available>");
		DBG::print("   Serial Num:         "); (pSumStruct->serialNumStringIndex)
									? DBG::println((const char*)pSumStruct->iStrings[pSumStruct->serialNumStringIndex])
									: DBG::println("<Not Available>");
		DBG::print(  "   Device Version:     ", pSumStruct->deviceMajorVersion); 
									DBG::print(".", pSumStruct->deviceMinorVersion);
									DBG::println(".", pSumStruct->deviceMinorSubVersion);
		DBG::println("   #of Configurations: ", pSumStruct->numberOfConfigurations);
		DBG::println("");
		DBG::println("   ** Default Config **");
		DBG::print("      Configuration Name:   "); (pSumStruct->configurationStringIndex)
									? DBG::println((const char*)pSumStruct->iStrings[pSumStruct->configurationStringIndex])
									: DBG::println("<Not Available>");
		DBG::println("      #of Interfaces:       ", pSumStruct->numberOfInterfaces);
		DBG::print(  "      Self Powered:         "); (pSumStruct->selfPowered) ? DBG::println("Yes") : DBG::println("No");
		DBG::print(  "      Remote Wakeup:        "); (pSumStruct->remoteWakeup) ? DBG::println("Yes") : DBG::println("No");
		DBG::print(  "      Max Bus Power:        ", pSumStruct->maxBusPower_mA); DBG::println("mA");
		DBG::println("");
		DBG::println("   ** Midi Streaming Interface **");
		DBG::print("      Name:                 "); (pSumStruct->interfaceStringIndex)
									? DBG::println((const char*)pSumStruct->iStrings[pSumStruct->interfaceStringIndex])
									: DBG::println("<Not Available>"); 
		DBG::print(  "      USB Midi Version:     ", pSumStruct->midiMajorVersion); 
									DBG::print(".", pSumStruct->midiMinorVersion);
									DBG::println(".", pSumStruct->midiMinorSubVersion);
		DBG::println("");
		DBG::println("**** Endpoint Mapping ****");
		DBG::println("# of OUT Cables:     ", pSumStruct->numberOfCablesOut);
		for (uint8_t i = 0; i < pSumStruct->numberOfCablesOut; ++i) {
			DBG::print("Host-->[Out Ep (ID=", pSumStruct->endpointOutID); DBG::print(")]-->");
			DBG::print("[Embed In Jack (ID=", pSumStruct->jackMapOut[i].embeddedJackID); DBG::print(")");

			if (pSumStruct->jackMapOut[i].embeddedJackiString) {
				DBG::print("\"");
				DBG::print((const char*)pSumStruct->iStrings[pSumStruct->jackMapOut[i].embeddedJackiString]);
				DBG::print("\"");
			}
			DBG::print("]-->"); 

 			DBG::print("[Ext Out Jack (ID=", pSumStruct->jackMapOut[i].externalJackID); DBG::print(")"); 
			if (pSumStruct->jackMapOut[i].externalJackiString) {
				DBG::print("\"");
				DBG::print((const char*)pSumStruct->iStrings[pSumStruct->jackMapOut[i].externalJackiString]);
				DBG::print("\"");
			}
			DBG::println("]");
		} // end for loop out cables

		DBG::println("# of IN Cables:      ", pSumStruct->numberOfCablesIn);
		for (uint8_t i = 0; i < pSumStruct->numberOfCablesIn; ++i) {
			DBG::print("Host<--[In Ep (ID=", pSumStruct->endpointInID); DBG::print(")]<--");
			DBG::print("[Embed Out Jack (ID=", pSumStruct->jackMapIn[i].embeddedJackID); DBG::print(")");
			if (pSumStruct->jackMapIn[i].embeddedJackiString) {
				DBG::print("\"");
				DBG::print((const char*)pSumStruct->iStrings[pSumStruct->jackMapIn[i].embeddedJackiString]);
				DBG::print("\"");
			}
			DBG::print("]<--"); 

			DBG::print("[Ext In Jack (ID=", pSumStruct->jackMapIn[i].externalJackID); DBG::print(")");
			if (pSumStruct->jackMapIn[i].externalJackiString) {
				DBG::print("\"");
				DBG::print((const char*)pSumStruct->iStrings[pSumStruct->jackMapIn[i].externalJackiString]);
				DBG::print("\"");
			}
			DBG::print("]"); 
			DBG::println("");
		}// end for loop In cables

	}


    static void print(const char* s) { DBGSerial.print(s); }
    static void print(int n) { DBGSerial.print(n); }
    static void print(unsigned int n) { DBGSerial.print(n); }
    static void print(long n) { DBGSerial.print(n); }
    static void print(unsigned long n) { DBGSerial.print(n); }
    static void println(const char* s) { DBGSerial.println(s); }
    static void println(int n) { DBGSerial.println(n); }
    static void println(unsigned int n) { DBGSerial.println(n); }
    static void println(long n) { DBGSerial.println(n); }
    static void println(unsigned long n) { DBGSerial.println(n); }
    static void println() { DBGSerial.println(); }
    static void print(uint32_t n, uint8_t b) { DBGSerial.print(n, b); }
    static void println(uint32_t n, uint8_t b) { DBGSerial.println(n, b); }
    static void print(const char* s, int n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.print(n, b);}
    static void print(const char* s, unsigned int n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.print(n, b);}
    static void print(const char* s, long n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.print(n, b);}
    static void print(const char* s, unsigned long n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.print(n, b);}
    static void println(const char* s, int n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.println(n, b);}
    static void println(const char* s, unsigned int n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.println(n, b);}
    static void println(const char* s, long n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.println(n, b);}
    static void println(const char* s, unsigned long n, uint8_t b = DEC) {DBGSerial.print(s); DBGSerial.println(n, b);}
	static void println(const char* s, const char* s2) { DBGSerial.print(s); DBGSerial.println(s2); }


	static void println(const char* s, USBDefs::USB_DESCRIPTOR_TYPE n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t) n, b); }
	static void println(const char* s, USBDefs::MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t)n, b); }
	static void println(const char* s, USBDefs::MIDISTREAMING_INOUTJACK_TYPE n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t)n, b); }
	static void println(const char* s, USBDefs::MS_EP_DESCRIPTOR_SUBTYPE n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t)n, b); }
	static void println(const char* s, USBDefs::AUDIO_INTERFACE_SUBCLASS n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t)n, b); }
	static void println(const char* s, USBDefs::AUDIOCONTROL_INTERFACE_DESCRIPTOR_SUBTYPE n, uint8_t b = DEC) { DBGSerial.print(s); DBGSerial.println((uint8_t)n, b); }

};
