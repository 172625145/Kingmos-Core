// right shift
__int64 __imp___rt_srsh( __int64 l, int shift )
{
#ifdef SWAP
	struct {
		unsigned long lower;
		long upper;
	} l64;
#else
	struct {
		long upper;
		unsigned long lower;
	} l64;
#endif
	int i = shift;
	unsigned long mask = 0xffffffff;
	*(__int64*)&l64 = l;
	l64.lower >>= i;
	//mask >>= 32 - i;
	l64.lower |= l64.upper << (32 - i);
	l64.upper >>= i;
	l = *(__int64*)&l64;		 
	// lilin change- 2001.12.28-begin
	return l;
}
