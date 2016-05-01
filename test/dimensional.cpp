
#include "../include/dimensional/dimensional.hpp"


// type name printing
#include <typeinfo>
#include <cxxabi.h>
#include <memory>
#include <new>
template<typename Stream, typename T>
inline Stream &operator<<( Stream &s, meta::type<T> t )
{
	int status;
	const char *const mangled = typeid( t ).name();
	char *const demangled = abi::__cxa_demangle( mangled, nullptr, 0, &status );
	std::unique_ptr<char, decltype(&std::free)> p{ demangled, &std::free };
	switch ( status )
	{
		case  0: return s << demangled;
		case -1: throw std::bad_alloc{};
		case -2:
		case -3:
		default: return s << mangled;
	}
}

// HACK: To placate linker, we're defining here some static members used
// in non-constant expressions. The issue will be resolved either at the
// library API or at the testing framework level.
namespace dimensional
{
	template<typename Tag, typename Power>
	constexpr meta::type<Tag> dimension_factor<Tag,Power>::tag;
	template<typename Tag, typename Power>
	constexpr Power           dimension_factor<Tag,Power>::power;

	template<typename T, typename Dim, typename Scale>
	constexpr meta::type<T>   quantity<T,unit<Dim,Scale>>::type;
	template<typename T, typename Dim, typename Scale>
	constexpr unit<Dim,Scale> quantity<T,unit<Dim,Scale>>::unit;
}		


#include "test.hpp"		
test
{
	using namespace dimensional;
	using namespace dimensional::constant_literals;
	using namespace meta;

	{
		struct tag;

		{
			setup( constexpr auto res = dimension<tag>{} );
			expect( seq::size_f(res.factors) )eq( 1_ );
			expect( seq::front_f(res.factors).get().tag )eq( type<tag>{} );
			expect( seq::front_f(res.factors).get().power )eq( 1_ );
		}

		{
			setup( constexpr auto d = dimension<tag>{} );
			setup( constexpr auto res = d*d );
			expect( seq::size_f(res.factors) )eq( 1_ );
			expect( seq::front_f(res.factors).get().tag )eq( type<tag>{} );
			expect( seq::front_f(res.factors).get().power )eq( 2_ );
		}
	}

	{
		constexpr auto kg = unit_of( dimension<struct mass>{} );
		constexpr auto  g = 1_/ 1'000_* kg;
		constexpr auto  t = 1'000_* kg;
		constexpr auto kt = 1'000_* t;

		constexpr auto  m = unit_of( dimension<struct length>{} );
		constexpr auto dm = 1_/ 10_* m;
		constexpr auto cm = 1_/ 100_* m;

		{
			setup( constexpr auto res = 2*g * 2*g );
			expect( res.count() )eq( 4 );
			expect( res.unit )eq( g*g );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 2*t * 2*t );
			expect( res.count() )eq( 4 );
			expect( res.unit )eq( t*t );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 2*g * 2*kg );
			expect( res.count() )eq( 4 );
			expect( res.unit )eq( g*kg );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 2*kg * 2*t );
			expect( res.count() )eq( 4 );
			expect( res.unit )eq( kg*t );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 2*cm * 2*m );
			expect( res.count() )eq( 4 );
			expect( res.unit )eq( dm*dm );
			expect( res.type )eq( int_ );
		}

		{
			setup( constexpr auto res = 1*kg + 1*kg );
			expect( res.count() )eq( 2 );
			expect( res.unit )eq( kg );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 1*kg - 1*kg );
			expect( res.count() )eq( 0 );
			expect( res.unit )eq( kg );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 1*kg - 1*g );
			expect( res.count() )eq( 999 );
			expect( res.unit )eq( g );
			expect( res.type )eq( int_ );
		}
		{
			setup( constexpr auto res = 1.0*kg - 1*g );
			expect( res.count() )eq( 999.0 );
			expect( res.unit )eq( g );
			expect( res.type )eq( double_ );
		}
		{
			setup( auto res = 1*kg );
			setup( res -= 1*g );
			expect( res.count() )eq( 0 );
		}
		{
			setup( auto res = 1.0*kg );
			setup( res -= 1*g );
			expect( res.count() )eq( 1.0-(1.0/1'000) );
			expect( res.unit )eq( kg );
			expect( res.type )eq( double_ );
		}
		{
			setup( constexpr auto res = 1*kt - 1*t );
			expect( res.count() )eq( 999 );
			expect( res.unit )eq( t );
			expect( res.type )eq( int_ );
		}

		{
			constexpr auto mm = 1_/ 1'000_* m;
			constexpr auto in = 25.4_* mm;
			setup( constexpr auto res = 1*mm + 1*in );
			expect( res.count() )eq( 132 );
			expect( res.unit )eq( 1_/ 5_*mm );
			expect( res.type )eq( int_ );
		}

		{
			setup();
			expect( 1 *kg )eq( 1000 *g );
			expect( 1.*kg )eq( 1000 *g );
			expect( 1 *kg )eq( 1000.*g );
			expect( 1.*kg )eq( 1000.*g );
		}
	}
}
