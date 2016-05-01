
#include <type_traits>

namespace mjk
{
	// additive identity maker
	// to be user-specialized
	template<typename T>
	struct zero
	{
		constexpr T operator()() const { return T(); }
	};

	// returns additive identity of the argument's type
	template<typename T>
	inline constexpr decltype(auto)
	make_zero( T &&/*ref_val*/ )
	{ return zero<std::decay_t<T>>{}(); }


	// multiplicative identity maker
	// to be user-specialized
	template<typename T>
	struct one
	{
		constexpr T operator()() const { return T{1}; }
	};

	// returns multiplicative identity of the argument's type
	template<typename T>
	inline constexpr decltype(auto)
	make_one( T &&/*ref_val*/ )
	{ return one<std::decay_t<T>>{}(); }
}
