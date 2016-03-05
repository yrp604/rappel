* move all arch specific stuff into their own folder tree
* a mode where `rdtsc` is inserted before and after every instruction, and timing info retrieved
* the ability to map arbitrary memory in the child process. I.e. map a file into the address space, or just declare chunks of memory as mapped
* thumb support
* ability to specify the entry point
* move generated binaries to memfd or shm so as to remove filesystem dep
* .showstack command
* split .show into a family of commands?
