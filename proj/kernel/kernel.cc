#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "sys.h"
#include "threads.h"
#include "ports.h"

const char* initName = "/sbin/init";
const uint16_t VGA_WIDTH = 80;
const uint16_t VGA_HEIGHT = 25;
uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

namespace gheith {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
}


// void display_string(const char* str) {
//     uint16_t* current_pos = VGA_MEMORY;

//     // Loop over each character in the string
//     for (int i = 0; str[i] != '\0'; i++) {
//         // If we've reached the end of a row, move to the start of the next row
//         if ((current_pos - VGA_MEMORY) % VGA_WIDTH == 0 && current_pos != VGA_MEMORY) {
//             current_pos += VGA_WIDTH;
//         }

//         // Write the character to the VGA buffer
//         *current_pos++ = (uint16_t)str[i] | 0x0700;
//     }
// }



void kernelMain(void) {
     //change VGA mode to text mode
    Ports addressPort(0x3D4);
    addressPort.write(0x03);
    Ports dataPort(0x3D5);
    dataPort.write(0x00);
    // MACHINE::outb(0x3D4, 0x03); //write value 3 to address port 0x3d4
    // MACHINE::outb(0x3D5, 0x00); 

    // Pointer to video memory
    unsigned short* video_memory = (unsigned short*) 0xB8000;
    while(1) {
        // Output a string to the screen
        const char* message = "Hello, world!";
        for (int i = 0; message[i] != '\0'; ++i) {
            video_memory[i] = (video_memory[i] & 0xFF00) | message[i];
        }
    }
    
    
    auto argv = new const char* [2];
    argv[0] = "init";
    argv[1] = nullptr;
    
    int rc = SYS::exec(initName,1,argv);
    Debug::panic("*** rc = %d",rc);
}

