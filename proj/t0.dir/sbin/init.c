#include <stdbool.h>
#include <stdio.h>
#include "stdint.h"
// #include "ports.h"
// #include "machine.h"
// #include "display.h"

// #include "/u/ramosden/cs439/windows_manager_proj/windows_manager_proj/kernel/machine.h"
// #include "/u/ramosden/cs439/windows_manager_proj/windows_manager_proj/kernel/ports.h"
// #include "libc.h"
// #include "/usr/include/x86_64-linux-gnu/sys/io.h"

// void write_to_address(int val) {

//     //change VGA mode to text mode
//     //0x3D4 selects VGA register that controls VGA mode
//     outb(0x3D4, 0x03); //write value 3 to address port 0x3d4
//     outb(0x3D5, 0x00); //writs value 0x00 to data port 0x3D5 to set vga mode to text mode

// }

// void read_regs(unsigned char* regs) {

// }

// void display_words(const char* str, unsigned char color) {
//     unsigned short * vga_memory_addr = (unsigned short*)0xB8000;

//     while(*str){
//         unsigned short data = (unsigned char)(*str++) | ((unsigned short) color << 8);
//         *vga_memory_addr++ = data;
//     }
// }

int main() {
    //change VGA mode to text mode
    // Ports addressPort(0x3D4);
    // Ports dataPort(0x3D5);
    
    // // addressPort.write(0x03);
    // // dataPort.write(0x00);
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
    
//   while(1) {

//         // clear_screen();
//         char* line;
//         for (int i = 1; i <= 35; ++i) {
//             int_to_string(i, line, 50);
//             print_string(line);
//             print_nl();
//         }

//     }

    // while (true) {
    //     printf("here!!!!!!");
    // }
    // return 0;
}

