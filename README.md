
`dimensional.hpp`
=================

A (proof‐of‐concept) toolkit for dimensional analysis in modern C++.


Intro
-----

If that description made you go <i><q>Dimensional
analysis? But I’m not launching any probes to orbit distant planets any time
this life</q></i>, then good! That means the following code won’t offend your
sense of mathematical elegance (too much). So here, have a mashup of some
mundane programming we all do on a day‐to‐day basis…

```C++
#include <dimensional/dimensional.hpp>
#include <dimensional/si.hpp>
#include <cassert>

auto ad_hoc()
{
	using dimensional::dimension;
	using dimensional::dimensionless;
	using namespace dimensional::constant_literals;

	constexpr auto information = dimension<struct information_tag>{};
	constexpr auto bit   = unit_of(information), b = bit;
	constexpr auto octet = 8_*bit, byte = octet, B = byte;
	static_assert( octet.dimension == bit.dimension, "" );

	constexpr auto kibi = 1_ << 10_, Ki = kibi;
	constexpr auto mebi = 1_ << 20_, Mi = mebi;
	using si::k;
	using si::M;
	using si::s;

	const auto cluster_size  = 4u*(Ki*B);
	const auto database_size = 42ull*(Mi*B);
	const auto clusters_in_database = database_size / cluster_size;
	static_assert( clusters_in_database.dimension == dimensionless, "" );
	assert( clusters_in_database.count() == 42*256ull );
	constexpr auto Mbps = M*b/s;
	auto throughput = 10.24*Mbps;
	constexpr auto bandwidth = information/si::time;
	constexpr auto bps = unit_of(bandwidth);
	static_assert( bps == b/s, "" );
	throughput -= 240*(k*bps);
	assert( throughput == 10*Mbps );

	constexpr auto virtual_length = dimension<struct virtual_length_tag>{};
	constexpr auto pixel = unit_of(virtual_length), px = pixel;
	constexpr auto inch  = 0.0254_*si::metre,       in = inch;
	constexpr auto point = 1_/72_*in,               pt = point;
	constexpr auto ppi = px/in;
	constexpr auto dpi = 1_/in;
	auto fallacy = 96.*ppi;
	auto imperial_blots = 300.L*dpi;
	return imperial_blots/fallacy;
}
```

