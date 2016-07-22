#include <string.h>

// This needs to be modified to handle expressions

#define RED "\x1b[0;31m"
#define RST "\x1b[0m"

#define DUMPREG64(x, y, z) \
	do {\
		if (y->x == z->x) \
		printf(REGFMT64, y->x); \
		else \
		printf(RED REGFMT64 RST, y->x); \
	} while (0)

#define PRINTREG64(h, x, y, z, t) \
	do {\
		printf("%s", h); \
		DUMPREG64(x, y, z); \
		printf("%s", t);\
	} while (0)

#define DUMPREG32(x, y, z) \
	do {\
		if (y->x == z->x) \
		printf(REGFMT32, y->x); \
		else \
		printf(RED REGFMT32 RST, y->x); \
	} while (0)

#define PRINTREG32(h, x, y, z, t) \
	do {\
		printf("%s", h); \
		DUMPREG32(x, y, z); \
		printf("%s", t);\
	} while (0)

#define DUMPREG16(x, y, z) \
	do {\
		if (y->x == z->x) \
		printf(REGFMT16, y->x); \
		else \
		printf(RED REGFMT16 RST, y->x); \
	} while (0)

#define PRINTREG16(h, x, y, z, t) \
	do {\
		printf("%s", h); \
		DUMPREG16(x, y, z); \
		printf("%s", t);\
	} while (0)

#define DUMPREG8(x, y, z) \
	do {\
		if (y->x == z->x) \
		printf(REGFMT8, y->x); \
		else \
		printf(RED REGFMT8 RST, y->x); \
	} while (0)

#define PRINTREG8(h, x, y, z,t) \
	do {\
		printf("%s", h); \
		DUMPREG8(x, y ,z); \
		printf("%s", t);\
	} while (0)

#define PRINTBIT(name, y, z, t) \
	do {\
		if (y == z) \
		printf("%s%d", name, y); \
		else \
		printf(RED "%s%d" RST, name, y); \
		printf("%s", t); \
	} while (0)
