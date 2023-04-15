#ifndef PORTS_H
#define PORTS_H
#include "stdint.h"
// #include "machine.h"



class Ports {
    
    public:
        int port_number;
        Ports(int port_number): port_number(port_number) {}

        ~Port() {}

        unsigned char read();

        void write(int val);
};
#endif
