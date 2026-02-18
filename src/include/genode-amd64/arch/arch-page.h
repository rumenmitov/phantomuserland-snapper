// Copied from amd64


#define PAGE_SHIFT 12

// INFO Changed name because PAGE_SIZE conflicts with Genode.
#define ARCH_PAGE_SIZE 4096

/* offset within page */
#define PAGE_MASK 0xfff
