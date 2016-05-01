
// dimensional analysis tools

#ifndef DIMENSIONAL_H
#define DIMENSIONAL_H

#include "impl/meta.hpp"
#include "impl/rational_constant.hpp"
#include "impl/mjk/conv"

namespace dimensional
{
	template<intmax_t Num = 0, intmax_t Den = 1>
	using constant = meta::rational_constant<Num,Den>;
	namespace constant_literals = meta::rational_constant_literals;


	// represents a single fundamental physical dimension, exponentiated
	// e.g. time; length squared
	template<typename Tag, typename Power>
	struct dimension_factor
	{
		using tag_type = Tag;
		static constexpr meta::type<Tag> tag{};
		static constexpr Power power{};
	};

	// represents a product of multiple exponentiated dimensions
	// e.g. mass¹ × length¹ × time⁻²
	template<typename FactorSet>
	struct dimension_product
	{
		static_assert( !sizeof(FactorSet*),
			"bad parameter for 'dimension_product': "
				"'meta::set' of 'dimension_factor' expected" );
	};
	template<typename... Tags, typename... Powers>
	struct dimension_product< meta::set< dimension_factor<Tags,Powers>... > >
	{
		static constexpr auto factors = meta::set< dimension_factor<Tags,Powers>... >{};
	};

	// makes a dimension_product for a single dimension
	template<typename Tag>
	using dimension = dimension_product< meta::set< dimension_factor<Tag,constant<1>> > >;

	constexpr auto dimensionless = dimension_product< meta::set<> >{};

	namespace impl
	{
		template<typename DimA, typename DimB>
		using is_related = decltype( DimA::tag == DimB::tag );

		template<typename Dim>
		using is_one = decltype( Dim::power == constant<>{} );

		template<typename DimA, typename DimB>
		struct mul
		{
			static_assert( is_related<DimA,DimB>::value, "" );
			using type = dimension_factor<typename DimA::tag_type, decltype(DimA::power + DimB::power)>;
		};

		template< typename Set >
		using remove_ones = meta::uset::remove_if< Set, is_one >;

		template< typename SetA, typename SetB >
		struct multiply;

		template<
			template<typename...> class Set,
			typename... FactorsA,
			typename FactorsB_Head, typename... FactorsB_Tail
		>
		struct multiply< Set<FactorsA...>, Set<FactorsB_Head,FactorsB_Tail...> >
		{
		private:
			using tailprod = typename multiply< Set<FactorsA...>, Set<FactorsB_Tail...> >::type;
			using multiplied = meta::uset::insert< tailprod, FactorsB_Head, is_related, mul >;
		public:
			using type = remove_ones< multiplied >;
		};

		template<
			template<typename...> class Set,
			typename... FactorsA
		>
		struct multiply< Set<FactorsA...>, Set<> >
		{
			using type = Set<FactorsA...>;
		};

		template<typename Pow>
		struct pow
		{
			template<typename Dim>
			struct f
			{
				using type = dimension_factor< typename Dim::tag_type, decltype(Dim::power * Pow{}) >;
			};
		};
	}

	// dimen * dimen
	template<typename FactorsA, typename FactorsB>
	inline constexpr auto operator*( dimension_product<FactorsA>, dimension_product<FactorsB> )
	{ return dimension_product< typename impl::multiply<FactorsA,FactorsB>::type >{}; }

	// dimen ^ const
	template<typename FactorsA, intmax_t Num, intmax_t Denom>
	inline constexpr auto operator^( dimension_product<FactorsA>, constant<Num,Denom> pow )
	{ return []{}, dimension_product< impl::remove_ones<
		meta::uset::transform<FactorsA, impl::pow<decltype(pow)>::template f> > >{}; }

	// dimen / dimen
	template<typename FactorsA, typename FactorsB>
	inline constexpr auto operator/( dimension_product<FactorsA> a, dimension_product<FactorsB> b )
	{ return a * (b^constant<-1>{}); }

