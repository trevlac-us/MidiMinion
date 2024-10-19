
#include "USBMidiDriver.h"
#include "DevicesInterface.h"
#include "DBG.h"


//---------------  USBMidiDriver    CLASS START ------------------

//**********  Functions working with Parent class ***********
bool USBMidiDriver::claim(Device_t* dev, int type, const uint8_t* descriptors, uint32_t len) {
	
	// If the base driver claimed the device <returns true>, it's a midi interface
	// We want to then kick off the process of gathering info on the device
	// queue a request for the device descriptor ... which will callback our control() func.
	if ( MIDIDevice::claim(dev, type, descriptors, len) ) {
		requestDeviceDescriptor(dev);
		return true;
	}
	return false;
}
void USBMidiDriver::disconnect() { fireEvent(Defs::DeviceEventType::DeviceDisconnect); }
void USBMidiDriver::control(const Transfer_t* transfer) {
	//This is a callback function from making a USBHost::queue_Control_Transfer()

	//   If all goes well we get here by asking for: device-->config 9 bytes-->config full-->string1-->stringN
	//   So ... we need to check the last request and then setup the next ...

	// --- Given Last type ... setup next
	switch (findControlCallbackType(transfer)) {

	case USBMidiDriver::DEVICE: {
		requestConfigDescriptor(transfer->pipe->device, USBDefs::DESCRIPTOR_LENGTH::CONFIG);
		return;
	}

	case USBMidiDriver::CONFIG_HEADER: {
		const USBDefs::ConfigStruct* configDescriptor = (USBDefs::ConfigStruct*)(&(myData->configDescriptorBuff[0]));
		uint16_t configTotalLength = configDescriptor->bTotalLengthMSB << 8 | configDescriptor->bTotalLengthLSB;
		const uint16_t buffSz = sizeof(myData->configDescriptorBuff) / sizeof(myData->configDescriptorBuff[0]);

		// only ask for max our buffer size
		if (configTotalLength > buffSz) { configTotalLength = buffSz; }

		if (configTotalLength <= configDescriptor->header.bLength) {
			xERR::println("ERROR Config has no interface.  TotalLength=", configTotalLength);
			return;
		}

		//  request and wait for callback with FULL data
		requestConfigDescriptor(transfer->pipe->device, configTotalLength);
		return;
	}

	case USBMidiDriver::CONFIG_FULL: {
		DescriptorHelper::parseConfigData(myData);
		DescriptorHelper::buildStackOfStringsToProcess(myData);

		if (!(moreStringsToRequest(transfer->pipe->device))) { fireEvent(Defs::DeviceEventType::DeviceConnect); }

		return;
	}

	case USBMidiDriver::STRING: {
		USBDefs::StringStruct* stringDescriptor = (USBDefs::StringStruct*)(&(myData->stringDescriptorBuff[0]));
		const uint8_t requestedStringIndex = transfer->setup.wValue & 0x00FF;  //This is the string index we requested

		if (stringDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::STRING) {
			xERR::println("ERROR - String Descriptor type is not correct.  Should be 3 is=", stringDescriptor->header.bDescriptorType);
			//return; << weird error (didn't get type we requested) but allow next request?
		}
		else {
			DescriptorHelper::saveStringDescriptor(myData, requestedStringIndex);
		}

		if (!(moreStringsToRequest(transfer->pipe->device))) { fireEvent(Defs::DeviceEventType::DeviceConnect); }

		return;
	}

	case USBMidiDriver::ERROR:
	default:
		xERR::println("ERROR Unknown or Error Control Transfer");
	}
	return;
}



