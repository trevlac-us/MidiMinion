#include "UserInterface.h"
#include "DataStorage.h"
#include "DBG.h"
#include "DevicesInterface.h"



// -----------  MidiMinion CLASS DEF -----------
class MidiMinion {
public: 
    static const bool PrintClassDEBUG = true;
    static const bool PrintClassERROR = true;

    // - States
    enum class DEVICEMODE : uint8_t { Starting, Connected, Disconnected };
    enum class PROCESSMODE : uint8_t { Off, Reading, Routing, Writing };


    // --- MODE Processing Stuff ----
    static inline DEVICEMODE deviceMode = DEVICEMODE::Starting;
    static inline PROCESSMODE processMode = PROCESSMODE::Off;

    static void starting();
    static void disconnected();
    static void connected();

}; // -----  MidiMinion STATIC Class END ----------



// ----------  ARDUINO LOOP -----------------

void setup() { delay(100); }
void loop() {

    // ************ PROCESS DEVICE EVENTS  ****************
        switch (MidiMinion::deviceMode) {
            case MidiMinion::DEVICEMODE::Starting: { MidiMinion::starting(); break; }
            case MidiMinion::DEVICEMODE::Disconnected: { MidiMinion::disconnected();  break; }
            case MidiMinion::DEVICEMODE::Connected: { 
                MidiMinion::connected(); 
                DeviceManager::readMidiMessages();
                break; 
            }
            default: break;
        }
    
        // ALWAYS remove the msg to not stall the Q
        if (DeviceEventQueue::getNextEventType() != Defs::DeviceEventType::EmptyQueue) {DeviceEventQueue::remove(); }

        DeviceManager::task(); // -- Not sure devices use this

        // --- Give DeviceQ Priority and loop again
        if (DeviceEventQueue::hasMoreWork()) return;
    // ***** End PROCESS DEVICE EVENTS ***********************



    // ************ PROCESS MIDI MESSAGE  ****************
        while(MidiMsgQueue::hasMoreWork()) { 

            //TODO:  Run Transformers which load UI Q
            //    { Off, Reading, Routing, Writing };

            MidiMsgQueue::remove(); 
        }
    // ********** END PROCESS MIDI MESSAGE  **************

    

    // TODO:  Process UI Messages


} // ----------  ARDUINO LOOP END -----------------



// -----------  MidiMinion FUNCTIONS for Processing Device Modes -----------

void MidiMinion::starting() {
    DeviceManager::begin();

    // are devices ready in at startup ?  Probably not USB ones yet ...

    if (DeviceManager::aDeviceIsReady() ) { 
        MidiMinion::deviceMode = MidiMinion::DEVICEMODE::Connected; 
    }
    else { 
        MidiMinion::deviceMode = MidiMinion::DEVICEMODE::Disconnected; 
    }
}
void MidiMinion::disconnected() {
    switch (DeviceEventQueue::getNextEventType())
    {
        case Defs::DeviceEventType::EmptyQueue: { break; }

        case Defs::DeviceEventType::DeviceConnect: {
            // Add msg to UI Q ...change mode
            UIMessageQueue::add(Defs::UIMessageType::DeviceConnect, DeviceEventQueue::getNextEventDataPtr());
            MidiMinion::deviceMode = MidiMinion::DEVICEMODE::Connected;
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
    switch (DeviceEventQueue::getNextEventType())
    {
        case Defs::DeviceEventType::EmptyQueue: { break; }

        case Defs::DeviceEventType::DeviceDisconnect: {
            UIMessageQueue::add(Defs::UIMessageType::DeviceDisconnect, DeviceEventQueue::getNextEventDataPtr());
            if (!DeviceManager::aDeviceIsReady()) {
                MidiMinion::deviceMode = MidiMinion::DEVICEMODE::Disconnected;
            }
            break;
        }

        case Defs::DeviceEventType::DeviceConnect: {
            UIMessageQueue::add(Defs::UIMessageType::DeviceConnect, DeviceEventQueue::getNextEventDataPtr());
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

