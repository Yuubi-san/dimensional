
#include <type_traits>

namespace mjk
{
	// signed greatest common divisor
	// the result may be negative if any of the arguments are negative
	template<typename T, typename U>
	inline constexpr std::common_type_t<T,U>
	sgcd( const T &dividend, const U &divisor )
	{
		return !divisor ?
			dividend
		:
			sgcd( divisor, dividend % divisor );
	}
}
