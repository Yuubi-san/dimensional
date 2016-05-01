
#include <utility>

namespace mjk
{
	namespace impl
	{
		template<typename Src, typename Dst>
		struct rev_int_seq;

		template
		<
			template<typename I, I...> class IntSeq,
			typename T, T Src0, T... Src, T... Dst
		>
		struct rev_int_seq
		<
			IntSeq<T, Src0, Src...>,
			IntSeq<T,       Dst...>
		>
		{
			using type = typename rev_int_seq
			<
				IntSeq<T,       Src...>,
				IntSeq<T, Src0, Dst...>
			>::type;
		};

		template
		<
			template<typename I, I...> class IntSeq,
			typename T, T... Dst
		>
		struct rev_int_seq
		<
			IntSeq<T>,
			IntSeq<T, Dst...>
		>
		{
			using type = IntSeq<T, Dst...>;
		};

		template<typename IntSeq>
		struct reverse_integer_sequence;

		template
		<
			template<typename I, I...> class IntSeq,
			typename Int, Int... Ints
		>
		struct reverse_integer_sequence< IntSeq<Int,Ints...> >
		{
			using type = typename rev_int_seq
			<
				IntSeq<Int,Ints...>,
				IntSeq<Int>
			>::type;
		};


		template<typename L, typename R>
		struct integer_sequence_cat;
		template
		<
			template<typename I, I...> class IntSeq,
			typename T, T... L, T... R
		>
		struct integer_sequence_cat< IntSeq<T, L...>, IntSeq<T, R...> >
		{
			using type = IntSeq<T, L..., R...>;
		};


		//  O(log n) implementation of make_integer_sequence.
		//  Doesn't exceed template instantiation depth limit
		// of compilers for much longer sequences than a O(n)
		// implementation. Helps with compilation times and
		// compiler memory usage.

		// if From <  To:  [From, To]
		// if From >= To:  [To, From] reversed
		template<typename T, T From, T To>
		struct make_closed_integer_range
		{
			static constexpr auto MidL =
				From < To ?
					From + (To - From)/2
				:
					To+1 + (From - To)/2
			;
			static constexpr auto MidR =
				From < To ? MidL+1 : MidL-1;
			using type = typename integer_sequence_cat
			<
				typename make_closed_integer_range< T, From, MidL >::type,
				typename make_closed_integer_range< T, MidR, To   >::type
			>::type;
		};
		template<typename T, T FromTo>
		struct make_closed_integer_range<T, FromTo, FromTo>
		{
			using type = std::integer_sequence<T,FromTo>;
		};

		// if From <  To:  [From, To)
		// if From >= To:  [To, From) reversed
		template<typename T, T From, T To>
		struct make_halfopen_integer_range
		{
			static constexpr auto From_ = From < To ? From : From-1;
			static constexpr auto To_   = From < To ? To-1 : To;
			using type = typename make_closed_integer_range<T, From_, To_>::type;
		};
		template<typename T, T FromTo>
		struct make_halfopen_integer_range<T, FromTo, FromTo>
		{
			using type = std::integer_sequence<T>;
		};
	}

	// if From <  To:  [From, To)
	// if From >= To:  [To, From) reversed
	template<typename T, T From, T To>
	using make_integer_range = typename impl::make_halfopen_integer_range<T, From, To>::type;

	// [0, Length)
	template<typename T, T Length>
	using make_integer_sequence = make_integer_range<T, 0, Length>;

	// [0, Length)
	template<decltype(sizeof 0) Length>
	using make_index_sequence = make_integer_sequence<decltype(Length), Length>;

	// [0, sizeof...(T))
	template<typename... T>
	using index_sequence_for = make_index_sequence<sizeof...(T)>;

	template<typename Seq>
	using reverse_integer_sequence = typename impl::reverse_integer_sequence<Seq>::type;
	template<typename Seq>
	using reverse_integer_sequence_t [[deprecated]] = reverse_integer_sequence<Seq>;
}
