#define NSIG 32

// Use high sentinel values so valid user function addresses
// (including 0x0 in this xv6 user linker layout) are not mistaken
// for default/ignore handlers.
#define SIG_DFL (~0ULL)
#define SIG_IGN (~1ULL)

#define SIGKILL 9
#define SIGUSR1 10
