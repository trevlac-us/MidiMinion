#pragma once
#include "DBG.h"


class DataStorage {
 public:

	 class USBMidiDriverData {
	 private:
		 static const bool PrintClassDEBUG = true;
		 static const bool PrintClassERROR = true;

	//---------------  USBMidiDriver    -------  DATA STOARGE START ------------

		// buffers for descriptor requests and storage of results
		//
		uint8_t configDescriptorBuff[2048] __attribute__((aligned(16)));
		uint8_t deviceDescriptorBuff[USBDefs::DESCRIPTOR_LENGTH::DEVICE] __attribute__((aligned(16)));

		//When parsing the descriptors, fill this array with a pointer 
		//   to the descriptor's start in the buffer, and the descriptor type that is there
		struct descriptorData { void* dataPtr;	Defs::USBDescriptorStructType type; }
		descriptorArray[40]{ nullptr, Defs::USBDescriptorStructType::Unknown };
		uint8_t nextArrayIndex = 0;


		// ---- iString Data from Device Descriptors -----------

			//buffer to recieve a Unicode string from the device
			//
			uint8_t stringDescriptorBuff[USBDefs::DESCRIPTOR_LENGTH::ISTRING_MAX] __attribute__((aligned(16)));

			//buffer to hold the strings as ascii after they are requested ... limit to ~ 11*127
			//
			uint8_t iSB[1400]{};

			//Array of pointers to strings in the above buffer.  
			//   NOTE:USB Spec string index 0 means no string. so we keep 1st element as null
			uint8_t* iStrings[12]{ 0,0,0,0,0,0,0,0,0,0,0,0 };

			//stack of string index numbers from the descriptors
			//   This is loaded as we parse the descriptors ... and then 
			//   requested one by one from the device ...
			uint8_t topOfstackOfstringsToProcess = 0;
			uint8_t stackOfstringsToProcess[sizeof(iStrings) / sizeof(iStrings[0])];

		// ---- iString Data from Device Descriptors END -----------

		// ---- Summary of Device Descriptors for External Viewing -----------
			Defs::EndpointJackMap_t InCables[16]{
					[0] = {							//All 16 are zero initialized 
						.embeddedJackID = 0,        //... just pointing out struct
						.embeddedJackiString = 0,
						.externalJackID = 0,
						.externalJackiString = 0
						}
			};
			Defs::EndpointJackMap_t OutCables[16]{
					[0] = {
						.embeddedJackID = 0,
						.embeddedJackiString = 0,
						.externalJackID = 0,
						.externalJackiString = 0
						}
			};
			Defs::DeviceInfoSummary_t deviceInfoSummary{
				// Device Stuff
				 .USBvendorID = 0,
				 .USBMajorVersion = 0,
				 .USBMinorVersion = 0,
				 .USBMinorSubVersion = 0,
				 .manufacturerStringIndex = 0,
				 .productStringIndex = 0,
				 .serialNumStringIndex = 0,
				 .deviceMajorVersion = 0,
				 .deviceMinorVersion = 0,
				 .deviceMinorSubVersion = 0,
				 .numberOfConfigurations = 0,
				 // Config Stuff
				  .numberOfInterfaces = 0,
				  .configurationStringIndex = 0,
				  .selfPowered = false,
				  .remoteWakeup = false,
				  .maxBusPower_mA = 0,
				  // Midi Streaming
				   .interfaceStringIndex = 0,
				   .midiMajorVersion = 0,
				   .midiMinorVersion = 0,
				   .midiMinorSubVersion = 0,
				   //MS Endpoint
					.endpointInID = 0,
					.numberOfCablesIn = 0,
					.jackMapIn = InCables,		//see struct above
					.endpointOutID = 0,
					.numberOfCablesOut = 0,
					.jackMapOut = OutCables,	//see struct above
					//Strings
					 .iStrings = iStrings		//see array above
			};
		// ---- Summary of Device Descriptors for External Viewing END -----------

		bool deviceReady = false;
	//---------------  USBMidiDriver    -------  DATA STOARGE END ------------


