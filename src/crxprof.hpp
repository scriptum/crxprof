#ifndef CRXPROF_HPP__
#define CRXPROF_HPP__

#include <iostream>
#include <libunwind.h>
#include <stdint.h>
#include <vector>
#include "defs.hpp"

#define DEFAULT_FREQ            100
#define MAX_STACK_DEPTH         128

struct fn_descr {
    char *name;
    long addr;
    unsigned len;
};

struct calltree_node;
typedef std::vector<calltree_node *> siblings_t;


struct calltree_node {
    const fn_descr *pfn;
    uint64_t nintermediate;
    uint64_t nself;
    siblings_t childs;

    calltree_node(const fn_descr *a_pfn) : pfn(a_pfn), nintermediate(0), nself(0) {}
};

enum crxprof_method { PROF_REALTIME = 1, PROF_CPUTIME = 2, PROF_IOWAIT = 4 };

struct trace_stack {
    unw_word_t ips[MAX_STACK_DEPTH];
    int depth;
};

struct ptrace_context {
    pid_t pid;
    unw_addr_space_t addr_space;
    void *unwind_rctx;
    uint64_t prev_cputime;
    siginfo_t stop_info;
    clockid_t clock_id;

    char procstat_path[sizeof("/proc/4000000000/stat")];
    struct trace_stack stk;

    uint64_t nsnaps;
    uint64_t nsnaps_accounted;
};

struct vproperties {
    unsigned max_depth;
    int  min_cost;
    bool print_fullstack;

    vproperties() : max_depth(-1U), min_cost(5), print_fullstack(false)  {}
};


/* ptrace-related functions */
bool trace_init(pid_t pid, struct ptrace_context *ctx);
bool get_backtrace(struct ptrace_context *ctx);
bool fill_backtrace(uint64_t cost, const struct trace_stack &stk, 
                   const std::vector<fn_descr> &funcs, calltree_node **root);
uint64_t get_cputime_ns(struct ptrace_context *ctx);
char get_procstate(const ptrace_context &ctx); /* One character from the string "RSDZTW" */

/* visualize and dumps */
void visualize_profile(calltree_node *root, const vproperties &vprops);
void dump_callgrind(const std::vector<fn_descr> &funcs, calltree_node *root, std::ostream &os);


void print_message(const char *fmt, ...) __attribute__((__format__(printf, 1, 2)));

#endif /* CRXPROF_HPP__ */