	// dimen == dimen
	template<typename FactorsA, typename FactorsB>
	inline constexpr auto operator==( dimension_product<FactorsA>, dimension_product<FactorsB> )
	{ return meta::uset::equal<FactorsA,FactorsB>{}; }
	// dimen != dimen
	template<typename FactorsA, typename FactorsB>
	inline constexpr auto operator!=( dimension_product<FactorsA> a, dimension_product<FactorsB> b )
	{ return meta::not_( a == b ); }

	// +dimen
	template<typename Factors>
	inline constexpr auto operator+( dimension_product<Factors> d )
	{ return d; }



	template<typename Dimension, typename Scale>
	struct unit
	{
		static_assert( !sizeof(Dimension*),
			"bad parameters for 'unit': "
				"'dimension_product' and 'constant' expected" );
	};
	template<typename Factors, intmax_t Num, intmax_t Den>
	struct unit< dimension_product<Factors>, constant<Num,Den> >
	{
		static constexpr auto dimension = dimension_product<Factors>{};
		static constexpr auto scale     = constant<Num,Den>{};
	};

	// dimen -> unit
	template<typename FactorSet>
	inline constexpr auto unit_of( dimension_product<FactorSet> d )
	{ return unit< decltype(d), constant<1> >{}; }

	constexpr auto unitless = unit_of( dimensionless );


	// unit * unit
	template<typename DimA, typename ScaleA, typename DimB, typename ScaleB>
	inline constexpr auto operator*( unit<DimA, ScaleA>, unit<DimB, ScaleB> )
	{ return (ScaleA{} * ScaleB{}) * unit_of(DimA{} * DimB{}); }

	// unit / unit
	template<typename DimA, typename ScaleA, typename DimB, typename ScaleB>
	inline constexpr auto operator/( unit<DimA, ScaleA>, unit<DimB, ScaleB> )
	{ return (ScaleA{} / ScaleB{}) * unit_of(DimA{} / DimB{}); }

	// unit ^ const
	template<typename Dim, typename Scale, intmax_t Num, intmax_t Denom>
	inline constexpr auto operator^( unit<Dim,Scale>, constant<Num,Denom> p )
	{ return pow(Scale{},p) * unit_of(Dim{}^p); }

	// sqrt( unit )
	template<typename Dim, typename Scale>
	inline constexpr auto sqrt( unit<Dim,Scale> u )
	{ return u ^ constant<1,2>{}; }

	// unit == unit
	template<typename DimA, typename ScaleA, typename DimB, typename ScaleB>
	inline constexpr auto operator==( unit<DimA, ScaleA>, unit<DimB, ScaleB> )
	{ return meta::bool_constant<DimA{} == DimB{} && ScaleA{} == ScaleB{}>{}; }

	// unit != unit
	template<typename DimA, typename ScaleA, typename DimB, typename ScaleB>
	inline constexpr auto operator!=( unit<DimA, ScaleA> a, unit<DimB, ScaleB> b )
	{ return meta::not_( a == b ); }

	// const * unit
	template<intmax_t Num, intmax_t Den, typename Dim, typename ScaleB>
	inline constexpr auto operator*( constant<Num,Den> scaleA, unit<Dim, ScaleB> )
	{ return unit<Dim, decltype(scaleA*ScaleB{})>{}; }

	// const / unit
	template<intmax_t Num, intmax_t Den, typename Dim, typename ScaleB>
	inline constexpr auto operator/( constant<Num,Den> scaleA, unit<Dim, ScaleB> b )
	{ return scaleA * (b^constant<-1>{}); }


	template<typename T, typename Unit>
	class quantity;

	template<typename T, typename Dim, typename Scale>
	class quantity< T, unit<Dim, Scale> >
	{
		T val;

		using unit_type = unit<Dim, Scale>;
		using this_type = quantity<T, unit_type>;

	public:
		constexpr quantity() = default;
		explicit constexpr quantity( const T &val ) : val(val) {}

