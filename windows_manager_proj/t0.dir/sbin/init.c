#include <stdbool.h>
#include <stdio.h>
#include "stdint.h"
#include "/u/ramosden/cs439/windows_manager_proj/windows_manager_proj/kernel/machine.h"


// void write_to_address(int val) {

//     //change VGA mode to text mode
//     //0x3D4 selects VGA register that controls VGA mode
//     outb(0x3D4, 0x03); //write value 3 to address port 0x3d4
//     outb(0x3D5, 0x00); //writs value 0x00 to data port 0x3D5 to set vga mode to text mode

// }

// void read_regs(unsigned char* regs) {

// }

void display_words(const char* str, unsigned char color) {
    unsigned short * vga_memory_addr = (unsigned short*)0xB8000;

    while(*str){
        unsigned short data = (unsigned char)(*str++) | ((unsigned short) color << 8);
        *vga_memory_addr++ = data;
    }
}

int main() {
    //change VGA mode to text mode
    outb(0x3D4, 0x03); //write value 3 to address port 0x3d4
    outb(0x3D5, 0x00); 

    const char* str = "Hi from Denise and Kat!";
    unsigned char color = 0xF;
    display_words(str, color);


    while (true) {
        printf("here!!!!!!");
    }
}

