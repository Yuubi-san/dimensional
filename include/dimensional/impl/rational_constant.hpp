
#ifndef DIMENSIONAL_IMPL_RATIONAL_CONSTANT_H
#define DIMENSIONAL_IMPL_RATIONAL_CONSTANT_H

#include "meta.hpp"
#include "mjk/fun"
#include "mjk/integral_constant"
#include "mjk/math"
#include <cstdint>
#include <ratio>


namespace meta
{
	using std::intmax_t;
	using mjk::intmax_constant;
	using mjk::char_constant;

	namespace impl
	{
		inline constexpr intmax_t sign( intmax_t x )
		{ return x < 0 ? -1 : +1; }

		// std::abs isn't guaranteed to detect overflow in constant expressions
		inline constexpr intmax_t abs( intmax_t x )
		{ return x < 0 ? -x : +x; }
	}


	template<
		intmax_t Num = 0,
		intmax_t Den = 1>
	struct rational_constant
	{
		static constexpr intmax_constant<Num> num{};
		static constexpr intmax_constant<Den> den{};

		static_assert( den, "bad denominator" );
		static_assert( den > 0 && impl::abs(mjk::sgcd(Num,Den)) == 1,
			"rational_constant must be constructed in reduced form. "
			"use make_rational_constant to reduce automatically" );

		static constexpr std::ratio<num,den> to_ratio() { return {}; }

		constexpr operator intmax_t() const
		{
			static_assert( den == 1,
				"this rational_constant is not integral. "
				"divide num by den manually to truncate" );
			return num.value;
		}
	};

	template<intmax_t Num = 0, intmax_t Den = 1>
	inline constexpr auto make_rational_constant(
		intmax_constant<Num> num = {},
		intmax_constant<Den> den = {} )
	{
		static_assert( den, "dbz" );
		using gcd = intmax_constant< impl::abs(mjk::sgcd(Num,Den)) >;
		using n   = intmax_constant< impl::sign(den)*num/gcd{} >;
		using d   = intmax_constant< impl::abs(den/gcd{}) >;
		return rational_constant< n{}, d{} >{};
	}

	template<typename Ratio>
	inline constexpr auto make_rational_constant( Ratio r = {} )
	{
		return rational_constant<r.num, r.den>{};
	}


	// this is mostly an implementation detail now and should soon be zapped
	template<intmax_t Num = 0, intmax_t Den = 1>
	using ratio =
		rational_constant<Num, Den>;
	//	rational_constant<intmax_constant<Num>, intmax_constant<Den>>;
	//   Ugh. Typifying the params casues ICE in g++ 4.9.0
	// inside mjk::power. Time to upgrade...
}

namespace mjk
{
	template<intmax_t Num, intmax_t Den>
	struct zero< meta::ratio<Num, Den> >
	{ constexpr auto operator()() const { return meta::ratio<0>{}; } };
	template<intmax_t Num, intmax_t Den>
	struct one<  meta::ratio<Num, Den> >
	{ constexpr auto operator()() const { return meta::ratio<1>{}; } };
}

namespace meta
{
	// +ratio
	template<intmax_t Num, intmax_t Den>
	inline constexpr auto operator+( ratio<Num,Den> r )
	{ return r; }
	// -ratio
	template<intmax_t Num, intmax_t Den>
	inline constexpr auto operator-( ratio<Num,Den> )
	{ return ratio<-Num, Den>{}; }
	// ratio + ratio
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator+( ratio<NumA,DenA> a, ratio<NumB,DenB> b )
	{
		using r = std::ratio_add< decltype(a), decltype(b) >;
		return ratio< r::num, r::den >{};
	}
	// ratio * ratio
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator*( ratio<NumA,DenA> a, ratio<NumB,DenB> b )
	{
		using r = std::ratio_multiply< decltype(a), decltype(b) >;
		return ratio< r::num, r::den >{};
	}
	// ratio / ratio
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator/( ratio<NumA,DenA> a, ratio<NumB,DenB> b )
	{
		using r = std::ratio_divide< decltype(a), decltype(b) >;
		return ratio< r::num, r::den >{};
	}
	// ratio << int
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator<<( ratio<NumA,DenA> a, ratio<NumB,DenB> sh )
	{
		static_assert( sh.den == 1,
			"non-integral shift" );
		return a * ratio< (intmax_t{1} << sh.num) >{};
	}
	// ratio == ratio
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator==( ratio<NumA,DenA> a, ratio<NumB,DenB> b )
	{ return std::ratio_equal< decltype(a), decltype(b) >{}; }
	// ratio < ratio
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto operator< ( ratio<NumA,DenA> a, ratio<NumB,DenB> b )
	{ return std::ratio_less < decltype(a), decltype(b) >{}; }
	// sqrt(ratio)
	template<intmax_t Num, intmax_t Den>
	inline constexpr auto sqrt( ratio<Num,Den> )
	{
		using LD = long double;
		using root = ratio
		<
			static_cast<intmax_t>(std::sqrt( LD{Num} )),
			static_cast<intmax_t>(std::sqrt( LD{Den} ))
		>;
		static_assert( mjk::square(root{}) == ratio<Num,Den>{},
			"inexact square root" );
		return root{};
	}
	// cbrt(ratio)
	template<intmax_t Num, intmax_t Den>
	inline constexpr auto cbrt( ratio<Num,Den> )
	{
		using LD = long double;
		using root = ratio
		<
			static_cast<intmax_t>(std::cbrt( LD{Num} )),
			static_cast<intmax_t>(std::cbrt( LD{Den} ))
		>;
		static_assert( mjk::cube(root{}) == ratio<Num,Den>{},
			"inexact cube root" );
		return root{};
	}
	// pow(ratio, ratio)
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto pow( ratio<NumA,DenA> b, ratio<NumB,DenB> n )
	{
		return mjk::root( mjk::pow(b, n.num), n.den );
	}
	// root(ratio, ratio)
	template<intmax_t NumA, intmax_t DenA, intmax_t NumB, intmax_t DenB>
	inline constexpr auto root( ratio<NumA,DenA> r, ratio<NumB,DenB> i )
	{
		return meta::pow( r, ratio<1>{}/i );
	}
	// PHILOSORAPTOR: can and sholud pow/root be
	// generalized for any type of base/radicand?


