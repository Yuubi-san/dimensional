
#include <cstdint>
#include "../include/dimensional/impl/rational_constant.hpp"

#include "test.hpp"
test
{
	using namespace meta::rational_constant_literals;
	using meta::make_rational_constant;
	using meta::ratio;
	using mjk::square;

	expect( make_rational_constant<INTMAX_MIN>() )eq( ratio<INTMAX_MIN,1>{} );

	expect( 0_ )eq( ratio<0,1>{} );
	expect( 1_ )eq( ratio<1,1>{} );
	expect( .0_ )eq( ratio<0,1>{} );
	expect( 0._ )eq( ratio<0,1>{} );
	expect( 0.0_ )eq( ratio<0,1>{} );
	expect( 0x0_ )eq( ratio<0,1>{} );

	expect( 9_ )eq( ratio<9,1>{} );
	expect( 19_ )eq( ratio<19,1>{} );
	expect( 07.0_ )eq( ratio<7,1>{} );
	expect( 017.0_ )eq( ratio<15,1>{} );
	expect( 0xf_ )eq( ratio<15,1>{} );
	expect( 0b1_ )eq( ratio<1,1>{} );
	expect( 0b11_ )eq( ratio<3,1>{} );

	expect( 0.1_ )eq( ratio<1,10>{} );
	expect( 0.01_ )eq( ratio<1,100>{} );
	expect( 0.11_ )eq( ratio<11,100>{} );
	expect( 00.1_ )eq( ratio<1,8>{} );
	expect( 00.01_ )eq( ratio<1,64>{} );
	expect( 00.11_ )eq( ratio<9,64>{} );
#if 0
	// TODO: scientific format support
	expect( 0.1e0_ )eq( ratio<1,10>{} );
	expect( 0x0.1p0_ )eq( ratio<1,16>{} );
#endif

	expect( 0b111111111111111111111111111111111111111111111111111111111111111_ )
		eq( ratio<INT64_MAX,1>{} );
	expect( 0777777777777777777777_ )eq( ratio<INT64_MAX,1>{} );
	expect( 9223372036854775807_ )eq( ratio<INT64_MAX,1>{} );
	expect( 0x7fffffffffffffff_ )eq( ratio<INT64_MAX,1>{} );
	expect( 0X7fFfFfFfFfFfFfFf_ )eq( ratio<INT64_MAX,1>{} );

#if 0
	// FIXME: leading zeroes may cause overflow
	// possible fix: trim them
	// a better one: dispatch on zero/non-zero digit before exponentiating the base
	expect( 0x0'0000'0000'0000'0000_ )eq( ratio<0,1>{} );
#endif

	using meta::root;
	// ^ The declaration works around gcc bug #67835 (or
	// #69283). For some reason pow doesn't trigger the
	// bug, despite having almost identical definition
	// and being used the same way.

	{
		constexpr auto a2 = square( ratio<3,1>{} );
		constexpr auto b2 = square( ratio<4,1>{} );
		cexpect( a2.num  ==  9 ); cexpect( a2.den  == 1 );
		cexpect( b2.num  == 16 ); cexpect( b2.den  == 1 );
		constexpr auto sum = a2 + b2;
		cexpect( sum.num == 25 ); cexpect( sum.den == 1 );
		constexpr auto rt = sqrt( sum );
		cexpect( rt.num  ==  5 ); cexpect( rt.den  == 1 );
	}
	expect( sqrt( square( 3_) + square( 4_) ) )eq( 5_);
	expect( sqrt( square( 1_/ 3_) + square( 1_/ 4_) ) )eq( 5_/ 12_);

	// greatest integral square that fits into 32-bit signed integer
	expect( sqrt( 2'147'395'600_) )eq( 46'340_);
	// greatest integral square that fits into 64-bit signed integer
	expect( sqrt( 9'223'372'030'926'249'001_) )eq( 3'037'000'499_);

	expect( sqrt( 2'147'441'940.25_) )eq( 46'340.5_);
	expect( sqrt( 2'147'423'404.09_) )eq( 46'340.3_);

	expect( pow( 1_, +1337_) )eq( 1_);
	expect( pow( 1_, -1337_) )eq( 1_);

	expect( pow( 4_, 0.5_) )eq( 2_);
	expect( pow( 4_, 1_/ 2_) )eq( 2_);
	expect( pow( 4_, 2_/ 4_) )eq( 2_);
	expect( pow( 0x10_, 1_/ 4_) )eq( 2_);
	expect( pow( 0x10_, 3_/ 4_) )eq( 0x8_);
	expect( pow(0x100_, 7_/ 8_) )eq( 0x80_);
	expect( pow(0x100_, 0.125_) )eq( 2_);
	expect( root( 1_, +(1_<< 62_)) )eq( 1_);
	expect( root( 1_, -(1_<< 62_)) )eq( 1_);
	expect( root(100_, 2_/ 1_) )eq( 10_);
	expect( root( 10_, 1_/ 2_) )eq(100_);

	expect( pow( 8_, 1_/ 3_) )eq( 2_);
	expect( pow( 1000_, 1_/ 3_) )eq( 10_);

	expect( root(1000_, 3_/ 1_) )eq(  10_);
	expect( root(  10_, 1_/ 3_) )eq(1000_);

	expect( pow( 512_, 1_/ 9_) )eq( 2_);
	expect( root( 512_, 9_) )eq( 2_);

	expect( pow( 64_, 1_/6_) )eq( 2_);
	expect( root( 64_, 6_) )eq( 2_);

	expect( root( 2_, -1_) )eq( 1_/2_);
	expect( root( 4_, -2_) )eq( 1_/2_);
	expect( root( 8_, -3_) )eq( 1_/2_);
}
