#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "threads.h"
#include "process.h"
#include "machine.h"
#include "ext2.h"
#include "elf.h"
#include "libk.h"
#include "file.h"
#include "heap.h"
#include "shared.h"
#include "kernel.h"

char* abs_path;

int SYS::exec(const char* path,
              int argc,
              const char* argv[]
) {
    using namespace gheith;
    auto file = root_fs->find(root_fs->root,path); 
    if (file == nullptr) {
        return -1;
    }
    if (!file->is_file()) {
        return -1;
    }

    uint32_t sp = 0xefffe000;
    uint32_t* moving_sp = (uint32_t*)sp; 

    uint32_t* char_ptrs[argc];

    for (int i = 0; i < argc; i++) {
        uint32_t len = K::strlen(argv[i]) + 1;
        uint32_t moveAmt = len / 4;

        if (len % 4 != 0) {
            moving_sp = moving_sp - (moveAmt + 1);
        } else { 
            moving_sp = moving_sp - moveAmt;
        }

        char_ptrs[i] = moving_sp;
        memcpy(moving_sp, (char*) argv[i], K::strlen(argv[i])+1);
    }

    moving_sp = moving_sp - 1;
    *moving_sp = '\0';

    for (int k = argc-1; k >= 0; k--) {
        // Debug::printf("arg[%d]:%s\n", k, char_ptrs[k]);
        *moving_sp = (uint32_t)char_ptrs[k];
        moving_sp--;
    }

    *moving_sp = (uint32_t)(moving_sp + 1);
    moving_sp--;
    *moving_sp = (uint32_t)argc; 
    sp = (uint32_t)moving_sp; 

    uint32_t e = ELF::load(file);

    file == nullptr;
    switchToUser(e,sp,0);
    Debug::panic("*** implement switchToUser");
    return -1;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    using namespace gheith;

    // current()->process;
    uint32_t *userEsp = (uint32_t*)frame[3];
    uint32_t userPC = frame[0];

    // Debug::printf("*** syscall #%d\n",eax);

    switch (eax) {
    case 0:
        {
            auto status = userEsp[1];
	    // MISSING();
            current()->process->output->set(status);
            stop();
            return 0;
        }
    case 1: /* write */
        {
            // extern ssize_t write(int fd, void* buf, size_t nbyte);
            int fd = (int) userEsp[1];
            char* buf = (char*) userEsp[2];
            size_t nbyte = (size_t) userEsp[3];

            auto file = current()->process->getFile(fd);

            if (fd == 0) {
                return -1;
            }

            if (file == nullptr) {
                return -1;
            }
            return file->write(buf,nbyte);
        }
    case 2: /* fork */
    	{
            using namespace gheith;
            int id = 0;
            Shared<Process> child_process = current()->process->fork(id);
            if (id > 0) {
                thread(child_process, [userPC, userEsp] {
                    switchToUser(userPC, (uint32_t) userEsp, false);
                });
            } 

            return id;
    	}
    case 3: /* sem */
        {
            // int sem(uint32_t initial);
		    uint32_t initial = (uint32_t) userEsp[1];
            int new_sem = current()->process->newSemaphore(initial);
    		return new_sem;
        }

    case 4: /* up */
    	{
            // int up(int id);
		   int id = (int) userEsp[1];

            Shared<Semaphore> s = current()->process->getSemaphore(id);

            if (id == -1) {
                return -1;
            }

            if (s != nullptr) {
                s->up();
                return 0;
            } 
    		return -1;
    	}
    case 5: /* down */
      	{
            // int down(int id);
            int id = (int) userEsp[1];

            Shared<Semaphore> s = current()->process->getSemaphore(id);
            
            if (id == -1) {
                return -1;
            }

            if (s != nullptr) {
                s->down();
                return 0;
            }
    		return -1;
       	}
    case 6: /* close */
    {
        // extern int close(int id);
        int id = (int) userEsp[1];
        return current()->process->close(id);
    }

    case 7: /* shutdown */
		Debug::shutdown();
        return -1;

    case 8: /* wait */
    {
        int id = (int) userEsp[1];
        uint32_t* status = (uint32_t*) userEsp[2];

        int num = current()->process->wait(id, status);
        return num;
    }

    case 9: /* execl */
    // execl(const char* path, const char* arg0, ...);
    {
        char* path = (char*) userEsp[1];

        if ((uint32_t) path < 0x80000000) {
            return -1;
        }
        
        // TEST 60 CHECK
        Shared<Node> file = root_fs->find(root_fs->root, path);

        if (file == nullptr) {
            return -1;
        }

        ElfHeader hdr;
        file->read(0, hdr);
        uint32_t hoff = hdr.phoff;

        if (hdr.magic0 != 0x7f || hdr.magic1 != 'E' || hdr.magic2 != 'L' || hdr.magic3 != 'F') {
            return -1;
        }

        for (uint32_t i=0; i<hdr.phnum; i++) {
            ProgramHeader phdr;
            file->read(hoff, phdr);
            hoff += hdr.phentsize;
            
            if (phdr.type == 1) {
                if (phdr.vaddr < 0x80000000 || (phdr.memsz + phdr.vaddr < 0x80000000)) {
                    return -1;
                }
            }
        }

        // Loop until \0 - argc
        int argc_ = 0;
        for (int i = 2; userEsp[i] != 0; i++) {
            if (userEsp[i] < 0x80000000 || userEsp[i] > 0xFFFFFFFF) {
                return -1;
            }
            argc_++;
        }

        // Make a NEW array of pointers (on kernel's heap) and fill it in - argv
        char** argv_ = new char*[argc_];

        int p = 0;
        for (int a = 2; a < argc_+2; a++) {
            char* argument = (char*) userEsp[a];
            // Debug::printf("%s", argument);
            argv_[p] = argument;
            // Debug::printf("%s", argv_[p]);
            p++;
        }

        // Clear semaphores
        current()->process->clearSems();

        // exec(const char* path, int argc, const char* argv[])
        SYS::exec(path, argc_, (const char**) argv_);

        // Clear the address space
    }

    case 10: /* open */
    {
        using namespace gheith;

        // extern int open(const char* fn, int flags);
        char* abs_path = (char*) userEsp[1];

        if ((uint32_t)abs_path < 0x80000000) {
            return -1;
        }

        // Shared<Node> find(Shared<Node> current, const char* path)
        Shared<Node> node = root_fs->find(root_fs->root, abs_path);
        

        // Not valid file path
        if (node == nullptr) {
            return -1;
        } 

        char buf[node->size_in_bytes() + 1];
        buf[node->size_in_bytes()] = '\0';
        // Loop 10 times, checking if it's a symlink
        // if not, return -1
        // do get_symbol() to find the next thing it's pointing to in the link
        int i;
        for (i = 0; i < 10; i++) {
            if (node->is_symlink()) {
                node->get_symbol((char*) buf);
                buf[node->size_in_bytes()] = 0;
                node = root_fs->find(root_fs->root, buf);
            } else {
                break;
            }
        }
        if (i >= 10) {
            return -1;
        }

        File2* file2 = new File2(node);
        Shared<File> file{file2};

        auto index = current()->process->setFile(file);
        return index;
    }

    case 11: /* len */
    {
        // extern ssize_t len(int fd);
        int fd = (int) userEsp[1];
        auto file = current()->process->getFile(fd);

        // If fd is an invalid file descriptor
        if (file == nullptr) {
            return -1;
        } else if (fd == 0 || fd == 1 || fd == 2) { // Points to stdin/out/err
            return 0;
        }
        // Return length of the file
        return file->size();
    }
    
    case 12: /* read */
    {
        // extern ssize_t read(int fd, void* buf, size_t nbyte);
        // int fd = (int) userEsp[1];
        // char* buf = (char*) userEsp[2];
        // size_t nbyte = (size_t) userEsp[3];

        // auto file = current()->process->getFile(fd);

        // // Return negative if fd is invalid, not in user space, or points to stdin/out/err
        // if (file == nullptr || 
        //     // (uint32_t) buf < 0x80000000 || (uint32_t) buf > 0xFFFFFFFF || /* (uint32_t) buf == APIC/LAPIC */
        //     fd == 0 || fd == 1 || fd == 2) {
        //         return -1;
        // } 
            int fd = (int) userEsp[1];
            char* buf = (char*) userEsp[2];
            size_t nbyte = (size_t) userEsp[3];

            auto file = current()->process->getFile(fd);

            if (file == nullptr || fd == 0 || fd == 1 || fd == 2) {
                return -1;
            }

            if ((uint32_t)buf <= 0x80000000 
            || (uint32_t)buf >= kConfig.ioAPIC
            || (uint32_t)buf >= kConfig.localAPIC) {
                return -1;
            }

            return file->read(buf,nbyte);
    }

    case 13: /* seek */
    {
        // seek(int fd, off_t offset);
        int fd = (int) userEsp[1];
        off_t offset = (off_t) userEsp[2];

        auto file = current()->process->getFile(fd);
         
        // Return negative if fd is invalid fd
        // fd points to stdin/out/err
        if (file == nullptr || fd == 0 || fd == 1 || fd == 2) {
            return -1;
        }

        // Return offset
        return file->seek(offset);
    }

    default:
        Debug::printf("*** 1000000000 unknown system call %d\n",eax);
        return -1;
    }
    
}   

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
