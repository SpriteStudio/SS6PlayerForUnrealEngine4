
#ifndef __XORSHIFT32__
#define __XORSHIFT32__



class xorshift32
{
protected:
    uint32 y;
public:
	xorshift32(){}
	~xorshift32(){}

	void	init_genrand(uint32 seed) {y=seed;}

	uint32 genrand_uint32(){
		y = y ^ ( y << 13 );
		y = y ^ ( y >> 17 );
		y = y ^ ( y << 15 );

		return y;
	}

	float genrand_float32() {
		uint32 v = genrand_uint32();
		uint32 res = (v >> 9) | 0x3f800000;
		return (*(float*)&res) - 1.0f;
	}

} ;




#endif
