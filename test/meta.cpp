
#include "../include/dimensional/impl/meta.hpp"

int main()
{
	using namespace meta;

	static_assert( is_same< decltype(not_(true_type{})), false_type >{}, "" );
	static_assert( is_same< decltype(not_(false_type{})), true_type >{}, "" );

	static_assert( all<true >::value == true,  "" );
	static_assert( all<false>::value == false, "" );

	static_assert(  seq::is_uset< sequence<int> >{}, "" );
	static_assert( !seq::is_uset< sequence<int,int> >{}, "" );
	static_assert(  seq::is_uset< sequence<int,long,void,volatile char *const &> >{}, "" );
	static_assert( !seq::is_uset< sequence<int,long,void,int,volatile char *const &> >{}, "" );

	static_assert( uset::equal< set<int>, set<int> >{}, "" );
	static_assert( uset::equal<
		set<int,long,void,volatile char *const &>,
		set<long,volatile char *const &,void,int> >{}, "" );
}
