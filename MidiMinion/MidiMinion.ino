#include "DataStorage.h"
#include "DBG.h"
#include "DevicesInterface.h"



// -----------  MidiMinion CLASS DEF -----------
class MidiMinion {
public: 
    static const bool PrintClassDEBUG = true;
    static const bool PrintClassERROR = true;

    // - Processing States
    enum class MODE : uint8_t { Starting, Connected, Disconnected, ReadMidiMessages };


    // --- MODE Processing Stuff ----
    static inline MODE mode = MODE::Starting;
    static void starting();
    static void disconnected();
    static void connected();

}; // -----  MidiMinion STATIC Class END ----------



// ----------  ARDUINO LOOP -----------------

void setup() { delay(100); }
void loop() {

    // ************ PROCESS DEVICES  ****************

        switch (MidiMinion::mode) {
            case MidiMinion::MODE::Starting: { MidiMinion::starting(); break; }
            case MidiMinion::MODE::Disconnected: { MidiMinion::disconnected();  break; }
            case MidiMinion::MODE::Connected: { MidiMinion::connected(); break; }
            case MidiMinion::MODE::ReadMidiMessages: {
                DeviceManager::readMidiMessages();
                    // process any messages in the queue

                    break;
                }// ReadMidi Mode
            default: break;
        }
    
        // ALWAYS remove the msg to not stall the Q
        if (DeviceMessageQueue::getNextMsgType() != Defs::DeviceEventType::EmptyQueue) {
            DeviceMessageQueue::remove(); 
        }

        DeviceManager::task(); // -- Not sure devices use this

        // --- Give DeviceQ Priority and loop again
        if (DeviceMessageQueue::hasMoreWork()) return;

    // ***** End Process DeviceMsgQ ***********************

    
    
    // TODO:  Process UI Messages


} // ----------  ARDUINO LOOP END -----------------



// -----------  MidiMinion FUNCTIONS for Processing Modes -----------

void MidiMinion::starting() {
    DeviceManager::begin();

    // are devices ready in at startup ?  Probably not USB ones yet ...

    if (DeviceManager::aDeviceIsReady() ) { 
        MidiMinion::mode = MidiMinion::MODE::Connected; 
    }
    else { 
        MidiMinion::mode = MidiMinion::MODE::Disconnected; 
    }
}
void MidiMinion::disconnected() {
    switch (DeviceMessageQueue::getNextMsgType())
    {
        case Defs::DeviceEventType::EmptyQueue: { break; }
        case Defs::DeviceEventType::MidiMessage: { break; }

        case Defs::DeviceEventType::DeviceConnect: {
            // TODO: tell UIout to process
            MidiMinion::mode = MidiMinion::MODE::Connected;
            break;
        }
        case Defs::DeviceEventType::DeviceDisconnect: {
            xDBGS(MidiMinion)::println("Got a disconnect event after all devices are disconnected?");
            break;
        }
        case Defs::DeviceEventType::ERROR: {
            xERRS(MidiMinion)::println("ERROR - Got an ERROR Event type in main loop ?");
            break;
        }
        default: {
            xERRS(MidiMinion)::println("ERROR - Unknown Event type ?");
            break;
        }
    }
}
void MidiMinion::connected() {
    switch (DeviceMessageQueue::getNextMsgType())
    {
        case Defs::DeviceEventType::EmptyQueue: { break; }
        case Defs::DeviceEventType::MidiMessage: {
            // TODO: handle these
            break;
        }
        case Defs::DeviceEventType::DeviceDisconnect: {
            // TODO: tell UIout to process
            if (!DeviceManager::aDeviceIsReady()) {
                MidiMinion::mode = MidiMinion::MODE::Disconnected;
            }
            break;
        }
        case Defs::DeviceEventType::DeviceConnect: {
            // TODO: tell UIout to process
            break;
        }
        case Defs::DeviceEventType::ERROR: {
            xERRS(MidiMinion)::println("ERROR - Got an ERROR Event type in main loop ?");
            break;
        }
        default: {
            xERRS(MidiMinion)::println("ERROR - Unknown Event type ?");
            break;
        }
    }
}

