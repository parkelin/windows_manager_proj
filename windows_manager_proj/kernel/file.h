#ifndef _FILE_H_
#define _FILE_H_

#include "atomic.h"
#include "stdint.h"
#include "shared.h"
#include "ext2.h"

class File {
    Atomic<uint32_t> ref_count;
public:
    File(): ref_count(0) {}
    virtual ~File() {}
    virtual bool isFile() = 0;
    virtual bool isDirectory() = 0;
    virtual off_t size() = 0;
    virtual off_t seek(off_t offset) = 0;
    virtual ssize_t read(void* buf, size_t size) = 0;
    virtual ssize_t write(void* buf, size_t size) = 0;

    friend class Shared<File>;
};

class File2 : public File {
    Atomic<uint32_t> ref_count;
public:
    uint32_t offset = 0;
    Shared<Node> node;
    File2(Shared<Node> n): ref_count(0) { node = n; }
    ~File2() {}
    bool isFile() { return false; }
    bool isDirectory() {return false; }

    off_t size() { 
        return node->size_in_bytes();
    }

    off_t seek(off_t off) { 
        offset = off;
        return offset;
    }

    ssize_t read(void* buf, size_t size) { 
        // make sure we DON'T hit an invalid address (non user space)
        // or hit the end of the file (offset no longer in the file)
        // Debug::printf("in read, offset: %d, size: %d, file size: %d\n", offset, size, node->size_in_bytes());
        
        if (offset + size >= node->size_in_bytes()) {
            size = node->size_in_bytes() - offset;
            if (size < 0) {
                return 0;
            } 
            node->read_all(offset, size, (char*) buf); // offset, n, buffer
        } else {
            node->read_all(offset, size, (char*) buf);
        }
        offset += size;
        return size;
    }

    ssize_t write(void* buf, size_t size) { 
        return 0; 
    }

    friend class Shared<File2>;
};

#endif
