
#include "../include/dimensional/si.hpp"

#include "test.hpp"
test
{
	using namespace si;
	using namespace std::chrono;
	using dimensional::constant;

	cexpect( power*si::time == energy );

	{
		constexpr auto kgf = 9.8_*kg*metre/(s^2_);
		cexpect( kgf.scale == constant<49,5>{} );
		cexpect( kgf.dimension == mass*length/(si::time^2_) );
	}

	{
		constexpr auto ns = nano*s;

		{
			setup( nanoseconds res = 0*ns );
			expect( res.count() )eq( 0 );
		}
		{
			setup( nanoseconds res = 0.0*ns );
			expect( res.count() )eq( 0 );
		}

		{
			setup( seconds res = 1337*s );
			expect( res.count() )eq( 1337 );
		}
		{
			setup( nanoseconds res = 1729*ns );
			expect( res.count() )eq( 1729 );
		}

		{
			setup( seconds res = 2*int{giga} * ns );
			expect( res.count() )eq( 2 );
		}
		{
			setup( minutes res = 61*s );
			expect( res.count() )eq( 1 );
		}


		using ins = decltype(intmax_t{}*ns);
		using is  = decltype(intmax_t{}*s);

		{
			setup( ins res = nanoseconds{0} );
			expect( res.count() )eq( 0 );
		}
#if 1
		using dns = decltype(double{}*ns);
		{
			setup( dns res = nanoseconds{0} );
			expect( res.count() )eq( 0.0 );
		}
#endif

		{
			setup( is res = seconds{1337} );
			expect( res.count() )eq( 1337 );
		}
		{
			setup( ins res = nanoseconds{1729} );
			expect( res.count() )eq( 1729 );
		}

		{
			setup( ins res = seconds{2} );
			expect( res.count() )eq( 2*giga );
		}
		{
			setup( is res = minutes{1} );
			expect( res.count() )eq( 60 );
		}
	}
}
