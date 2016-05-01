
#include "dimensional.hpp"
#include <chrono>

// Système International d’unités
namespace si
{
	using namespace dimensional::constant_literals;

	// ref: https://en.wikipedia.org/wiki/International_System_of_Units#Units_and_prefixes

	inline namespace dimen
	{
		using dimensional::dimension;
		constexpr dimension<struct _length_tag>      length{};
		constexpr dimension<struct _mass_tag>        mass{};
		constexpr dimension<struct _time_tag>        time{};
		constexpr dimension<struct _energy_tag>      energy{};
		constexpr dimension<struct _charge_tag>      charge{};
		constexpr dimension<struct _temperature_tag> temperature{};
		constexpr dimension<struct _substance_tag>   substance{};
		constexpr dimension<struct _luminous_intensity_tag> luminous_intensity{};

		constexpr auto current    = charge/time;
		constexpr auto potential  = energy/charge;
		constexpr auto power      = energy/time;
		constexpr auto volume     = length^3_;
		constexpr auto flow       = volume/time;
	}

	inline namespace unit
	{
		// base
		constexpr auto  m  = unit_of(length);
		constexpr auto kg  = unit_of(mass);
		constexpr auto  s  = unit_of(time);
		constexpr auto  A  = unit_of(current);
		constexpr auto  K  = unit_of(temperature);
		constexpr auto mol = unit_of(substance);
		constexpr auto cd  = unit_of(luminous_intensity);
		constexpr auto metre    = m;
		constexpr auto kilogram = kg;
		constexpr auto second   = s;
		constexpr auto ampere   = A;
		constexpr auto kelvin   = K;
		constexpr auto mole     = mol;
		constexpr auto candela  = cd;

		// derived
		constexpr auto rad = m/m,           radian = rad;
		constexpr auto  sr = (m^2_)/(m^2_), steradian = sr;
		constexpr auto  Hz = 1_/s,          hertz  = Hz;
		constexpr auto  N  = kg*m/(s^2_),   newton = N;
		constexpr auto  Pa = N/(m^2_),      pascal = Pa;
		constexpr auto  J  = N*m, joule   = J;
		constexpr auto  W  = J/s, watt    = W;
		constexpr auto  C  = s*A, coulomb = C;
		constexpr auto  V  = W/A, volt    = V;
		constexpr auto  F  = C/V, farad   = F;
		constexpr auto  O  = V/A, ohm     = O, \u03A9 = O;
		constexpr auto  S  = A/V, siemens = S;
		constexpr auto  Wb = V*s, weber   = Wb;
		constexpr auto  T  = Wb/(m^2_), tesla     = T;
		constexpr auto  H  = Wb/A,      henry     = H;
		constexpr auto degC= K,   degree_celsius  = degC;
		constexpr auto  lm = cd*sr,     lumen     = lm;
		constexpr auto  lx = lm/(m^2_), lux       = lx;
		constexpr auto  Bq = 1_/s,      becquerel = Bq;
		constexpr auto  Gy = J/kg,      gray      = Gy;
		constexpr auto  Sv = J/kg,      sievert   = Sv;
		constexpr auto kat = mol/s,     katal     = kat;

		// convenience stuff
		constexpr auto  g  = 1_/1000_*kg, gram = g;
	}

	inline namespace prefix
	{
		constexpr auto da=                                10_,  deca = da;
		constexpr auto h =                               100_, hecto = h;
		constexpr auto k =                             1'000_,  kilo = k;
		constexpr auto M =                         1'000'000_,  mega = M;
		constexpr auto G =                     1'000'000'000_,  giga = G;
		constexpr auto T =                 1'000'000'000'000_,  tera = T;
		constexpr auto P =             1'000'000'000'000'000_,  peta = P;
		constexpr auto E =         1'000'000'000'000'000'000_,   exa = E;
	#if INTMAX_MAX > INT64_MAX
		constexpr auto Z =     1'000'000'000'000'000'000'000_, zetta = Z;
		constexpr auto Y = 1'000'000'000'000'000'000'000'000_, yotta = Y;
	#endif

		constexpr auto d = 1_/da, deci = d;
		constexpr auto c = 1_/h, centi = c;
		constexpr auto m = 1_/k, milli = m;
		constexpr auto u = 1_/M, micro = u, \u03BC = u;
		constexpr auto n = 1_/G,  nano = n;
		constexpr auto p = 1_/T,  pico = p;
		constexpr auto f = 1_/P, femto = f;
		constexpr auto a = 1_/E,  atto = a;
	#if INTMAX_MAX > INT64_MAX
		constexpr auto z = 1_/Z, zepto = z;
		constexpr auto y = 1_/Y, yocto = y;
		// do want namespace templates
		// or better: integral_constant of unbounded precision
	#endif
	}

	namespace impl
	{
		using time = decltype(+si::time);
	}
}


namespace mjk
{
	// time quantity -> std::chrono::duration converter
	template
	<
		typename T, typename Scale,
		typename Rep, typename Period
	>
	struct conversion
	<
		dimensional::quantity< T,
			dimensional::unit<si::impl::time, Scale> >,
		std::chrono::duration< Rep, Period >
	>
	{
		using qty = dimensional::quantity< T,
		            	dimensional::unit<si::impl::time, Scale> >;
		using dur = std::chrono::duration< Rep, Period >;

		constexpr dur operator()( const qty &q ) const
		{
			using p = typename dur::period;
			using scale = dimensional::constant<p::num,p::den>;
			return dur{Rep( q.to( scale{} ).count() )};
		}
	};

	// time quantity <- std::chrono::duration converter
	template
	<
		typename T, typename Scale,
		typename Rep, typename Period
	>
	struct conversion
	<
		std::chrono::duration< Rep, Period >,
		dimensional::quantity< T,
			dimensional::unit<si::impl::time, Scale> >
	>
	{
		using qty = dimensional::quantity< T,
		            	dimensional::unit<si::impl::time, Scale> >;
		using dur = std::chrono::duration< Rep, Period >;

		constexpr qty operator()( const dur &d ) const
		{
			using p = typename dur::period;
			using scale = dimensional::constant<p::num,p::den>;
			return d.count() * (scale{} * si::second);
		}
	};
}