	template<intmax_t Num, intmax_t Den>
	inline constexpr auto isnan( ratio<Num,Den> ) { return mjk::false_type{}; }


	template<typename Stream, intmax_t Num, intmax_t Den>
	inline Stream &operator<<( Stream &s, ratio<Num,Den> r )
	{
		return s << r.num.value <<'/'<< r.den.value;
	}


	namespace impl
	{

		template<typename>
		struct lower;
		template<char c>
		struct lower< char_constant<c> >
		{
			using type = char_constant< (c >= 'A' && c <= 'Z') ? c+('a'-'A') : c >;
		};

		template<typename Digit, typename Base>
		constexpr auto parse_digit( Digit d, Base )
		{
			static_assert(
				(Digit{} >= '0' && Digit{} <= '9') ||
				(Digit{} >= 'a' && Digit{} < 'a'+Base{}-10),
				// FIXME:                  ^  assuming signed overflow does not occur when reducing constant in comparison [-Wstrict-overflow]
				"digit out of range" );
			return ratio< (d <= '9') ? (d - '0') : (d - 'a' + 10) >{};
		}

		template<typename... Digits, std::size_t... Powers, typename Base>
		constexpr auto parse_digits_2
		(
			sequence< Digits... >,
			std::index_sequence< Powers... >,
			Base base
		)
		{
			return (void)base, mjk::sum
			(
				ratio<>{},
				( parse_digit(Digits{}, base) * mjk::power<Powers>{}(base) )...
			);
		}
		template<unsigned Base, typename... Digits>
		constexpr auto parse_digits( sequence<Digits...> dig )
		{
			return parse_digits_2( dig,
				mjk::make_integer_range< std::size_t, sizeof...(Digits), 0 >{},
				ratio<Base>{} );
		}

		template<unsigned Base, typename number>
		constexpr auto parse_number( number )
		{
			static_assert(
				!seq::find< number, char_constant<'p'> >::value &&
				!seq::find< number, char_constant<'e'> >::value,
				"scientific format not supported yet" );
			using dot = char_constant<'.'>;
			using found  = seq::find  < number, dot >;
			using digits = seq::remove< number, dot >;
			return
				parse_digits<Base>( digits{} ) /
				mjk::pow( ratio<Base>{}, found::tail_size );
		}

		template<typename Head, typename... Tail>
		constexpr auto parse_literal_2( Head, sequence<Tail...> )
		{ return parse_number<8>( sequence<Head,Tail...>{} ); }
		template<typename Head, typename... Tail>
		constexpr auto parse_literal_2( char_constant<'.'>, sequence<Head,Tail...> )
		{ return parse_number<10>( sequence<char_constant<'.'>,Head,Tail...>{} ); }
		template<typename Head, typename... Tail>
		constexpr auto parse_literal_2( char_constant<'x'>, sequence<Head,Tail...> n )
		{ return parse_number<16>( n ); }
		template<typename Head, typename... Tail>
		constexpr auto parse_literal_2( char_constant<'b'>, sequence<Head,Tail...> n )
		{ return parse_number<2>( n ); }

		template<typename Head, typename... Tail>
		constexpr auto parse_literal( Head, sequence<Tail...> )
		{ return parse_number<10>( sequence<Head,Tail...>{} ); }
		template<typename Head, typename... Tail>
		constexpr auto parse_literal( char_constant<'0'>, sequence<Head,Tail...> )
		{ return parse_literal_2( Head{}, sequence<Tail...>{} ); }
		constexpr auto parse_literal( char_constant<'0'>, sequence<> )
		{ return ratio<>{}; }
	}

	template<char Head, char... Tail>
	constexpr auto parse_ratio()
	{
		using tail = sequence< char_constant<Tail>... >;
		return impl::parse_literal(
			char_constant<Head>{},
			seq::transform< seq::remove<tail, char_constant<'\''>>, impl::lower>{}
		);
	}


	namespace rational_constant_literals
	{
		template<char... Chars>
		constexpr auto operator""_()
		{
			return parse_ratio<Chars...>();
		}
	}
}

#endif
