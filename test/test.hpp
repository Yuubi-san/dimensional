
#ifndef WHAT_AM_I_DOING_H
#define WHAT_AM_I_DOING_H

#include <iostream>
#include <utility>
#include <type_traits>


template<typename L>
struct testerR
{
	template<typename>
	struct void_ { using type = void; };

	template<typename T>
	using emvoiden = typename void_<T>::type;

	template<typename T, typename = void>
	struct printable : std::false_type {};

	template<typename T>
	struct printable<T, emvoiden<
		decltype( std::declval<std::ostream &>() << std::declval<T>() )
	>> : std::true_type {};


	int (&count)[2];
	L l;
	const char *l_str;
	template<typename R>
	void eq( R &&r, const char *r_str )
	{
		using std::cerr;
		const bool cond = l == std::forward<R>(r);
		++count[cond];
		cerr<<' '<<(cond?' ':'X')<<' '<<' ';
		if ( cond )
			cerr<< l_str <<" \tis  "<< r_str;
		else
		{
			cerr<< l_str <<"  expected to be  "<< r_str <<", instead is  ";
			print( cerr, l, printable<L>{} );
		}
		cerr <<'\n';
	}

	template<typename T>
	std::ostream &print( std::ostream &s, const T &obj, std::true_type )
	{
		return s << obj;
	}
	template<typename T>
	std::ostream &print( std::ostream &s, const T &, std::false_type )
	{
		return s << "*something unprintable*";
	}
};

struct testerL
{
	int (&count)[2];
	template<typename L>
	auto operator()( L &&l, const char *l_str )
	{
		return testerR< std::decay_t<L> >{ count, std::forward<L>(l), l_str };
	}
};

#define test \
	template<typename F> \
	void test( F ); \
	int main() \
	{ \
		int count[] = {0,0}; \
		test( testerL{count} ); \
		\
		std::cerr << '\n'; \
		if ( count[false] ) \
			std::cerr << count[false] <<" of "<< (count[false]+count[true]) << " tests failed\n"; \
		else \
			std::cerr <<"all "<< count[true] << " tests OK\n"; \
		\
		return count[false]; \
	} \
	template<typename F> \
	void test( F expect )

#define setup( statement ) std::cerr << #statement << ";\n"; statement
#define expect( ... ) expect( (__VA_ARGS__), #__VA_ARGS__ )
#define eq( ... )   .eq( (__VA_ARGS__), #__VA_ARGS__ )
#define cexpect( ... ) static_assert( (__VA_ARGS__), #__VA_ARGS__ )

#endif