		template<typename TR, typename DimR, typename ScaleR>
		constexpr quantity( const quantity<TR, unit<DimR, ScaleR>> &rhs )
			: val( rhs.to( scale ).count() )
		{
			static_assert( rhs.dimension == dimension,
				"converting from quantity with different dimension" );
		}

		template<
			typename U,
			typename _conv = mjk::conversion< U, this_type >,
			typename _enabler = decltype(_conv{}) >
		constexpr quantity( const U &other )
			: val( _conv{}(other).count() )
		{}

		template<
			typename U,
			typename _conv = mjk::conversion< this_type, U >,
			typename _enabler = decltype(_conv{}) >
		operator U() const
		{
			return _conv{}( *this );
		}

		template<intmax_t Num, intmax_t Den>
		constexpr auto to( constant<Num, Den> c ) const
		{
			using s = decltype(scale / c);
			using num = decltype(s::num);
			using den = decltype(s::den);
			return T(count()*num{}/den{}) * (c * unit_of(dimension));
		}
		template<typename ToDim, typename ToScale>
		constexpr auto to( unit<ToDim, ToScale> u ) const
		{
			static_assert( u.dimension == dimension,
				"converting quantity to unit with different dimension" );
			return to( u.scale );
		}

		template<typename TR, typename UnitR>
		quantity &operator+=( const quantity<TR, UnitR> &rhs )
		{
			return *this = *this + rhs;
		}
		template<typename TR, typename UnitR>
		quantity &operator-=( const quantity<TR, UnitR> &rhs )
		{
			return *this = *this - rhs;
		}


		constexpr const auto &count() const { return val; }

		static constexpr meta::type<T> type{};
		static constexpr unit_type unit{};
		static constexpr auto dimension = unit.dimension;
		static constexpr auto scale = unit.scale;
		static constexpr auto num = scale.num;
		static constexpr auto den = scale.den;
	};

#ifdef VERYDEBUG
	template
	<
		typename... Stuff,
		typename Unit
	>
	class quantity< quantity<Stuff...>, Unit >
	{
		static_assert( !sizeof(Unit*),
			"quantity as datatype of quantity? something's fishy here" );
	};
#endif

	template<typename DataType, typename Dim, typename Scale>
	inline constexpr auto make_quantity( const DataType &val, unit<Dim, Scale> u = unitless )
	{ return quantity< DataType, decltype(u) >{ val }; }

	// T * unit
	template<typename DataType, typename Dim, typename Scale>
	inline constexpr auto operator*( const DataType &value, unit<Dim, Scale> u )
	{ return make_quantity( value, u ); }

	// quant * unit
	template<typename DataType, typename Unit, typename Dim, typename Scale>
	inline constexpr auto
	operator*( const quantity<DataType, Unit> &a, unit<Dim, Scale> u )
	{ return a.count() * (Unit{} * u); }

	// +quant
	template<typename DataType, typename Unit>
	inline constexpr auto
	operator+( const quantity<DataType, Unit> &q )
	{ return q; }

	// quant / unit
	template<typename DataType, typename Unit, typename Dim, typename Scale>
	inline constexpr auto
	operator/( const quantity<DataType, Unit> &a, unit<Dim, Scale> u )
	{ return a.count() * (Unit{} / u); }

	// quant * quant
	template<typename TA, typename TB, typename UnitA, typename UnitB>
	inline constexpr auto
	operator*( const quantity<TA, UnitA> &a,
			   const quantity<TB, UnitB> &b )
	{ return (a.count() * b.count()) * (UnitA{} * UnitB{}); }

	// quant / quant
	template<typename TA, typename TB, typename UnitA, typename UnitB>
	inline constexpr auto
	operator/( const quantity<TA, UnitA> &a,
			   const quantity<TB, UnitB> &b )
	{ return (a.count() / b.count()) * (UnitA{} / UnitB{}); }

	// T * quant
	template<typename DataTypeA, typename DataTypeB, typename Unit>
	inline constexpr auto
	operator*( const DataTypeA &a, const quantity<DataTypeB, Unit> &b )
	{ return (a * b.count()) * b.unit; }