		void init() {

			deviceReady = false;

			// Clear buffers
			memset(deviceDescriptorBuff, 0, sizeof(deviceDescriptorBuff));
			memset(configDescriptorBuff, 0, sizeof(configDescriptorBuff));
			memset(stringDescriptorBuff, 0, sizeof(stringDescriptorBuff));
			memset(iSB, 0, sizeof(iSB));

			// parsed descriptor info
			uint8_t length = sizeof(descriptorArray) / sizeof(descriptorArray[0]);
			for (size_t i = 0; i < length; i++) { descriptorArray[i] = { nullptr, Defs::USBDescriptorStructType::Unknown }; }
			nextArrayIndex = 0;


			// iString storage
			length = sizeof(iStrings) / sizeof(iStrings[0]);
			for (size_t i = 0; i < length; i++) {
				iStrings[i] = &(iSB[0]); // point to 1st spot
				stackOfstringsToProcess[i] = 0;
			}

			// Summary 
			length = sizeof(InCables) / sizeof(InCables[0]);
			for (size_t i = 0; i < length; i++) {
				InCables[i] = { .embeddedJackID = 0, .embeddedJackiString = 0, .externalJackID = 0, .externalJackiString = 0 };
				OutCables[i] = { .embeddedJackID = 0, .embeddedJackiString = 0, .externalJackID = 0, .externalJackiString = 0 };
			}

			deviceInfoSummary = {
				// Device Stuff
					.USBvendorID = 0,
					.USBMajorVersion = 0,
					.USBMinorVersion = 0,
					.USBMinorSubVersion = 0,
					.manufacturerStringIndex = 0,
					.productStringIndex = 0,
					.serialNumStringIndex = 0,
					.deviceMajorVersion = 0,
					.deviceMinorVersion = 0,
					.deviceMinorSubVersion = 0,
					.numberOfConfigurations = 0,
					// Config Stuff
					.numberOfInterfaces = 0,
					.configurationStringIndex = 0,
					.selfPowered = false,
					.remoteWakeup = false,
					.maxBusPower_mA = 0,
					// Midi Streaming
					.interfaceStringIndex = 0,
					.midiMajorVersion = 0,
					.midiMinorVersion = 0,
					.midiMinorSubVersion = 0,
					//MS Endpoint
					.endpointInID = 0,
					.numberOfCablesIn = 0,
					.jackMapIn = InCables,		//see struct above
					.endpointOutID = 0,
					.numberOfCablesOut = 0,
					.jackMapOut = OutCables,	//see struct above
					//Strings
						.iStrings = iStrings		//see array above
			};
		}