//**********  Private Worker Functions ***********
void USBMidiDriver::requestDeviceDescriptor(Device_t* dev) {

	//Make the setup packet and request the device descriptor 
	//Note: queue_Control_Transfer( ...,...,..., this) causes the host to call our control()

		// 1 = Device to Host, 00 = Standard Request Type, 00000 = Device is recipient
	const uint8_t bmRequestType = 0b10000000; // b7[1] b6-5[00] b4-0[00000]
	const uint8_t bRequest = 6; // 6 = GET_DESCRIPTOR
	const uint16_t wValue = (uint8_t)USBDefs::USB_DESCRIPTOR_TYPE::DEVICE << 8; // HiByte [0x01] = Descriptor Type (Device), LowByte[0x00] = Index (NA)
	const uint16_t wIndex = 0; // Not used for Device Descriptor request
	const uint16_t wLength = USBDefs::DESCRIPTOR_LENGTH::DEVICE; // Device Descriptor are 18 bytes long ... ask for all 18

	USBHost::mk_setup(USBsetupPacket, bmRequestType, bRequest, wValue, wIndex, wLength);
	if (!(USBHost::queue_Control_Transfer(dev, &USBsetupPacket, &(myData->deviceDescriptorBuff), this))) {
		xERR::println("Device queue_Control_Transfer returned FALSE");
	}

}
void USBMidiDriver::requestConfigDescriptor(Device_t* dev, uint16_t lengthToRequest) {

	// Make the setup packet to request config descriptor

	// 1 = Device to Host, 00 = Standard Request Type, 00000 = Device is recipient
	const uint8_t bmRequestType = 0b10000000; // b7[1] b6-5[00] b4-0[00000]
	const uint8_t bRequest = 6; // 6 = GET_DESCRIPTOR
	// HighByte [0x02] = Descriptor Type (Config), LowByte[0x00] = ConfigIndex 1st=0.  Do devices have >1 config?
	const uint16_t wValue = (uint8_t)USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION << 8;
	const uint16_t wIndex = 0; // Not used for Config Descriptor request
	const uint16_t wLength = lengthToRequest; // Config Descriptors are 9 bytes long ... 

	// Request the config descriptor
	USBHost::mk_setup(USBsetupPacket, bmRequestType, bRequest, wValue, wIndex, wLength);
	if (!(USBHost::queue_Control_Transfer(dev, &USBsetupPacket, &(myData->configDescriptorBuff), this))) {
		xERR::println("Config queue_Control_Transfer returned FALSE");
	}

}
void USBMidiDriver::requestStringDescriptor(Device_t* dev, uint8_t stringIndex) {

	// 1 = Device to Host, 00 = Standard Request Type, 00000 = Device is recipient
	const uint8_t bmRequestType = 0b10000000; // b7[1] b6-5[00] b4-0[00000]
	const uint8_t bRequest = 6; // 6 = GET_DESCRIPTOR
	// HiByte [0x03] = Descriptor Type (String), LowByte[0x0?] = Index of String
	const uint16_t wValue = (uint8_t)USBDefs::USB_DESCRIPTOR_TYPE::STRING << 8 | stringIndex;
	const uint16_t wIndex = dev->LanguageID; // USBHost got default language for us
	const uint16_t wLength = USBDefs::DESCRIPTOR_LENGTH::ISTRING_MAX; // Max a string can be

	USBHost::mk_setup(USBsetupPacket, bmRequestType, bRequest, wValue, wIndex, wLength);
	if (!(USBHost::queue_Control_Transfer(dev, &USBsetupPacket, &(myData->stringDescriptorBuff), this))) {
		xERR::println("String queue_Control_Transfer returned FALSE");
	}

}
USBMidiDriver::controlCallbackType USBMidiDriver::findControlCallbackType(const Transfer_t* transfer) {

	//Figures out what request we made ... to process the response
	USBDefs::USB_DESCRIPTOR_TYPE requestedDescriptorType = (USBDefs::USB_DESCRIPTOR_TYPE)(transfer->setup.wValue >> 8);
	switch (requestedDescriptorType) {

		case USBDefs::USB_DESCRIPTOR_TYPE::DEVICE:
		{
			USBDefs::DeviceStruct* deviceDescriptor = (USBDefs::DeviceStruct*)(&(myData->deviceDescriptorBuff[0]));

			if (deviceDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::DEVICE) {
				xERR::println("ERROR - Device Descriptor type is not correct.  Should be 1 is=", deviceDescriptor->header.bDescriptorType);
				return controlCallbackType::ERROR;
			}
			return controlCallbackType::DEVICE;;
		} // end case device descriptor

		case USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION: {
			const USBDefs::ConfigStruct* configDescriptor = (USBDefs::ConfigStruct*)(&(myData->configDescriptorBuff[0]));

			if (configDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION) {
				xERR::println("ERROR - Config Descriptor type is not correct.  Should be 2 is=", configDescriptor->header.bDescriptorType);
				xERR::print_hexbytes(&(myData->configDescriptorBuff[0]), 9);
				return controlCallbackType::ERROR;
			}

			// We request config 2x.  1st just config data (fixed length of 9) to get totallength value. 2nd totalLength of data
			// Figure out based on lengths requested ... how to process the config request
			const uint16_t requestedLength = transfer->setup.wLength;

			if (requestedLength == configDescriptor->header.bLength) { return controlCallbackType::CONFIG_HEADER; }
			if (requestedLength > configDescriptor->header.bLength) {
				return controlCallbackType::CONFIG_FULL;
			}
			else {
				xERR::println("ERROR - Something is wrong with Config request. Length requested was: ", requestedLength);
				return controlCallbackType::ERROR;
			}
		}

		case USBDefs::USB_DESCRIPTOR_TYPE::STRING: { return controlCallbackType::STRING;} 

		default: return controlCallbackType::ERROR;
	} 
}
bool USBMidiDriver::moreStringsToRequest(Device_t* dev) {

	if (myData->topOfstackOfstringsToProcess) {
		requestStringDescriptor(dev, myData->stackOfstringsToProcess[myData->topOfstackOfstringsToProcess--]);
		return true;
	}
	return false;
}
void USBMidiDriver::fireEvent(Defs::DeviceEventType event) {
	switch (event) {
	case Defs::DeviceEventType::DeviceConnect: { myData->deviceReady = true;	break; }
	case Defs::DeviceEventType::DeviceDisconnect: { MIDIDevice::disconnect(); init(); break; }
	default: {}
	}
	DeviceManager::USBMidiDriverNotifications(this, event);
}

