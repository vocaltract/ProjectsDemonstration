#ifdef DEBUG
#define Log(format, ...)\
    printf("[%d,%s] " format "\n",\
        __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log(format, ...) 
#endif
#ifdef DEBUG
  #define Assert(expr) assert(expr)
#else
  #define Assert(expr) 
#endif

#ifdef DEBUG_CTX
extern char etext;
#if __x86_64__
#define legal_context(ctx) { assert((ctx)->cs==8);\
    assert((ctx)->rip>=0x10000 && (ctx)->rip <=(uintptr_t)&etext);}
#else
#define legal_context(ctx) { assert((ctx)->cs==8);\
    assert((ctx)->eip>=0x10000 && (ctx)->eip <=(uintptr_t)&etext);}
#endif
#else
#define legal_context(ctx)
#endif