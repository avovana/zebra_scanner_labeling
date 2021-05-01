#include <iostream>
#include "ConsoleMain.h"
#include "ConsoleSampleEventListener.h"

#include <stdlib.h>
//class LoggingContext;

bool hasAlreadyOpen = false;
using namespace std;
/*
int ReturnChoice()
{
    cout << "======Select CoreScanner command====" << endl;
    cout << "=== 1. GetAttribute " << endl;
    cout << "=== 2. GetAllAttributes " << endl;
    cout << "=== 3. Get Scanners " << endl;
    cout << "=== 4. SetAttribute " << endl;
    cout << "=== 5. SetAttribute-Store " << endl;
    cout << "=== 6. Device LED ON or OFF " << endl;
    cout << "=== 7. Scanner Enable " << endl;
    cout << "=== 8. Scanner Disable " << endl;
    cout << "=== 9. Scanner Reboot " << endl;
    cout << "== 10. Action - Beeper command" << endl;
    cout << "== 11. AIM ON" << endl;
    cout << "== 12. AIM OFF" << endl;
    cout << "== 13. Get Device Topology" << endl;
    cout << "== 14. Firmware Update" << endl;
    cout << "== 15. Start New Firmware" << endl;
    cout << "== 16. Abort Firmware Update" << endl;
    cout << "== 17. Firmware Update From Plugin"<< std::endl;
    cout << "== 18. Get Version "<< endl;
    cout << "== 19. GetNextAttribute " << endl;

    cout << "====================================" << endl;
    cout << "=== 0. Exit" << endl;
    cout << "= 100. Main Menu" << endl;
    //cout << "====================================" << endl;
    //cout << "== 7. SetScaleZero " << endl;
    //cout << "== 8. Discover Tunneling " << endl;
    //cout << "== 9. Open called again " << endl;
    cout << "====================================" << endl;
    cout << "Enter choice : " ;

    int choice = 0;
    cin >> choice;
    return choice;
}

int main(void)
{
    cout << "Zebra Scanner Sample Application" << endl;
    cout << "====================================" << endl;

    SampleEventListener sel;
    SampleEventListener sel2;


    if(STATUS_OK != sel.Open())
    {
        std::cout << "Corescanner service is not Active." << std::endl;
        return STATUS_ERROR;
    }

    sel.GetScanners();

    int choice = ReturnChoice();

    do {
        switch (choice) {
        case 1:
            sel.GetAttribute();
            break;
        case 2:
            sel.GetAllAttributes();
            break;
        case 3:
            sel.GetScanners();
            break;
        case 4:
            sel.SetAttribute();
            break;
        case 5:
            sel.SetAttributeStore();
            break;
        case 6:
            sel.ExecuteActionCommand(CMD_DEVICE_LED_ON);
            break;
        case 7:
            sel.ExecuteActionCommand(CMD_DEVICE_SCAN_ENABLE);
            break;
        case 8:
            sel.ExecuteActionCommand(CMD_DEVICE_SCAN_DISABLE);
            break;
        case 9:
            sel.ExecuteActionCommand(CMD_REBOOT_SCANNER);
            break;
        case 10:
            sel.ExecuteActionCommand(CMD_DEVICE_BEEP_CONTROL);
            break;
        case 11:
            sel.ExecuteActionCommand(CMD_DEVICE_AIM_ON);
            break;
        case 12:
            sel.ExecuteActionCommand(CMD_DEVICE_AIM_OFF);
            break;
        case 13:
            sel.GetDeviceTopology();
            break;
        case 14:
            sel.FirmwareUpdate();
            break;
        case 15:
            sel.StartNewFirmware();
            break;
        case 16:
            sel.AbortFirmwareUpdate();
            break;
        case 17:
            sel.FirmwareUpdateFromPlugin();
            break;
        case 18:
            sel.GetVersion();
            break;
        case 19:
            sel.GetAttributeNext();
            break;
        case 100:
            cout << "====================================" << endl;
            break;
        case 0:
        default:
            break;
        }

        if(choice != 0)
            choice = ReturnChoice();

    } while (choice);
    sel.Close();
    return 0;
}
*/
