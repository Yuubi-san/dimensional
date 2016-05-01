
#include "../fun"
#include "../integral_constant"
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace mjk
{
	template<typename T>
	inline constexpr decltype(auto)
	square( T &&val )
	{
		return std::forward<T>(val) * val;
	}


	template<typename T>
	inline constexpr decltype(auto)
	cube( T &&val )
	{
		return square( std::forward<T>(val) ) * val;
	}


	namespace impl
	{
		using std::intmax_t;
		using std::forward;

		template
		<
			intmax_t N,
			typename Negativeness = bool_constant< (N < 0) >,
			typename Oddness      = bool_constant< (N % 2) >
		>
		struct power;

		using negative    = true_type;
		using nonnegative = false_type;
		using odd         = true_type;
		using even        = false_type;

		template<intmax_t N>
		struct power<N, nonnegative, odd>
		{
			static constexpr auto exponent = N;
			template<typename X>
			constexpr decltype(auto)
			operator()( X &&x ) const
			{
				return forward<X>(x) * power<N-1>{}(x);
			}
		};

		template<intmax_t N>
		struct power<N, nonnegative, even>
		{
			static constexpr auto exponent = N;
			template<typename X>
			constexpr decltype(auto)
			operator()( X &&x ) const
			{
				return square( power<N/2>{}( forward<X>(x) ) );
			}
		};

		template<>
		struct power<1>
		{
			static constexpr auto exponent = 1;
			template<typename X>
			constexpr decltype(auto)
			operator()( X &&x ) const
			{
				return forward<X>(x);
			}
		};

		template<>
		struct power<0>
		{
			static constexpr auto exponent = 0;
			template<typename X>
			constexpr decltype(auto)
			operator()( X &&x ) const
			{
				using std::isnan;
				return ternary( isnan(x), x, make_one(x) );
			}
		};

		template<intmax_t N>
		struct power<N, negative>
		{
			static constexpr auto exponent = N;
			template<typename X>
			constexpr decltype(auto)
			operator()( X &&x ) const
			{
				return make_one(x) / power<-N>{}(x);
			}
		};
	}


	template<std::intmax_t N>
	using power = impl::power<N>;


	template<typename T, typename I, I N>
	inline constexpr decltype(auto)
	pow( T &&x, std::integral_constant<I,N> )
	{
		return power< std::intmax_t{N} >{}( std::forward<T>(x) );
	}
}