	// quant * T
	template<typename DataTypeA, typename DataTypeB, typename Unit>
	inline constexpr auto
	operator*( const quantity<DataTypeA, Unit> &a, const DataTypeB &b )
	{ return (a.count() * b) * Unit{}; }

	// quant / T
	template<typename DataTypeA, typename DataTypeB, typename Unit>
	inline constexpr auto
	operator/( const quantity<DataTypeA, Unit> &a, const DataTypeB &b )
	{ return (a.count() / b) * Unit{}; }

	// sqrt( quant )
	template<typename DataType, typename Unit>
	inline auto sqrt( const quantity<DataType, Unit> &q )
	{
		using std::sqrt;
		return sqrt(q.count()) * sqrt(q.unit);
	}


	// heterogeneous operations (on quantities with different scale):
	// addition, comparison, etc.
	namespace impl
	{
		template<intmax_t Val>
		using c = mjk::intmax_constant<Val>;

		inline constexpr auto min(intmax_t a, intmax_t b) {return a<b ? a : b;}
		inline constexpr auto max(intmax_t a, intmax_t b) {return a>b ? a : b;}

		template<intmax_t A, intmax_t B>
		inline constexpr auto gcd( c<A>, c<B> )
		{
			static_assert( A >= 0 && B >= 0, "" );
			return c< mjk::sgcd(A,B) >{};
		}

		template<intmax_t A, intmax_t B>
		inline constexpr auto lcm( c<A>, c<B> )
		{ return c< max(A,B) / gcd(c<A>{},c<B>{}) * min(A,B) >{}; }

		template< typename F,
			typename TA, typename TB, typename UnitA, typename UnitB >
		inline constexpr auto
		heterop_raw( F op,
			const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
		{
			static_assert( a.dimension == b.dimension,
				"operating on quantities with different dimensions" );
			//   The aliases in place of static constexpr are due to what seems
			// like a deficiency in g++ 4.9.0.
			using num_gcd = c< gcd(a.num, b.num) >;
			using den_gcd = c< gcd(a.den, b.den) >;
			//   The following were moved out of the expression, otherwise
			// num and den suddenly appear before g++ linker with (naturally)
			// no out-of-class definitions. Another g++ bug, or a C++ defect?
			using a_scale = c< (a.num/num_gcd{})*(b.den/den_gcd{}) >;
			using b_scale = c< (b.num/num_gcd{})*(a.den/den_gcd{}) >;
			return op
			(
				TA( a.count() * a_scale{} ),
				TB( b.count() * b_scale{} )
			);
		}

		template< typename F, typename A, typename B >
		inline constexpr auto
		heterop( F op, const A &a, const B &b )
		{
			using common_scale = constant< gcd(a.num, b.num), lcm(a.den, b.den) >;
			using common_unit  = decltype( common_scale{} * unit_of(a.dimension) );
			return heterop_raw(op,a,b) * common_unit{};
		}
	}

	// quant + quant
	template< typename TA, typename TB, typename UnitA, typename UnitB >
	inline constexpr auto
	operator+( const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
	{ return impl::heterop( mjk::plus, a, b ); }
	// quant - quant
	template< typename TA, typename TB, typename UnitA, typename UnitB >
	inline constexpr auto
	operator-( const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
	{ return impl::heterop( mjk::minus, a, b ); }
	// quant < quant
	template< typename TA, typename TB, typename UnitA, typename UnitB >
	inline constexpr auto
	operator<( const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
	{ return impl::heterop_raw( mjk::less, a, b ); }
	// quant <= quant
	template< typename TA, typename TB, typename UnitA, typename UnitB >
	inline constexpr auto
	operator<=( const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
	{ return !(b < a); }
	// quant == quant
	template< typename TA, typename TB, typename UnitA, typename UnitB >
	inline constexpr auto
	operator==( const quantity<TA, UnitA> &a, const quantity<TB, UnitB> &b )
	{ return impl::heterop_raw( mjk::equal_to, a, b ); }
}

#endif
