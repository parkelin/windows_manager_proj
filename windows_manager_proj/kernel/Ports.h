// #include "stdint.h"
// #include "machine.h"



class Ports {
    
    public:
        int port_number;
        Ports(int port_number): port_number(port_number) {}

        ~Ports() {}

        unsigned char read();

        void write(int val);
};

