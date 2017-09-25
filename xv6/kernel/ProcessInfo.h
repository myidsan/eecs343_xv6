struct ProcessInfo {
   int pid; // process id
   int ppid; // parent pi:u
   int state; // state
   uint sz; // size in bytes
   char name[16]; // name of process
};
