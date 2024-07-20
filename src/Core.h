#ifndef _DCE_CORE_H
#define _DCE_CORE_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <signal.h>

#ifdef DEBUG
#define DCE_ASSERT(x, msg, ...) { if(!(x)) { printf(msg, ##__VA_ARGS__); raise(SIGTRAP); } }
#define DCE_ASSURE_OR_EXIT(x, msg, ...) { if(!(x)) { printf(msg, ##__VA_ARGS__); raise(SIGTRAP); exit(EXIT_FAILURE); } }
#else
#define DCE_ASSERT(x, msg, ...) 
#define DCE_ASSURE_OR_EXIT(x, msg, ...) { if(!(x)) { printf(msg, ##__VA_ARGS__); exit(EXIT_FAILURE); } }
#endif


#endif // !_DCE_CORE_H
