#include "types.h"


#define FDSET_SIZE		1024
#define NFDBIT			(sizeof(uint64) * 8)	//数组每一位可表示的fd个数
#define NFDBITS			(FDSET_SIZE / NFDBIT)	//数组位数

struct fdset {
	uint64 fd_bits[NFDBITS];
};