//   -------- OLD VERSION of control -----------
//	Device_t* dev = transfer->pipe->device;
//	USBDefs::USB_DESCRIPTOR_TYPE requestedDescriptorType = (USBDefs::USB_DESCRIPTOR_TYPE) (transfer->setup.wValue >>8);
//	const uint16_t requestedLength = transfer->setup.wLength;
//
//	//The switch figures out what request we made ... to process the response
//	switch (requestedDescriptorType) {
//
//		case USBDefs::USB_DESCRIPTOR_TYPE::DEVICE:
//	{
//			USBDefs::DeviceStruct* deviceDescriptor = (USBDefs::DeviceStruct*)(&deviceDescriptorBuff[0]);
//			
//			if (deviceDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::DEVICE) {
//				xERR::println("ERROR - Device Descriptor type is not correct.  Should be 1 is=", deviceDescriptor->header.bDescriptorType);
//				return;
//			}
//
//			//  request and wait for callback with CONFIG descriptor
//			requestConfigDescriptor(dev);
//			return; 
//
//		} // end case device descriptor
//
//
//		case USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION: {
//			const USBDefs::ConfigStruct* configDescriptor = (USBDefs::ConfigStruct *) (&configDescriptorBuff[0]);
//
//			if (configDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION) {
//				xERR::println("ERROR - Config Descriptor type is not correct.  Should be 2 is=", configDescriptor->header.bDescriptorType);
//				xERR::print_hexbytes(&configDescriptorBuff[0], 9);
//				return;
//			}
//
//			// We request config 2x.  1st just config data (fixed length of 9) to get totallength value. 2nd totalLength of data
//			// Figure out based on lengths requested ... how to process the config request
//			enum configRequestType : uint8_t { JUSTDescriptor, AllConfigData, Error};
//			configRequestType configLengthRequested = configRequestType::Error;
//
//			if (requestedLength == configDescriptor->header.bLength) configLengthRequested = configRequestType::JUSTDescriptor;
//			if (requestedLength > configDescriptor->header.bLength) configLengthRequested = configRequestType::AllConfigData;
//
//			switch(configLengthRequested) {
//			
//				case configRequestType::JUSTDescriptor: {
//					// Make the setup packet to request the full configuration ... we only have the 1st 9 bytes so far
//
//					// Going to get the full configuration ... or as much as fits in our buffer
//					uint16_t configTotalLength = configDescriptor->bTotalLengthMSB << 8 | configDescriptor->bTotalLengthLSB;
//					const uint16_t buffLeft = sizeof(configDescriptorBuff);
//					if (configTotalLength > buffLeft) { configTotalLength = buffLeft; }
//
//					if (configTotalLength <= configDescriptor->header.bLength) {
//						xERR::println("ERROR Config has no interface.  TotalLength=", configTotalLength);
//						return;
//					}
//
//					//  request and wait for callback with FULL data
//					requestConfigData(dev, configTotalLength);
//					return; 
//				} // end case we requested just the config descriptor
//
//				case configRequestType::AllConfigData: {
//					// Full data is now in the buffer ... so parse it
//					descriptorHelper.parseConfigData();
//
//					// Start Process of requesting the string descriptors ...
//					uint8_t stringIndex = descriptorHelper.nextStringToRequest();
//					if (stringIndex == 0) {
//						setDeviceReady();
//					}
//					else {
//						requestStringDescriptor(dev, stringIndex);
//					}
//
//					return; 
//				} // end case all config data
//
//				default: {
//					xERR::println("ERROR - Something is wrong with Config request. Length requested was: ", requestedLength);
//					return;
//				}
//
//			} // end switch configLengthRequested
//		} // end case ... config requests
//
//
//		case USBDefs::USB_DESCRIPTOR_TYPE::STRING:
//		{
//			USBDefs::StringStruct* stringDescriptor = (USBDefs::StringStruct*)(&stringDescriptorBuff[0]);
//			const uint8_t requestedStringIndex = transfer->setup.wValue & 0x00FF;  //This is the string index we requested
//
//
//			if (stringDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::STRING) {
//				xERR::println("ERROR - String Descriptor type is not correct.  Should be 3 is=", stringDescriptor->header.bDescriptorType);
//				//return; << weird error (didn't get type we requested) but allow next request?
//			}
//			else {
//				descriptorHelper.saveStringDescriptor(requestedStringIndex);
//			}
//
//			//  request next string and wait for callback with next descriptor
//			uint8_t nextStringIndex = descriptorHelper.nextStringToRequest();
//			if (nextStringIndex == 0) {
//				setDeviceReady();
//			}
//			else {
//				requestStringDescriptor(dev, nextStringIndex);
//			}
//
//			return;
//
//		} // end case string descriptor
//
//
//		default: // Error we got called on a request type we don't handle ?
//			return;
//	} // end switch on requested type
//}


