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
    //set up port 0x3c0, write index and data to same port
    unsigned char value;
    Ports firstport(0x3DA);
    unsigned char status = firstport.read();
    if((status & 0x01) == 0x01){
        Ports tempport(0x3C0);
        tempport.write(0x0A);
    }    
    Ports secondport(0x3C1);
    value = secondport.read();

    //port 0x3c2: miscellaneous output register
    unsigned char colorGraphicsMode = 0x01;
    unsigned char enableVideoBit = 0x20; //must combine these
    Ports miscPort(0x3C2);
    //enable VGA display by setting bit 0 of 0x3c2 to 1
    miscPort.write(colorGraphicsMode | enableVideoBit); //enable color graphics and text mode

    
    // accessing VGA port 0x3C4 
    Ports tempport(0x3C4);
    Ports tempporttwo(0x3C5);
    tempport.write(0x01);
    tempporttwo.write(0xFF);

    // accessing VGA port 0x3CE

    Ports tempportthree(0x3CE);
    Ports tempportfour(0x3CF);
    tempport.write(0x05);
    tempporttwo.write(0xFF);


    // accessing VGA port 0x3D4, mode control reg
    Ports tempportfive(0x3C2); // 
    Ports tempportsix(0x3D4);
    Ports tempportseven(0x3D5);
    

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

