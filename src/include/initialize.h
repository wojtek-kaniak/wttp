#if !defined (INCL_INITIALIZE_H)
#define INCL_INITIALIZE_H

/// Initialize global structures, should be called once per process
void initialize_global();

/// Initialize thread local structures on the current thread, should be called once per thread
void initialize_thread();

#endif