		void dumpAllDescriptors() { dumpDescriptors(false); }
		void dumpDescriptorsByType(Defs::USBDescriptorStructType tp) { dumpDescriptors(true, tp); }
		void dumpDescriptors(bool byType, Defs::USBDescriptorStructType tp = Defs::USBDescriptorStructType::Unknown) const {

			if (!(nextArrayIndex)) {
				xERR::println("ERROR - Descriptors are not available");
				return;
			}
			
			void* dPtr;

			for (uint8_t i = 0; i < nextArrayIndex; i++) {
				dPtr = descriptorArray[i].dataPtr;

				if (byType && descriptorArray[i].type != tp) continue;

				switch (descriptorArray[i].type) {
				case Defs::USBDescriptorStructType::Unknown: xDBG::print((USBDefs::UnknownStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::Config: xDBG::print((USBDefs::ConfigStruct*)dPtr,(uint8_t* const*)iStrings);  break;
				case Defs::USBDescriptorStructType::Interface: xDBG::print((USBDefs::InterfaceStruct*)dPtr, (uint8_t* const*)iStrings); break;
				case Defs::USBDescriptorStructType::Endpoint: xDBG::print((USBDefs::EndPointStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::AC_Interface_Header: xDBG::print((USBDefs::AC_InterfaceHeaderStruct*)dPtr);  break;
				case Defs::USBDescriptorStructType::MS_Interface_Header: xDBG::print((USBDefs::MS_InterfaceHeaderStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::MS_Interface_InJack: xDBG::print((USBDefs::MS_InterfaceInJackStruct*)dPtr, (uint8_t* const*)iStrings); break;
				case Defs::USBDescriptorStructType::MS_Interface_OutJack: xDBG::print((USBDefs::MS_InterfaceOutJackStruct*)dPtr, (uint8_t* const*)iStrings); break;
				case Defs::USBDescriptorStructType::MS_Interface_Element: xDBG::print((USBDefs::MS_InterfaceElementStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::MS_Endpoint_In: xDBG::print((USBDefs::MS_EndpointGeneralStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::MS_Endpoint_Out: xDBG::print((USBDefs::MS_EndpointGeneralStruct*)dPtr); break;
				case Defs::USBDescriptorStructType::Device: xDBG::print((USBDefs::DeviceStruct*)dPtr, (uint8_t* const*)iStrings); break;


				}
			}


		}// end function dumpDescriptors
		
		Defs::DeviceInfoSummary_t* getDeviceSummaryInfo() {
			if (deviceReady) {
				if (deviceInfoSummary.USBvendorID == 0) { buildDeviceSummaryInfo(); }
				return &deviceInfoSummary;
			}
			return nullptr;
		}
		void buildDeviceSummaryInfo() {
			memset(&deviceInfoSummary, 0, sizeof(deviceInfoSummary));
			void* dPtr;

			// This logic assumes there is only 1 of each of these descriptors ... or we get that last one ;-)
			for (uint8_t i = 0; i < nextArrayIndex; i++) {
				dPtr = descriptorArray[i].dataPtr;

				switch (descriptorArray[i].type) {
				case Defs::USBDescriptorStructType::Device: {
					USBDefs::DeviceStruct* data = (USBDefs::DeviceStruct*)dPtr;

					deviceInfoSummary.USBvendorID = data->idVendorMSB << 8 | data->idVendorLSB;
					deviceInfoSummary.USBMajorVersion = data->bcdUSBMSB;
					deviceInfoSummary.USBMinorVersion = data->bcdUSBLSB  >> 4;
					deviceInfoSummary.USBMinorSubVersion = data->bcdUSBLSB & 0b00001111;
					deviceInfoSummary.manufacturerStringIndex = data->iManufacturer ;
					deviceInfoSummary.productStringIndex = data->iProduct ;
					deviceInfoSummary.serialNumStringIndex = data->iSerialNumber ;
					deviceInfoSummary.deviceMajorVersion = data->bcdDeviceMSB ;
					deviceInfoSummary.deviceMinorVersion = data->bcdDeviceLSB >> 4;
					deviceInfoSummary.deviceMinorSubVersion = data->bcdDeviceLSB & 0b00001111;
					deviceInfoSummary.numberOfConfigurations = data->bNumConfigurations ;

					break;
				}
				case Defs::USBDescriptorStructType::Config: {
					USBDefs::ConfigStruct* data = (USBDefs::ConfigStruct*)dPtr;

					deviceInfoSummary.numberOfInterfaces = data->bNumInterfaces;
					deviceInfoSummary.configurationStringIndex = data->iConfiguration;
					deviceInfoSummary.maxBusPower_mA = data->bMaxPower *2;
					deviceInfoSummary.selfPowered =  (data->bmAttributes & 0b01000000);
					deviceInfoSummary.remoteWakeup = (data->bmAttributes & 0b00100000);

					break;
				}
				case Defs::USBDescriptorStructType::Interface: {
					USBDefs::InterfaceStruct* data = (USBDefs::InterfaceStruct*)dPtr;

					// Only want the midiStreaming StringIndex ... not AudioControl
					if (data->bInterfaceSubClass == USBDefs::AUDIO_INTERFACE_SUBCLASS::MIDISTREAMING) {
						deviceInfoSummary.interfaceStringIndex = data->iInterface;
					}
					break;
				}
				case Defs::USBDescriptorStructType::MS_Interface_Header: {
					USBDefs::MS_InterfaceHeaderStruct* data = (USBDefs::MS_InterfaceHeaderStruct*)dPtr;

					deviceInfoSummary.midiMajorVersion = data->bcdMidiSpecMSB;
					deviceInfoSummary.midiMinorVersion = data->bcdMidiSpecLSB >> 4;
					deviceInfoSummary.midiMinorSubVersion = data->bcdMidiSpecLSB & 0b00001111;

					break;

				}
				case Defs::USBDescriptorStructType::MS_Endpoint_In: {
					USBDefs::MS_EndpointGeneralStruct* data = (USBDefs::MS_EndpointGeneralStruct*)dPtr;
					deviceInfoSummary.numberOfCablesIn = data->bNumEmbMIDJack;
					break;
				}
				case Defs::USBDescriptorStructType::MS_Endpoint_Out: {
					USBDefs::MS_EndpointGeneralStruct* data = (USBDefs::MS_EndpointGeneralStruct*)dPtr;
					deviceInfoSummary.numberOfCablesOut = data->bNumEmbMIDJack;
					break;
				}
				default:
					break; 

				}
			}
			// Build/Rebuild the cables since summary struct includes them
			buildCables();
			deviceInfoSummary.jackMapIn = InCables;
			deviceInfoSummary.jackMapOut = OutCables;
			deviceInfoSummary.iStrings = iStrings;
		} // End function buildDeviceSummaryInfo
		void buildCables() {

			// We finding these 4 values for each InCables[x] and OutCables[x]
			//   ... we are also getting the Endpoint IDs 
			// 
			//typedef struct EndpointJackMap_struct {
			//	uint8_t embeddedJackID;
			//	uint8_t embeddedJackiString;
			//	uint8_t externalJackID;
			//	uint8_t externalJackiString;

			memset(OutCables, 0, sizeof(OutCables));
			memset(InCables, 0, sizeof(InCables));

			void* dPtr;
			USBDefs::InterfaceStruct* iD = nullptr;

			// Some assumptions ... going to bail if not 2 endpoints in the MS interface 1 in 1 out
			// loop 1 of 3 over the descriptorArray ....
			for (uint8_t i = 0; i < nextArrayIndex; i++) {
				dPtr = descriptorArray[i].dataPtr;

				switch (descriptorArray[i].type) {

				// Save to show we hit a midi streaming interface 
				case Defs::USBDescriptorStructType::Interface: {
					USBDefs::InterfaceStruct* data = (USBDefs::InterfaceStruct*)dPtr;
					iD = data; // Save for childern

					// Only want the midiStreaming endpoints ... not AudioControl
					if (data->bInterfaceSubClass == USBDefs::AUDIO_INTERFACE_SUBCLASS::MIDISTREAMING) {
						if (data->bNumEndpoints != 2) {
							xERR::println("ERROR - Cable Info logic assumes 2 endpoints ");
							return ;
						}
					}
					break;
				}
				// When we find an endpoint under a midi streaming interface, save it's ID
				case Defs::USBDescriptorStructType::Endpoint: {
					USBDefs::EndPointStruct* data = (USBDefs::EndPointStruct*)dPtr;

					//Parent must be midi streaming
					if (iD->bInterfaceSubClass == USBDefs::AUDIO_INTERFACE_SUBCLASS::MIDISTREAMING) {
						// MMBit: 1 = in, 0 = out ... ID is in bottom 4 bits
						if (data->bEndpointAddress & 0b10000000) { 
							deviceInfoSummary.endpointInID = (data->bEndpointAddress & 0b00001111);
						}
						else { 
							deviceInfoSummary.endpointOutID = (data->bEndpointAddress & 0b00001111);
						}
					}
					break; 
				}

				//  save ..... InCables[x].embeddedJackIDs
				case Defs::USBDescriptorStructType::MS_Endpoint_In: {
					USBDefs::MS_EndpointGeneralStruct* data = (USBDefs::MS_EndpointGeneralStruct*)dPtr;

					// Loop over # cables In and save baAssocJackID1 to inCables->embeddedJackID
					uint8_t sz = deviceInfoSummary.numberOfCablesIn; //= data->bNumEmbMIDJack;
					uint8_t* pJackID = & data->baAssocJackID1;
					for (uint8_t i = 0; i < sz; i++) {
						InCables[i].embeddedJackID = *(pJackID + i);
					}
					break;
				}
 			    //  save ..... OutCables[x].embeddedJackIDs
				case Defs::USBDescriptorStructType::MS_Endpoint_Out: {
					USBDefs::MS_EndpointGeneralStruct* data = (USBDefs::MS_EndpointGeneralStruct*)dPtr;

					// Loop over # cables Out and save baAssocJackID1 to outCables->embeddedJackID
					uint8_t sz = deviceInfoSummary.numberOfCablesOut; //= data->bNumEmbMIDJack;
					uint8_t* pJackID = &data->baAssocJackID1;
					for (uint8_t i = 0; i < sz; i++) {
						OutCables[i].embeddedJackID = *(pJackID + i);
					}
					break;
				}
				default:
					break;

				}// End Case

			} // End loop 1

			// loop 2 ... process MS_Interface_OutJacks ... for links from In to Out
			for (uint8_t i = 0; i < nextArrayIndex; i++) {
				
				if (descriptorArray[i].type == Defs::USBDescriptorStructType::MS_Interface_OutJack) {
					USBDefs::MS_InterfaceOutJackStruct* data =
						(USBDefs::MS_InterfaceOutJackStruct*)descriptorArray[i].dataPtr;;
					if (data->bJackType == USBDefs::MIDISTREAMING_INOUTJACK_TYPE::EXTERNAL)
					{
						// save ... OutCables[x].externalJackID
						// save ... OutCables[x].externalJackiString
						// NOTE: Only save 1st pin connection ...
						uint8_t sz = deviceInfoSummary.numberOfCablesOut;
						for (uint8_t i = 0; i < sz; i++) {
							if (OutCables[i].embeddedJackID == data->baSourceID1) {
								OutCables[i].externalJackID = data->bJackID;

								//cast the ms descriptor struct as a byte array ... and string index is last byte
								uint8_t* bytePtr = (uint8_t*)data;
								OutCables[i].externalJackiString = bytePtr[(data->header.bLength) - 1];;
								break; // should only find 1 match ....
							}
						}
					} // end if external

					if (data->bJackType == USBDefs::MIDISTREAMING_INOUTJACK_TYPE::EMBEDDED)
					{
						// save ... InCables[x].externalJackID
						// save ... InCables[x].embeddedJackiString
						// NOTE: Only save 1st pin connection ...
						uint8_t sz = deviceInfoSummary.numberOfCablesIn;
						for (uint8_t i = 0; i < sz; i++) {
							if (InCables[i].embeddedJackID == data->bJackID) {
								InCables[i].externalJackID = data->baSourceID1;

								//cast the ms descriptor struct as a byte array ... and string index is last byte
								uint8_t* bytePtr = (uint8_t*)data;
								InCables[i].embeddedJackiString = bytePtr[(data->header.bLength) - 1];;
								break; // should only find 1 match ....
							}
						}
					} // end if embedded
				} // end if MS_Interface_OutJack
			} // End of loop 2

			// loop 3 ... process MS_Interface_InJacks ... for iStrings
			for (uint8_t i = 0; i < nextArrayIndex; i++) {

				if (descriptorArray[i].type == Defs::USBDescriptorStructType::MS_Interface_InJack) {
					USBDefs::MS_InterfaceInJackStruct* data =
						(USBDefs::MS_InterfaceInJackStruct*)descriptorArray[i].dataPtr;;
					if (data->bJackType == USBDefs::MIDISTREAMING_INOUTJACK_TYPE::EMBEDDED)
					{
						// save ... OutCables[x].embeddedJackiString
							// NOTE: Only save 1st pin connection ...
						uint8_t sz = deviceInfoSummary.numberOfCablesOut;
						for (uint8_t i = 0; i < sz; i++) {
							if (OutCables[i].embeddedJackID == data->bJackID) {
								OutCables[i].embeddedJackiString = data->iJack;
								break; // should only find 1 match ....
							}
						}
					} // end if Embedded

					if (data->bJackType == USBDefs::MIDISTREAMING_INOUTJACK_TYPE::EXTERNAL)
					{
						// save ... InCables[x].embeddedJackiString
						// NOTE: Only save 1st pin connection ...
						uint8_t sz = deviceInfoSummary.numberOfCablesIn;
						for (uint8_t i = 0; i < sz; i++) {
							if (InCables[i].externalJackID == data->bJackID) {
								InCables[i].externalJackiString = data->iJack;
								break; // should only find 1 match ....
							}
						}
					} // end if external
				} // end if MS_Interface_InJack
			}// End of loop 3

		} // END Function


		friend class USBMidiDriver;
		friend class DescriptorHelper;
	 }; // end class USBMidiDriverData

};

extern DataStorage dataStorage;


