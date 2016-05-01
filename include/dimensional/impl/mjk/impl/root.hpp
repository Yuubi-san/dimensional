
#include "../integral_constant"
#include "../meta"
#include <cmath>
#include <cstdint>
#include <utility>

namespace mjk
{
	template<typename X>
	inline constexpr auto root( X &&x, intmax_constant<0> )
	{
		static_assert( !sizeof(x,0),
			"zeroth root is undefined" );
	}
	template<typename X>
	inline constexpr auto root( X &&x, intmax_constant<1> )
	{
		return std::forward<X>(x);
	}
	template<typename X, std::intmax_t I>
	inline constexpr auto root( X &&x, intmax_constant<I>,
		int_if< (I > 0) and I % 2 == 0 > = 0 )
	{
		using std::sqrt;
		return sqrt( root(std::forward<X>(x), intmax_constant<I/2>{}) );
	}
	template<typename X, std::intmax_t I>
	inline constexpr auto root( X &&x, intmax_constant<I>,
		int_if< (I > 0) and I % 3 == 0 and I % 2 > = 0 )
	{
		using std::cbrt;
		return cbrt( root(std::forward<X>(x), intmax_constant<I/3>{}) );
	}
	template<typename X, std::intmax_t I>
	inline constexpr auto root( X &&x, intmax_constant<I>,
		int_if< (I > 0) and I % 3 and I % 2 > = 0 )
	{
		static_assert( !sizeof(x,0),
			"not implemented" );
		// TODO: something to the effect of  sign(x) * pow( abs(x), 1/n )
	}
	template<typename X, std::intmax_t I>
	inline constexpr auto root( X &&x, intmax_constant<I>,
		int_if< (I < 0) > = 0 )
	{
		return
			make_one(x) /
			root( std::forward<X>(x), intmax_constant<-I>{} );
	}
}