//---------------  USBMidiDriver    CLASS END ------------------



//---------------  DescriptorHelper    -------  CLASS START ------------

void DescriptorHelper::parseConfigData(DataStorage::USBMidiDriverData* data) {

	uint8_t* ddPtr = &(data->deviceDescriptorBuff[0]);
	uint8_t* cdPtr = &(data->configDescriptorBuff[0]);
	xDBG::println("ConfigParse - Start");


	// Check the device buffer has the right type
	const USBDefs::DeviceStruct* deviceDescriptor = (USBDefs::DeviceStruct*)(ddPtr);
	if (deviceDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::DEVICE) {
		xERR::println("PARSE ERROR - Device Descriptor type is not correct.  Should be 1 is=", deviceDescriptor->header.bDescriptorType);
		return;
	}

	// Check the config has a good type
	USBDefs::ConfigStruct* configDescriptor = (USBDefs::ConfigStruct*)(cdPtr);
	if (configDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION) {
		xERR::println("PARSE ERROR - Config Descriptor type is not correct.  Should be 2 is=", configDescriptor->header.bDescriptorType);
		xERR::print_hexbytes(cdPtr, 9);
		return;
	}

	//Clear everthing out if we are going to parse
	data->init();

	//No need to parse the deviceBuffer
	data->descriptorArray[0].dataPtr = ddPtr;
	data->descriptorArray[0].type = Defs::USBDescriptorStructType::Device;
	data->nextArrayIndex++;


	// Limit how far we are going to go ... And find the descriptors in the config buffer
	const uint16_t configTotalLength = configDescriptor->bTotalLengthMSB << 8 | configDescriptor->bTotalLengthLSB;
	const uint16_t bufSize = min(sizeof(data->configDescriptorBuff)/ sizeof(data->configDescriptorBuff[0]), configTotalLength);
	const uint8_t arraySize = sizeof(data->descriptorArray)/sizeof(data->descriptorArray[0]);

	uint16_t offset = 0;
	USBDefs::UnknownStruct* ukD;
	USBDefs::InterfaceStruct* iD = nullptr;
	USBDefs::EndPointStruct* epD = nullptr;


	while ((offset < bufSize) && (data->nextArrayIndex < arraySize))
	{
		xDBG::println("ConfigParse - Descriptor#=", data->nextArrayIndex);

		//Confirm full size is in the buffer ...
		ukD = (USBDefs::UnknownStruct*)&(cdPtr[offset]);
		if ((offset + ukD->header.bLength) > bufSize) { break; }

		//Save the start of the descriptor in the array & type
		data->descriptorArray[data->nextArrayIndex].dataPtr = ukD;
		data->descriptorArray[data->nextArrayIndex].type = DescriptorHelper::getDescriptorStructType(ukD, iD, epD);

		//Save the last interface pointer as needed for child types
		if (data->descriptorArray[data->nextArrayIndex].type == Defs::USBDescriptorStructType::Interface) {
			iD = (USBDefs::InterfaceStruct*)data->descriptorArray[data->nextArrayIndex].dataPtr;
		}
		//Save the last endpoint pointer as needed for child types
		if (data->descriptorArray[data->nextArrayIndex].type == Defs::USBDescriptorStructType::Endpoint) {
			epD = (USBDefs::EndPointStruct*)data->descriptorArray[data->nextArrayIndex].dataPtr;
		}


		offset = offset + ukD->header.bLength; // move ahead to start of next descriptor
		data->nextArrayIndex++;

		if (data->nextArrayIndex >= arraySize) {
			xERR::println("PARSE ERROR - # descriptors > we can process = ", data->nextArrayIndex);
			break; // out of loop but still keep going ...
		}

	} // end loop



} // end function
Defs::USBDescriptorStructType DescriptorHelper::getDescriptorStructType(USBDefs::UnknownStruct* ukD
	, USBDefs::InterfaceStruct* iD, USBDefs::EndPointStruct* epD)  {

	if (ukD == nullptr) {
			xERR::println("ERROR - ukD == Null Pointer getDescriptorStructType ");
			return Defs::USBDescriptorStructType::Unknown;
	}
	
	xDBG::println("DescriptorTypeCode = ", ukD->header.bDescriptorType);
	//figure out the type of data structure
	switch (ukD->header.bDescriptorType) {
	
		//Standard USB cases ...
		case USBDefs::USB_DESCRIPTOR_TYPE::DEVICE: { return Defs::USBDescriptorStructType::Device; }
		case USBDefs::USB_DESCRIPTOR_TYPE::CONFIGURATION: { return Defs::USBDescriptorStructType::Config; }
		case USBDefs::USB_DESCRIPTOR_TYPE::INTERFACE: {
			iD = (USBDefs::InterfaceStruct*)ukD; //Save this for childern ...
			return Defs::USBDescriptorStructType::Interface;
		}
		case USBDefs::USB_DESCRIPTOR_TYPE::ENDPOINT: {
			epD = (USBDefs::EndPointStruct*)ukD; //Save this for class childern ...
			return Defs::USBDescriptorStructType::Endpoint;
		}
		//Class Specific ENDPOINT for Audio ...
		case USBDefs::USB_DESCRIPTOR_TYPE::AUDIOCLASS_ENDPOINT: {
	
			if (iD == nullptr) {
				xERR::println("ERROR - iD == Null Pointer getDescriptorStructType ");
				return Defs::USBDescriptorStructType::Unknown;
			}
			// Make sure iD is populated with an inteface ...
			if (iD->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::INTERFACE) {
				xERR::println("ERROR - Parent Interface is not valid ", iD->header.bDescriptorType);
				return Defs::USBDescriptorStructType::Unknown;
			}
	
			// we know it's sub class from the parent interface
			switch (iD->bInterfaceSubClass) {
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::MIDISTREAMING: {
					if (((USBDefs::MS_EndpointGeneralStruct*)ukD)->bDescriptorSubtype ==
						USBDefs::MS_EP_DESCRIPTOR_SUBTYPE::GENERAL) {
	
						if (epD == nullptr) {
							xERR::println("ERROR - epD == Null Pointer getDescriptorStructType ");
							return Defs::USBDescriptorStructType::Unknown;
						}
						// Make sure iD is populated with an endpoint ...
						if (epD->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::ENDPOINT) {
							xERR::println("ERROR - Parent Endpoint is not valid ", epD->header.bDescriptorType);
							return Defs::USBDescriptorStructType::Unknown;
						}
						//Parent Endpoint Address bit D7: Direction: 0 = OUT endpoint, 1 = IN endpoint
						if (((epD->bEndpointAddress)>>7) == 1) {
							return Defs::USBDescriptorStructType::MS_Endpoint_In;
						}
						else {
							return Defs::USBDescriptorStructType::MS_Endpoint_Out;
						}
					}
				}
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::UNDEFINED :
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::AUDIOCONTROL:
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::AUDIOSTREAMING:
					return Defs::USBDescriptorStructType::Unknown;
				default:
					break;
			}// End switch cs endpoint subclass
		default:
			break;
		} // End Class Specific Endpoint Cases
	
	
		//Class Specific INTERFACE for Audio ...
		case USBDefs::USB_DESCRIPTOR_TYPE::AUDIOCLASS_INTERFACE: {
	
			if (iD == nullptr) {
				xERR::println("ERROR - iD == Null Pointer getDescriptorStructType ");
				return Defs::USBDescriptorStructType::Unknown;
			}
			// Make sure iD is populated ...
			if (iD->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::INTERFACE) {
				xERR::println("ERROR - Parent Interface is not valid ", iD->header.bDescriptorType);
				return Defs::USBDescriptorStructType::Unknown;
			}
	
			// we know it's sub class from the parent interface
			switch (iD->bInterfaceSubClass) {
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::AUDIOCONTROL: {
	
					//know its class specific so cast to get the subtype in byte #3
					switch (((USBDefs::AC_InterfaceHeaderStruct*)ukD)->bDescriptorSubtype) {
	
						// even though its midi there is supposed to be an Audio Control Interface per spec
						case USBDefs::AUDIOCONTROL_INTERFACE_DESCRIPTOR_SUBTYPE::HEADER: {
							return Defs::USBDescriptorStructType::AC_Interface_Header;
						}
						default:
							break;
					} // End switch on AC class specific subtype
				}
	
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::MIDISTREAMING: {
	
					//know its class specific so cast to get the subtype in byte #3
					switch ( ((USBDefs::MS_UnknownInterfaceStruct*)ukD)->bDescriptorSubtype )
					{
	
						case USBDefs::MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE::HEADER: {
							return Defs::USBDescriptorStructType::MS_Interface_Header;
						}
						case USBDefs::MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE::IN_JACK: {
							return Defs::USBDescriptorStructType::MS_Interface_InJack;
						}
						case USBDefs::MIDISTREAMING_INTERFACE_DESCRIPTOR_SUBTYPE::OUT_JACK: {
							return Defs::USBDescriptorStructType::MS_Interface_OutJack;
						}
						default:
							break;
					}// End switch on MS class specific subtype
				} // End Midi Streaming
	
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::UNDEFINED:
				case USBDefs::AUDIO_INTERFACE_SUBCLASS::AUDIOSTREAMING:
					return Defs::USBDescriptorStructType::Unknown;
				default:
					break;
			} // End switch on AC/MS subclass
		}// End Class Specific Audio Class Specific Stuff ...
	
	}// End Main switch
	
	return Defs::USBDescriptorStructType::Unknown;
}
void DescriptorHelper::buildStackOfStringsToProcess(DataStorage::USBMidiDriverData* data){

	void* dPtr;

	for (uint8_t i = 0; i < data->nextArrayIndex; i++) {
		dPtr = data->descriptorArray[i].dataPtr;

		switch (data->descriptorArray[i].type) {
		case Defs::USBDescriptorStructType::Device: {

			validateIString(data, &((USBDefs::DeviceStruct*)dPtr)->iManufacturer );
			validateIString(data, &((USBDefs::DeviceStruct*)dPtr)->iProduct);
			validateIString(data, &((USBDefs::DeviceStruct*)dPtr)->iSerialNumber);
			break;
		}
		case Defs::USBDescriptorStructType::Config:{
			validateIString(data, &((USBDefs::ConfigStruct*)dPtr)->iConfiguration);
			break;
		}
		case Defs::USBDescriptorStructType::Interface: {
			validateIString(data, &((USBDefs::InterfaceStruct*)dPtr)->iInterface);
			break;
		}
		case Defs::USBDescriptorStructType::MS_Interface_InJack: {
			validateIString(data, &((USBDefs::MS_InterfaceInJackStruct*)dPtr)->iJack);
			break;
		}
		case Defs::USBDescriptorStructType::MS_Interface_OutJack: {

			// This descriptor is variable in length so struct does not have iJack
			// However, the iJack is the last byte of the data ... so we can get at it.	
			uint8_t* start = (uint8_t *) dPtr;
			uint8_t len = ((USBDefs::MS_InterfaceOutJackStruct*)dPtr)->header.bLength;
			uint8_t* ptrIJack = start +len - 1;

			validateIString(data, ptrIJack);
			break;
		}

			default:
				break; //No strings in these types ...

		}
	}

}
void DescriptorHelper::validateIString (DataStorage::USBMidiDriverData* data,uint8_t* idx) {

	if (*idx == 0) return;

	if (*idx < sizeof(data->iStrings)/sizeof(data->iStrings[0])) {
		data->stackOfstringsToProcess[++(data->topOfstackOfstringsToProcess)] = *idx;
	}
	else {
		xERR::println("ERROR - iString > max we can support ", *idx);
		*idx = 0;  // change it to zero so use of it don't ovr-run our arrays
	}

}
void DescriptorHelper::saveStringDescriptor(DataStorage::USBMidiDriverData* data, uint8_t stringIndex) {

	uint8_t* sdPtr = &(data->stringDescriptorBuff[0]);

	// Check the string buffer has the right type and some data
	const USBDefs::StringStruct* stringDescriptor = (USBDefs::StringStruct*)(sdPtr);
	if (stringDescriptor->header.bDescriptorType != USBDefs::USB_DESCRIPTOR_TYPE::STRING) {
		xERR::println("PARSE ERROR - String Descriptor type is not correct.  Should be 3 is=", stringDescriptor->header.bDescriptorType);
		return;
	}
	if (stringIndex<1) {
		xERR::println("REQUEST ERROR - String Descriptor request should be > 0. Index is=", stringIndex);
		return;
	}
	if (stringDescriptor->header.bLength < 4) {
		xERR::println("PARSE ERROR - String Descriptor has no data.  Length is=", stringDescriptor->header.bLength);
		return;
	}

	//find next spot in the buffer ... 2 nulls
	uint16_t offset = 1;
	for (; offset < sizeof(data->iSB)/sizeof(data->iSB[0]); offset++) {
		if (data->iSB[offset-1] == 0 && data->iSB[offset] == 0) break;
	}

	//set the pointer
	data->iStrings[stringIndex] = &(data->iSB[offset]);

	// Start at index 2 skipping stringDescriptor length,type ASCII = UnicodeLSB
	// increment by 2 as we want every other byte ..
	for (uint8_t i = 2; i < stringDescriptor->header.bLength; i=i+2) {
		if (sdPtr[i] >= 0 && sdPtr[i] <= 127) {
			data->iSB[offset++] = sdPtr[i];
		}
		else {
			data->iSB[offset++] = '?'; // Replace non-ASCII characters with '?'
		}
	}

	data->iSB[offset] = 0; // add a null at the end
} // End function saveStringDescriptor

//---------------  DescriptorHelper    -------  CLASS END ------------