Now that the reader is sufficiently overwhelmed by the outrageous expressive
power, I’ll go through this nonsense at a slower pace, describing line‐by‐line
what the heck is happening. If you feel confident in your understanding of most
of this code, however, feel free to skip to the explanation of the parts you
feel interested in. Or [skip the entire section](#more-concise-examples)
altogether.


### Long‐winded comments entangled with design discussion <small>(sorry)</small>

First of all, those little “tails” you see behind some of the numbers are
[UDL][]s that turn numbers into _types_. For example, that `8_` for octets
turns into an instance of `dimensional::​constant<8>` or, for inches, into
`constant<​127, 5'000>`. Yes, **constants** are reduced rationals, much like
`std::ratio`. Unlike `std::ratio`, however, constants have the usual arithmetic
defined: that `1_ << 10_` gives `kibi` the type `constant<​1024>` and, e.g.,
`decltype(​cbrt(27_/​1331_))` is exactly `constant<​3, 11>`.

Not to get sidetracked by the awesomenesses of compile‐time rational
arithmetic–and skipping some boilerplate–let us begin with the actual
code at hand:

		constexpr auto information = dimension<struct information_tag>{};

This decidedly not‐concise‐enough statement–which may one happy day become
`make_dimension([]{})` or even `make_dimension()`–simply creates a unique
**dimension** type and an instance thereof. It’s unique only by virtue of a
local (incomplete) struct that is used as a tag. If this was at a named
namespace scope, the struct would also become part of that namespace, making the
resulting dimension type global. And yes, you _can_ declare classes/structs
in‐line, which probably isn’t news to every C programmer in existence, but is
something I learned after, like, eight years of C++.

By the way, `constexpr` there isn’t really necessary. Since all the computations
are done at type level, there should be not a single code instruction generated
by any sane compiler. Assuming perfect inlining behavior, that is. And since
there is no standard way to guarantee it, it is advisable to always say
`constexpr`: whether it may be(come) a performance bottleneck or not, whether
it’s function level, namespace level or whatever, whether you care for [ODR][]
violation on not, constexpr is just a good habit to go with constants,
dimensions, and…

		constexpr auto bit   = unit_of(information), b = bit;

…**units**. Which get born out of dimensions and are, again, types stored–and
manipulated–in the form of their instances. The `T x = :::, y = x;` syntax
should make it self‐evident that `b` here is simply another name for `bit`,
carrying the same type and, thus, representing the same unit. But what would
happen if we’d multiplied it by a constant?

		constexpr auto octet = 8_*bit, byte = octet, B = byte;
		static_assert( octet.dimension == bit.dimension, "" );

Naturally, that means scaling, but `octet` is not simply a quantity “eight” with
the unit “bits”, it is the actual octet: the unit that is eight times the size
of bit.

When units interact, their individual scale factors don’t mean anything, it is
_their ratio_ that matters. Thus, we could have just as well chosen the octet as
our “fundamental” unit of information, and derived the bit from it using the
factor `1_/8_`. By the way, `octet/8_` wouldn’t pass, and that’s by current
design: we don’t say “octet over eight”, but “one eighth of octet”. Immediately
following the primary definition is an ambiguous–but good enough for this
context–`byte` alias, along with its symbol: `B`.

		constexpr auto kibi = 1_ << 10_, Ki = kibi;
		constexpr auto mebi = 1_ << 20_, Mi = mebi;
		using si::k;
		using si::M;
		using si::s;

Defining a couple of named constants to be used as prefixes and importing some
of the already defined SI ones–for kilo and mega–plus the symbol for the unit of
time.

		const auto cluster_size  = 4u*(Ki*B);
		const auto database_size = 42ull*(Mi*B);

Time for more magic numbers that assume things about the execution environment!
Can’t have too many of these. But as you probably noticed, those two are just
`unsigned` and `unsigned long long`, they don’t hold the value as part of their
type and as such don’t qualify as constants by our standards. Naturally,
multiplying by one of them won’t change the unit’s scale, unlike `Ki*B`, which
produces “kibibyte” unit out of “byte” one. So, enough suspense, what’s the
result? It’s the thing we’re all gathered here for and will be interacting with
the most: **quantity**–a “unitized” value for _unit‐safe programming_.
`cluster_size` has value 4 of type `unsigned` and has unit “kibibyte” aka “eight
kibibit” (which consists of dimension “information” and scale 8192). And no,
I won’t spell the actual types of those. Not because they’re particularly scary,
but mostly because I’d like to keep users away from spelling them, too. Consider
types an implementation detail, at least until the library has stabilized.

<i><q>Then how do I…</q></i>

	constexpr auto length = dimension< struct length_tag >{};
	constexpr auto parsec = unit_of( length ),  pc = parsec;
	using distance_qty = decltype( num::astro{} * pc );
	
	tril ftl::engine::jump_fwd( const distance_qty &d )
	{
		return maybe;  // TODO
	}

<i><q>Oh.</q></i>

That’s the short answer, but let us follow that train of thought a bit further.
Continued:

	using parsec_t = decltype(+pc);
	using length_t = decltype(+length);

Think of the unary plus here as short for `decay`: so that you don’t end up with
a cv‐qualified or reference type. In fact, any function returning by value
effectively “decays” the type as a bonus, so no need for it in these cases as
well: `decltype(k*W*h)`, `decltype(sqrt(acre))`.

Going deeper. Say you wanted to constrain only the unit of a function argument
and leave the data type unconstrained. The usual way would be resorting to
coupling with a concrete template name:

	template<typename T>
	tril jump_fwd( const quantity<T, parsec_t> &dist );

But actually… why limit the scale of the unit? We probably just want a distance,
_any distance_. And with the usual way of doing things this only leads us deeper
into coupling:

	template<typename T, typename Scale>
	tril jump_fwd( const quantity<T, unit<Scale, length_t>> &dist );

Still, there is something more sinister lurking in code like the above examples,
waiting for the unsuspecting programmer to discover at the least opportune time.
Thing is, there is more than one way to represent a dimension as a _type_ if
said dimension is a product of two or more fundamental dimensions. For example,
`si::force`, being a product of three fundamental dimensions (raised to
appropriate powers), has <span style='font-family:serif'>3! = 6</span> possible
representations, all equivalent from the point of view of dimensional analysis,
but different from the point of view of C++ type system: templates don’t have a
notion of parameter _sets_, only parameter _sequences_, so sets of types can
only be emulated as sorted sequences of types. I have yet to research
compile‐time type collation thoroughly (RTTI is just that: it’s _run‐time_), so
for now no sorting. To illustrate:

	auto m = 1*kg;
	auto a = 1*(metre/s/s);
	auto F₁ = m*a;
	auto F₂ = a*m;
	static_assert( F₁.dimension == F₂.dimension, "" );
	static_assert( not std::is_same<decltype(F₁), decltype(F₂)>{}, "" );

The <span style='font-family:serif'>{mass¹, length¹, time⁻²}</span> set is
represented differently from the <span style='font-family:serif'>{length¹,
time⁻², mass¹}</span> set, even though it’s the same set. (BTW,
<span style='font-family:serif'>dim²</span> and
<span style='font-family:serif'>dim<sup>⁻⁴⁄₋₂</sup></span> _do_
have the same type thanks to guaranteed ratio reduction, as do `254_/​10'000_*m`
and `127_/​5'000_*m` units.)

To a traditionally‐minded C++ programmer this may seem like a very serious
issue, but this is not necessarily the case, even in absence of CTTI niceties.
There is more general, overarching problem here: the whole deduction approach.
Why limit `jump_fwd` to accepting only instances of one chosen template
specialized on another chosen template? This forces a user of our FTL engine to
either (1) stick with the particular quantity and unit classes from the library
of our choosing, (2) publicly inherit their own classes from said library’s
classes, or (3) define conversions from the classes of their choosing and enjoy
completely redundant explicit casting everywhere. In all this “deduction hell”,
adding to the mix one particular representation of a dimension doesn’t seem like
a major villain. Still, what do?

To start with, let us go back to the basics, eliminating the intricate deduction
and simply accepting everything on the outside, while keeping things reasonably
user‐friendly with manual error checking on the inside:

	template<typename Distance>
	tril jump_fwd( const Distance &d )
	{
		static_assert( d.dimension == length, "" );
		:::
	}

Of course that doesn’t quite lend itself to overloading, we’d need to handle
_that_ manually too (with tag dispatching or whatnot), which, of course, isn’t
terribly suited for ad‐hoc extension, possibly by users. Besides, we’re talking
_modern_ C++ here. We have SFINAE, we have `enable_if`, wheee!..

	int int_if( std::true_type );
	
	template<typename Parsecs>
	tril jump_fwd( const Parsecs &dist, decltype(int_if(dist.unit == pc)) = 0 );
	// or, less constrained:
	template<typename Distance>
	tril jump_fwd(const Distance &d, decltype(int_if(d.dimension == length))=0);

Ugh. This is almost not horrible, it solves _most_ of the above issues and isn’t
even much lengthier, but is this how it really should look? It’s ugly enough
even without actual `std::​enable_if` and with only one argument, and I didn’t
even touch upon mutual exclusion here yet, so these two, if both present, will
make a call like `jump_fwd(1*pc)` ambiguous since no one told the compiler that
the second overload is–you guessed it–less specialized. Now imagine you had
several dozens of operators to define (they don’t like extra arguments very
much, default or not, so the SFINAE machinery is slipping into the trailing
return type, where you have to either _spell the actual return type_ or spill
short of entire function body for it to be deduced) and you’ll get the idea of
how the innards of this library _would_ look like if I didn’t use concrete names
everywhere.
And every kid’s mom knows concrete is bad for your generality.

So, we need something more radical. Something smart, robust and expressive. We
need _[concepts][]_:

	tril jump_fwd( const Distance &d );

And if we still wanted to constrain the scale, `Distance` would become something
along the lines of `Quantity<​parsec_t>`. Sadly, I still haven’t got my hands on
GCC trunk (now GCC 6), otherwise the library would likely already have some
(conditionally defined) concepts to play with.

…But I digress. On with our chaotic example.

		const auto clusters_in_database = database_size / cluster_size;
		static_assert( clusters_in_database.dimension == dimensionless, "" );

Dividing quantities with same dimension yields a dimensionless quantity. Duh.
But what’s its value? Well, what would you _like_ it to be?

		assert( clusters_in_database.count() == 42*256ull );

Sounds reasonable. What does the library say?

	cout << clusters_in_database.count() << '\n';
>     10

![wat][wat]

Aand that’s where a design hole lies, because the answer library gave
_surprised the library’s designer_. What happened? To be precise, the result is
`10ull*(Ki*unitless)`, and that’s actually a perfectly valid answer, considering
one of the design constraints I imposed: no additional runtime operations in
implementation of multiplication and division. That is to say a division of two
quantities should produce the same machine instructions as division of raw,
non‐unitized values. Hence the 10. ’Cause we’re dividing 42 by 4. Be it 42.0,
this would be, correctly, 10.5 kibiclusters, so my logic was at least _somewhat_
valid. But it still may result in loss of precision in, say, a fixed‐point
number, so I can’t simply special‐case on `is_integral`.

A better answer would be 10.5*1024 clusters. That is, moving the scale from the
type into the value by virtue of multiplication… violating the constraint of
“no extra operations”, which is unexpected in itself, not to mention possible
overflow. So, I’m gradually coming to the conclusion that there may be no right
solution here and I should simply forbid division of differently‐scaled
quantities, prompting the user to decide on the right way for themselves. Like
so:

	const auto clusters_in_database = database_size.to(kibi) / cluster_size;
	assert( clusters_in_database.count() == 42*256ull );

Moving on and away from this uncomfortable place.

		constexpr auto Mbps = M*b/s;
		auto throughput = 10.24*Mbps;

Here we define a handy alias for megabits per second, so as not to type
`(M*b/s)` everywhere. The parens are often necessary due to left‐associativity
of multiplication: the `10.24*M*b/s` would be parsed as `((10.24*M)*b)/s`, and
that opens a whole ’nuther can of worms–in the spirit of the division woes
before. Just like `std::​integral_constant`, our constants have implicit
conversion to integers, which C++ happily pounces on, producing `10240000.0`.
Whoops.

It gets worse. The value then gets to meet with the bit unit, which’d be okay by
itself, but not when the next thing that happens to the resulting quantity is
division by the second unit (no pun intended). Because that must not be allowed.
<i><q>Dividing/multiplying quantities by units for
syntax sugar like in `5*cm/s`? Why not?</q></i> Because that’s effectively a
_reinterpretation_ of the value of one unit as a value of another. _Unit
[punning][]_, if you will. An unintended pun indeed.

Now, one could make a point that dividing/multiplying raw values by units for
syntax sugar is equally unsafe. My strive for concise syntax at the dawn of the
project begged to differ, but ever since I discovered this dark side of it, it
bugged me, too. Confer:

	auto v =                myRobot.velocity()*(m/s) ;
	auto v = make_quantity( myRobot.velocity(), m/s );

The first line is obviously begging for trouble. There _must_ be interaction
with non‐unit‐safe code at _some_ points, and it’s critical to make those light
up like christmas trees as well as scream “stand back, type juggling in process”
at the reader _and_ `grep`–much like `const_cast` does for non‐const‐correct
code. The second line is perfectly greppable and couldn’t(?) be more safe, in
addition to cutting short the tug of war between operators by the function‐call
comma’s superiorly low precedence.

But… but what about constants? It’s equally possible to screw up their numeric
value as is their unit, people pay much more attention when defining them, and
they _usually_ ([looking at you, Lockheed Martin][mco]) aren’t buried in the
algorithm’s logic, sooo… can I keep this cute little syntax? Pwease?

	constexpr auto g₀ = 9.80665*m/(s^2_);  // OK? ish?
	template<typename T>
	constexpr auto turn = tau<T>*rad;      // grey zone?

Oh, by the way. That `^` there, I’m afraid we can’t have that at all. Can you
recall the relative precedence of this operator immediately? I mean _without_
google. Thought so.

I still haven’t made up my mind what is the right choice, but the indexing
operator seems innocent enough and even makes it all less cluttered:
`9.80665*m/s[2_]`. The `m/s**2_` approach is definitely not an option, being
basically multiplication, with the ensuing precedence issues.

Moving _on_. Christ, this half‐sensical example keeps uncovering design flaws!

		constexpr auto bandwidth = information/si::time;

Luckily dimensions don’t know scale, nor do they values, so this simply and
unsurprisingly produces a dimension which is the ratio of information and time.
Good time to dip the reader a little into how the dimensions are represented.
Because what we did right now is slammed our custom dimension together with one
from what practically is a third‐party library (`si.hpp` relies only on public
API and resided in the examples directory until the very last minute). How does
that work? [Expression templates][et]? Nuh‐uh. Well, kind of, but in a more
well‐mannered way than an arbitrary [AST][] built out of binary operations.

A dimension is, conceptually, an N‐ary product of rational powers of some
_fundamental_ dimensions (like information¹ × time⁻¹). So it can be thought of
as unordered set of tag‐power pairs. This is superior to a fixed‐length sequence
of powers wherein position signifies a certain fundamental dimension, because
that only allows for a fixed set of them, e.g. that of SI. This may result in a
closed/closed system: that which you can neither extend nor edit.

To the contrary, this library allows you to make up dimensions as you go, while
mixing and matching anything and everything, and still get dimensionally‐correct
results.

		constexpr auto bps = unit_of(bandwidth);
		static_assert( bps == b/s, "" );

Since we defined bandwidth as the ratio of amount of information to time, it’s
reasonable to expect that its unit will be the same thing as the ratio of our
information unit to the SI time unit. And indeed it is; exactly so.

		throughput -= 240*(k*bps);
		assert( throughput == 10*Mbps );

Units of the same dimension interact transparently, despite having different
scale. Whether this is a good thing or not is debatable. Time will tell, I
guess. Anyway, subtracting 240 kbps from 10.24 Mbps effectively subtracts
0.24 Mbps, although dividing 240 by 1000 is not how it’s done. The expression is
equivalent to `throughput = throughput - 240*(k*bps)`. Subtraction (and other
“linear” operations, such as addition and comparison) converts both sides to a
unit of some scale that is the greatest common divisor of the two scales (which
in this simple case happens to be one of them–kilo–because 1000 evenly divides
1 000 000). That way the conversion coefficients are always integers greater
than or equal to one. This is to satisfy another design constraint: never invent
runtime divisions in arithmetic operations unless that’s absolutely necessary.
And of course divisions may be absolutely necessary in conversions, such as in
assigning the kbps difference back to the original Mbps variable, thus dividing
the former by 1000. To summarize in terms of built‐in types:
`(10.24*1000LL - 240*1LL) * 1LL / 1000LL`.

The GCD approach sometimes leads to surprising, albeit correct, behavior of the
operations mentioned above. Think of what the result of adding together one
millimetre and one inch would be. That’s right, 132
one‐fifths‐of‐millimetre! <span style='font-size:1.5em'>☻</span>

		constexpr auto virtual_length = dimension<struct virtual_length_tag>{};
		constexpr auto pixel = unit_of(virtual_length), px = pixel;

Here we have an example of pulling an opaquely‐named doppelganger dimension out
of thin air simply in order to create a unit that has no static relation to
physical length units. After refactoring the library a little bit, we’ll be able
to “dynamicalize” any static part of a quantity: its dimension and/or its scale.
The latter could solve the pixel↔inch conversion problem more elegantly, if
less explicitly. More on this [later](#dynamic-scale).

		constexpr auto inch  = 0.0254_*si::metre,       in = inch;
		constexpr auto point = 1_/72_*in,               pt = point;
		constexpr auto ppi = px/in;
		constexpr auto dpi = 1_/in;

Don’t be mislead by the point in `0.0254_`, it’s still a rational constant
(¹²⁷⁄₅ ₀₀₀), not a float. And we spell the unit name there because `m` would be
ambiguous: could be metre or milli. I’m not sure which one is needed more often
in practice, so right now neither is getting preference. For the time being, it
can be disambiguated like `unit::m` and `prefix::m`, or simply by spelling the
words. Same with tesla/tera and, possibly, some others. I briefly entertained
the idea of a “chameleon” type that’d work as a unit in one context and a prefix
in another, but that wouldn’t really work out: should `m*kg/s[2_]` be
interpreted as newton or as gram per square second?

Although one could define a separate dimension for the dots, I just didn’t feel
like it for no specific reason, so dpi here is simply the inverse of inch, just
like fps is often simply hertz–the inverse of second.

		auto fallacy = 96.*ppi;
		auto imperial_blots = 300.L*dpi;
		return imperial_blots/fallacy;

Quick! What’s the return type (conceptually)? Squint to verify yourself: <s>long
double dots per pixel, which is a unit of inverse virtual length</s>.

Okay, done with the Frankenstein’s monster, now let’s try something more _sane_.



More (concise) examples
-----------------------

### How about some colorimetry?

```C++
#include <dimensional/dimensional.hpp>
using dimensional::dimension;

template<typename, typename> struct pair {};

template<typename ColorSpace, typename Channel>
constexpr auto chan = unit_of(dimension<pair<ColorSpace,Channel>>{});

template<typename T, typename ColorSpace>
struct rgb
{
	decltype(T{}*chan<ColorSpace, struct _red_tag>) r;
	decltype(T{}*chan<ColorSpace, struct _grn_tag>) g;
	decltype(T{}*chan<ColorSpace, struct _blu_tag>) b;
};

template<typename T> using sRGB = rgb<T, struct        srgb_csp_tag>;
template<typename T> using  RGB = rgb<T, struct linear_srgb_csp_tag>;

using color        = sRGB<uint_least10_t>;
using linear_color =  RGB<float>;
```

So far so clear(?). Now we’d like to desaturate that.

<pre>template&lt;typename T, typename ColorSpace&gt;
auto desaturate( const rgb&lt;T,ColorSpace&gt; &amp;c )
{
&#x20;   <img alt='// one does not simply'
&#x20;       src='https://imgflip.com/s/meme/One-Does-Not-Simply.jpg'
&#x20;       title='// one does not simply'
&#x20;       style='width:20em' />
&#x20;   return (c.r + c.g + c.b) / 3.0;
}<br />
assert( desaturate(linear_color()) == 0 );</pre>

>     In file included from color.cpp:1:0:
>     dimensional.hpp: In instantiation of 'constexpr auto dimensional::impl::heterop_raw(F, const dimensional::quantity<TB, UnitB>&, const dimensional::quantity<TB, UnitB>&) [with F = mjk::<anonymous struct>; TA = float; TB = float; UnitA = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >; UnitB = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _grn_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >]':
>     dimensional.hpp:409:29:   required from 'constexpr auto dimensional::impl::heterop(F, const A&, const B&) [with F = mjk::<anonymous struct>; A = dimensional::quantity<float, dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> > >; B = dimensional::quantity<float, dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _grn_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> > >]'
>     dimensional.hpp:417:42:   required from 'constexpr auto dimensional::operator+(const dimensional::quantity<TA, UnitA>&, const dimensional::quantity<TB, UnitB>&) [with TA = float; TB = float; UnitA = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >; UnitB = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _grn_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >]'
>     color.cpp:26:13:   required from 'auto desaturate(const rgb<T, ColorSpace>&) [with T = float; ColorSpace = linear_srgb_csp_tag]'
>     color.cpp:28:34:   required from here
>     dimensional.hpp:385:4: error: static assertion failed: operating on quantities with different dimensions
>         static_assert( a.dimension == b.dimension,
>         ^
>     dimensional.hpp: In instantiation of 'constexpr auto dimensional::impl::heterop_raw(F, const dimensional::quantity<TB, UnitB>&, const dimensional::quantity<TB, UnitB>&) [with F = mjk::<anonymous struct>; TA = float; TB = float; UnitA = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >; UnitB = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _blu_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >]':
>     dimensional.hpp:409:29:   required from 'constexpr auto dimensional::impl::heterop(F, const A&, const B&) [with F = mjk::<anonymous struct>; A = dimensional::quantity<float, dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> > >; B = dimensional::quantity<float, dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _blu_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> > >]'
>     dimensional.hpp:417:42:   required from 'constexpr auto dimensional::operator+(const dimensional::quantity<TA, UnitA>&, const dimensional::quantity<TB, UnitB>&) [with TA = float; TB = float; UnitA = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _red_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >; UnitB = dimensional::unit<dimensional::dimension_product<meta::set<dimensional::dimension_factor<pair<linear_srgb_csp_tag, _blu_tag>, meta::rational_constant<1ll, 1ll> > > >, meta::rational_constant<1ll, 1ll> >]'
>     color.cpp:26:17:   required from 'auto desaturate(const rgb<T, ColorSpace>&) [with T = float; ColorSpace = linear_srgb_csp_tag]'
>     color.cpp:28:34:   required from here
>     dimensional.hpp:385:4: error: static assertion failed: operating on quantities with different dimensions

…And that’s, of course, by design. Because before one can determine what a
color looks like in black‐and‐white, they better be sure to know what does
“white” even _mean_ (black is the easy one). In different viewing conditions,
human visual system will have different sensitivity to each of the frequencies
a display emits. If you’re in a room flooded with dim tungsten light, you
_probably_ will be a lot more sensitive to the blue side of spectrum than when
at office lit by 4500 K fluorescent tubes, so a blue channel’s contribution to
the perceived intensity of the color will be more pronounced in the former case.
And we’re making an assumption that we’re desaturating _for humans to see_. What
about cats? Dogs? Mantis shrimp? Forget meatbags, what if we’re simulating b/w
photographic film? For science. Then the whole story of viewing conditions goes
out the window, and we need the response curve of a particular film. So yeah,
you’re not getting away with (r+g+b)/3 even if r, g and b are raw linear
non‐weighted watts. Fortunately, there are standards and guidelines, so it’s not
all that hopeless.

	template<typename T>
	auto desaturate( const rgb<T, linear_srgb_csp_tag> &c )
	{
		return make_quantity(
			0.2126 * c.r.count() +
			0.7152 * c.g.count() +
			0.0722 * c.b.count(),
			chan<linear_srgb_csp_tag, struct luminance_tag> );
	}
	
	template<typename T>
	auto desaturate( const rgb<T, srgb_csp_tag> &c )
	{
		return gamma_compress(desaturate(gamma_expand(c)));
	}

That should do the trick for the sRGB color space (specifically), but
[don’t take my word for it][desat].

Dimensionification of weights and gamma‐expansion/​‐compression are left as
exercises for the reader.

#### BRG

The unit‐safe approach also rightfully forbids you to just `swap(c.r, c.b)` in
order to get BGR triplet stored in an RGB type, because, again, that equates to
type punning. BRG is a separate way of representing RGB color in memory and
should be treated as such by being honoured with a separate type.


### Bulk data

```C++
using color = sRGB<std::uint8_t>;
static_assert( sizeof(color) == sizeof(std::uint8_t)*3, "" );

constexpr auto in = 0.0254_*m;
constexpr auto px = unit_of( dimension<struct dev_len>() );

auto res = 300*(px/in);
auto area = (8.3*in) * (11.7*in) * square(res);
static_assert( area.unit == square(px), "" );
std::vector<color> canvas{ area.count() };
assert( canvas.size() == 8'739'900 );
```

That’s 26 219 700 quantity objects, one byte each.

Instead of `area.count()` with the accompanying assertion on the unit, we could
ask how many (square) pixels there are in `area` more “directly”, with division:
`area/(1*square(px))`. While that _may_ look more natural to a mathematician, it
yields questionable benefits in terms of readability _and_ maintainability.


### Foreign type interoperability

```C++
using si::unit::m;
using si::s;

constexpr auto g0 = 9.8*(m/s/s);

template<typename Height>
void wait_til_the_fish_jumps_to( Height h )
{
	static_assert( h.dimension == si::length,
		"height must be length. don’t question that." );
	std::this_thread::sleep_for(
		std::chrono::nanoseconds{ 2 * sqrt(2*h/g0) } );
	// TODO: add air
}

int main()
{
	wait_til_the_fish_jumps_to( 42*m );  // about 6 seconds

	using namespace std::chrono_literals;
	auto USD = unit_of( dimensional::dimension<struct bucks>{} );
	assert( 400'000*USD/12s == 33'333*(USD/s) );
}
```

Note: The last line currently won’t compile without explicitly converting `12s`
to `decltype(0ll*si::s)` (which, admittedly, would utterly devastate the purpose
of using a chrono literal in this example). The solution to that is _not_ pretty
without concepts, hence not implemented as yet. And I’m not sure a transparent
solution (i.e. one that avoids stating conversion explicitly in any way or form)
even exists for the case of passing quantities to functions that take `duration`
with deducible params, such as `sleep_for`. Explicit conversion have to remain
at least in _some_ from, and even though it can be reduced to a minimum, like in
`sleep_for(​dur( 2*sqrt(​2*h/g0) ))`, still, the decision to forgo concept‐based
signatures for standard duration‐taking functions to favor one single `duration`
template was a considerable loss for the generality of standard library,
which–outside STL–is already modest at best (ah, the omnipresence of
`std::​string`…). Welp, at least we are spared from obviating the cast by
turning deduction off, as in
`std::​this_thread::​sleep_for<​long long, std::nano>( 2*sqrt(​2*h/g0) )`.

I think to get rid of the casts for good, while keeping the same level of
customizability, would require conversion‐to‐concept operators. Is that a thing?
(Doesn’t seem to be.)


### Examples of complete type stacks

```C++
#include <boost/multiprecision/cpp_int.hpp>
#include <dimensional/dimensional.hpp>
#include <mjk/vec>
#include <mjk/point>

using dimensional::dimension;

using data_type  = boost::multiprecision::cpp_rational;


// for safe geometric computations in different 3D coordinate spaces

constexpr auto L = dimension<struct _length_tag>{},  length = L;
constexpr auto m = unit_of(L),  metre = m;
using metre_qty  = decltype( data_type{}*m );
using metre_vec  = mjk::vec< metre_qty, 3 >;

using position   = mjk::point< metre_vec, struct world_space_tag >;
using offset     = metre_vec;
using local_pos  = mjk::point< metre_vec, struct entity_local_space_tag >;


// for temperatures

constexpr auto Θ = dimension<struct _tempr_tag>{},  temperature = Θ;
constexpr auto K = unit_of(Θ),  kelvin = K;
constexpr auto deg_C =       K, degree_celsius    = deg_C;
constexpr auto deg_F = 5_/9_*K, degree_fahrenheit = deg_F;
using kelvin_qty = decltype( data_type{}*K ) );
using  deg_C_qty = decltype( data_type{}*deg_C ) );
using  deg_F_qty = decltype( data_type{}*deg_F ) );
static_assert(     std::is_same< deg_C_qty, kelvin_qty >{}, "" );
static_assert( not std::is_same< deg_F_qty, kelvin_qty >{}, "" );

using abs_tempr  = mjk::point< kelvin_qty, struct absolute_scale_tag >;
using tempr_diff = kelvin_qty;
static_assert( not std::is_same< abs_tempr,  deg_C_qty >{}, "" );
static_assert(     std::is_same< tempr_diff, deg_C_qty >{}, "" );
using tempr_C = mjk::point< deg_C_qty, struct celsius_scale_tag >;
using tempr_F = mjk::point< deg_F_qty, struct fahrenheit_scale_tag >;
```

Names could be better, and `mjk::​point` is yet to be finished and released, but
you get the idea. If not, then, in short, this example touches upon the notion
of _spaces_ (or _scales_ in the one‐dimensional case of temperatures), and if
you’re familiar with `std::​chrono`, you can think of spaces as a generalization
of _clocks_. Thus, `mjk::​point` is a generalization of _time point_.



Design matters
--------------

With the majority of design points being elaborated on in the introduction,
what’s left to do is to make a short summary of the goals. In a semi‐random
order:

1. Have clear‐cut concepts

	Dimension, unit, scale, quantity.

2. Allow for easy extension of and transparent interaction between different
   dimension/unit sets

	Check.

	Notice the absence of a “unit system” concept above. For fear of collisions
	between completely unrelated units, which nonetheless share a common
	dimension–such as _length_–I considered introducing “universe”–practically,
	unit system–tags. Trying to fit them onto quantities and quickly failing, I
	started mentally pushing them up the abstraction tree–into units, then
	dimensions–until realizing one simple fact: the dimension’s namespace
	**_is_** the tag, and there really is no need to use the same tag for
	conceptually similar, but not necessarily identical things. If I want to
	work with a unit of length from some fantasy world that has _unknowable_
	relation to, say, metre, nothing really says I have to reuse the metre’s
	dimension. In fact, it even looks fishy:

		constexpr auto quux = unit_of(si::length);

	“Isn’t SI unit of length the metre?”–immediately comes to mind. Instead it
	should look more like `quux = unit_of(uvuvuwu::​length)`. Problem solved,
	and the solution feels natural. In addition, if I fancy to do complicated
	computations mixing in real‐world units, I don’t need to do anything
	special, it _just works_: `(6*quux) * (34.68*m)` will give me heterogeneous
	area. Now, of course taking square root of it would result in neither metres
	nor quuxes, but dividing the area by either of them would produce something
	meaningful, so _quux‐metre_ is still a useful unit for intermediate results.
	As well as quux‐metre<sup>½</sup>, for that matter.

3. Make syntax read like prose

	1. Minimize boilerplate as much as preprocessorlessly possible
		* `constexpr auto` is _almost_ not noisy
	2. Little to none angle brackets and double colons
		* some pointies will still get on your carpet for explicit concept
		  parametrization purposes

	Nearly check. Constexpr lambdas may greatly help with both, turning
	`dimension<​struct blah_blah_tag>{}` into `make_dimension(​[]{})`.

4. Ensure constexpr transparency

	Check. `constexpr auto A = τ*square(1.0*m)/2;`

5. No space overhead

	Check.

6. No time overhead

	1. Never invent operations in implementation of `operator*` and `operator/`

		Check. Although division looks like it needs to be more conservative
		in accepting hetero‐scaled quantities.

	2. Never invent divisions in implementation of other arithmetic operations;
	   however, extra multiplications are fine

		Check. As a bonus, the extra multiplications are always integral.

	All functions look inlinable, but actual assembly is yet to be inspected.

7. Allow for interoperability with unrelated types, such as
   `std::​chrono::​duration`

	Partly check. It’s in need of some concepts love, because:
	* explicit conversion of foreign types still often required
		* may be worthwhile to try C++14 solutions
	* class template specialization can at times be
	  [slightly][understatement] unsightly
		* see [`si.hpp:117`](include/dimensional/si.hpp#L117) for an example

8. Agnostic to data type

	Looks like a check. Should work equally well(?) with anything from
	`unsigned char` to `BigDecimalFloat` to `std::​string`. Not sure about
	string, though. Also, ET transparency needs testing.

	This may not be that good of a thing due to 9.2 and 9.3.

9. Reliably safe / fool‐proof / hard‐to‐misuse

	1. availability of `thruster.get_force()*N` syntax kinda undermines this
	2. possibly‐unexpected overflow opportunity in `+`, `<`, `==`, etc.
		* ` INT_MAX*kg + 0*g` has signed integer overflow
		* `UINT_MAX*kg + 0*g` is `(UINT_MAX-999)*g` due to unsigned overflow
	3. possibly‐unexpected precision loss opportunity in `+=` and `-=`
		* `for (auto m = 0*kg; m < 1*kg; m += 1*g)` is infinite loop
		* `for (auto m = 1*kg; m > 0*kg; m -= 1*g)` does only one iteration
	4. ???

10. Minimal external dependencies

	None so far.

11. Modular

	From the user perspective–still not very much. At the very least, need to
	cut the header into several pieces. Those would still have multi‐tier
	relationship if further decoupling of dimensions, units and quantities is
	not made by leveraging concepts.

12. Good compilation performance

	Clearly a room for improvement, even without sophisticated benchmarking.

	1. Including the monolithic `dimensional.hpp` seems pretty quick: took
	   g++ 4.9.0 on a decade‐old computer only 0.42 s. As for compiling code
	   that actually uses the library… well, including `si.hpp` took 1.06 s,
	   with 0.63 s alone being template instantiation time.
	2. Memory usage is about 20.5 MiB and 39.3 MiB for the main header and SI
	   one, resp.

	Literal‐parsing code looks like _the_ most shameless performance hog, with
	recursive meta‐algorithms presumably being a close second. I’ll take a wild
	guess and say that the O(_N_×_M_) operations on dimensions is _not_ the main
	culprit, though, what with _N_ and _M_ not exceeding 3 for the majority
	(all?) of operations in the definition of `si` units.

	Why the quadratic complexity, though? Because I didn’t (yet) find a way to
	strict‐weakly order the tags without subjecting the user to atrocities of
	`dimension<​'s','i','.',​'l','e','n','g','t','h'​>{}` degree. Template‐based
	string UDLs would help (and also solve some of the I/O problems), but the
	ultimate answer seems to lie in the area of reflection or, at least, CTTI,
	because `"si.length"_dim` still would rely on the programmer paying extra
	attention to keep the prefixes in sync with the namespace name and each
	other.


### Honorable mention

At a glance, it seems UDLs were designed with `25.4mm * 1.21GW` kind of usage in
mind. And that’s true, as evidenced by `std::chrono` library, but, as evidenced
by `std::chrono` again, UDLs are not composable: you get to specify the unit and
leave the datatype choice to the library designers (`3m + 14s`). Or specify the
datatype and be left in a unitless world (`3.f + 14LL`). So, if you want to have
the cake and eat it too, embrace the combinatorial explosion of
N<sub><small>DT</small></sub>×N<sub><small>U</small></sub> literals!
(And with prefixes accounted for, this becomes
N<sub><small>DT</small></sub>×N<sub><small>P</small></sub>×N<sub><small>U</small></sub>.)

But we live in the world where STL is not just a thing, it’s _thriving_. In the
world where N<sub><small>DT</small></sub>+N<sub><small>U</small></sub> is
possible _and_ is the default way–the Tao of C++. Hence the operator syntax,
with literals responsible strictly for the datatype (`3.f*m + 14LL*s`).



Roadmap / Further work
----------------------

Major points that didn’t come up in design goals discussion, in a
somewhat‐prioritized order:

1. Flesh out the library by implementing the rest of the most basic operations.
   For example, for quantities:

	* there’s `<`,  but no `>`,
	* there’s `+=`, but no `*=`,
	* there’s `sqrt`, but no `cbrt`.

	Et cetera.

2. Allow for dimensions and scales whose identity/value is not part of their
   type–“dynamicalize” the library. Examples follow.

	#### Dynamic scale

	In the introductory example we utilized dimensions to define a screen‐length
	unit of statically‐unknown relation to physical lengths:

		constexpr auto virtual_length = dimension<struct virtual_length_tag>{};
		constexpr auto pixel = unit_of(virtual_length),   px = pixel;

	With runtime‐scaled units we could allow smoother conversions between them:

		const     auto pixel = dynamic_ppi{display}*inch, px = pixel;

	where `dynamic_ppi` would satisfy `Scale` concept and query the current ppi
	of a device for each conversion of `pixel` quantities to and from other
	length quantities:

		void Widget::SetSize( decltype(int2{}*px) sz );
		:::
		widget.SetSize( (int2{210,297}*mm).to(px) );

	Since the unit of `sz` is stateful (and–suppose–doesn’t have any meaningful
	state a default constructor could put it into, if defined), we can’t perform
	the conversion to it without a unit instance, hence the explicit `.to(px)`.

	Alternatively, we could store the conversion coefficient at construction and
	use it throughout the unit’s lifetime:

		const auto left_disp_pixel  = make_scale(displays[0].ppi())*inch;
		const auto right_disp_pixel = make_scale(displays[1].ppi())*inch;

	Of course the stateful approach means each `pixel` quantity has to become
	fattier by at least a byte. Else, we’d need a global variable:

		extern display_device display;
		constexpr auto pixel = dynamic_ppi<&display>{}*inch, px = pixel;

	Despite requiring evil global variables, such a scale has the advantage of
	statelessness, which allows for seamless implicit conversions, without
	keeping an instance of the unit handy:

		widget.SetSize( int2{210,297}*mm );

	#### Dynamic everything

	Another example of use would be a unit‐aware calculator that stores things
	akin to `rational{​1852,​1000} * (scales["k"]*​units["m"])` (where `units`
	maps strings to stuff like `unit_of(​make_dyn_dimension(​"si.length"s))`) in
	the leaves of its AST, handles dimension mismatches “eagerly” with branching
	or, perhaps, “lazily” with exceptions, and presents the result either “as
	is” or after conversion to a desired unit.

3. Move‐friendliness

	Regarding the move semantics, currently everything is very “nineties”:
	`const &` in arguments proliferate. Forwarding references with constrained
	placeholders would be ideal. Until concepts, we could perhaps make do with
	simple pass‐by‐value. Overloading on r‐valueness is the last resort and is
	practically out of the question, considering that binary operations would
	require four overloads each.

4. `noexcept` transparency

	Hard to believe that the work on `noexcept(auto)` proposal was postponed
	again. Oh well. The manual way may turn out to be not so scary in the end.
	We’ll see. This is a low‐priority one right now.

5. I/O

	Will probably require an approach similar to that for foreign type
	interoperability in order to have `cout << 1.5*W*15s` print `22.5 J`.
	At first, a simple value‐scale‐dimension triplet could be enough, like

	> `22.5 [1/1]si.mass^2*si.length^2*si.time^-2`

	This is supposed to be highly machine‐readable, so input validation comes
	cheaply.

	Accepting any scale and converting to the scale required seems to be of too
	low a bang‐to‐buck ratio to consider right now. On the other side of effort
	spectrum is the pass‐me‐that‐beer approach: leave it for the user to read
	raw values and construct quantities with them.

	And then there’s i18n. God help us.

6. Logarithmic quantities?



<br />
<br />
<br />

<img
 alt='&lt;photo of an elephant in the room&gt;'
 src='https://upload.wikimedia.org/wikipedia/commons/3/3b/The_Elephant_in_the_Room_Banksy-Barely_legal-2006.jpg'
 style=
 '
  display: block;
  margin-left:  auto;
  margin-right: auto;
  text-align: center;
 '
/>

Okay, let’s finally address it. What about…

[Boost.Units][]
---------------

Not much to say about it, really, given it was practically _yesterday_ (as of
this writing) that I condescended to familiarize myself with it. However, what
fleeting acquaintance I had with the library, was enough to conclude for myself
that our paths are those of little codirectionality. Reading the docs, my eyes
glazed over from the amount of boilerplate and syntactic noise, and I haven’t
even got past the basics. The verbosity is in part due to having been designed
and written for C++98, and thus unavoidable, but it’s amplified by a number of
seemingly unnecessary concepts (one of them being _unit systems_), which do not
only clutterify very simple things, but–overall–impose too much structure, in my
view, making the library very rigid. Will my project be able to succeed without
much of this structure? Well, I wouldn’t be able to tell without trying anyway.

Despite the drastic difference in design, it wouldn’t be impossible at all to
write glue for smooth interaction of new code that uses `dimensional::​quantity`
with code already heavily relying on Boost.Units’ quantities, just as has been
demonstrated for `std::​chrono::​duration`.



Installation and Requirements
-----------------------------

Just add `include` directory to your compiler’s include paths, and you’re ready.

The library requires some C++14 support. It’s been developed on g++ 4.9.0 so
far, so (in theory) any newer version goes as well. Haven’t tried on clang++,
but it may just work on not‐too‐old versions. Visual C++ 2027 also has pretty
decent chances to compile this.



Docs
----

Only this readme, for the time being.



License
-------

[3‐clause BSD](LICENSE.md).



Contributing
------------

The best way to help right now is to:

* (ab)use the hell out of the library,
* [file issues][issues] with bugs, suggestions and questions–on design,
  implementation _and_ documentation.

And not necessarily in that order.

Submitting code is probably a bit too early: what’s written is likely to change
drastically, so even contributed tests would not be exempt from major overhauls
during this primordial stage, not to mention I’m still unsure of my choice of
the license.



Acknowledgements
----------------

I want to thank [Louis Dionne][] for popularizing the “constexpr
metaprogramming” paradigm. After watching several presentations of his, even
though I didn’t go right away and try [Hana][], the seedling of idea was
planted and soon started cropping up in my code _everywhere_. Eventually I
started to really _get_ what that “fusion” of run‐time and compile‐time,
static and dynamic, is all about.

For better or worse, I’m still inclined not to scrap what habitual meta‐code
I’ve hacked together as a back‐end for this project. Partly because turning
this mess into something elegant–or even beautiful–is _interesting_, and even
enlightening.

Another thank‐you goes to [Howard Hinnant][] and the rest of `std::​chrono`
authors for bringing this well‐thought‐out facility to the masses, thus
allowing–among many things–to draw inspiration from it. Even without properly
studying its design until recently (and inventing many a wheel as a
consequence), just the idea and implementation of ratio types already gave
me a solid foundation to work from. The `.count()` syntax is not incidental
either, even though I still may change that to `.value()`.

The similarities will most likely go beyond, though. I’ve yet to externalize
the common type computations via `std::​common_type` specialization; to decide
between `quantity_cast` and `qty_cast` to complement `.to(:::)` syntax; to come
to the conclusion that something like `treat_as_floating_point` is needed… The
list goes on.



[UDL]: http://en.cppreference.com/w/cpp/language/user_literal
	"user‐defined literal"
[ODR]: http://en.cppreference.com/w/cpp/language/definition#One_Definition_Rule
	"one definition rule"
[et]:  https://en.wikipedia.org/wiki/Expression_templates
[AST]: https://en.wikipedia.org/wiki/Abstract_syntax_tree
	"abstract syntax tree"
[concepts]: http://en.cppreference.com/w/cpp/language/constraints
[punning]:  https://en.wikipedia.org/wiki/Type_punning
	"type punning"
[mco]:      http://web.mit.edu/16.070/www/readings/Failures_MCO_MPL.pdf#6
	"The failures of mars climate orbiter and mars polar lander: A perspective from the people involved., page 6"
[desat]:    https://en.wikipedia.org/wiki/Grayscale#Colorimetric_.28luminance-preserving.29_conversion_to_grayscale
	"Colorimetric conversion to grayscale"
[understatement]: http://tvtropes.org/pmwiki/pmwiki.php/Main/Understatement
[Boost.Units]:    https://www.boost.org/doc/libs/release/doc/html/boost_units.html
[issues]:         https://github.com/Yuubi-san/dimensional/issues "Issues"
[Louis Dionne]:   https://github.com/ldionne          "ldionne"
[Hana]:           https://github.com/boostorg/hana    "boostorg/hana"
[Howard Hinnant]: https://github.com/HowardHinnant    "HowardHinnant"

[wat]:      http://i3.kym-cdn.com/photos/images/original/000/173/580/Wat.jpg



<br />
<br />
<br />

--------------------------------------------------------------------------------

	constexpr auto text_amount   = make_dimension([]{});
	constexpr auto sense_amount  = make_sense().dimension();
	constexpr auto reading_speed = text_amount/si::time;
	
	namespace expressive
	{
		constexpr auto energy = sense_amount;
		constexpr auto power =
			energy / (text_amount/reading_speed);
	}
	
	namespace dimensional
	{
		constexpr auto χ = (9e3 + ε) * unit_of(expressive::power);
	}
