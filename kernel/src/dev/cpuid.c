#include <dev/cpuid.h>

char* vendors[] = {
	"GenuineIntel",
	"AuthenticAMD",
	"Unknown CPU"
};

char* cpuid_name()
{
	uint32_t magic, blank;
	cpuid(0, blank, magic, blank, blank);

	switch(magic)
	{
		case 0x756e6547:
			return vendors[0];
		case 0x68747541:
			return vendors[1];
		default:
			return vendors[2];
	}
}
