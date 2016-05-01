
#ifndef DIMENSIONAL_IMPL_META_H
#define DIMENSIONAL_IMPL_META_H

#include <type_traits>
#include "mjk/integral_constant"

namespace meta
{
	using std::is_same;

	using mjk::bool_constant;
	using mjk::size_constant;
	using mjk::true_type;
	using mjk::false_type;


	template<typename...>
	struct sequence {};
	template<typename...>
	struct set {};

	template<typename T>
	struct identity
	{
		using type = T;
	};

	template<typename T>
	struct is
	{
		template<typename U>
		struct pred : is_same<T,U> {};
	};

	template<typename...>
	struct always : true_type {};

	template<typename To, typename From>
	struct assign : identity<From> {};

	template<
		template<typename...> class F,
		typename _2nd>
	struct bind_2nd
	{
		template<typename _1st, typename... Rest>
		struct type_template : F<_1st, _2nd, Rest...> {};
	};

	template
	<
		bool Ref,
		template<typename...> class IfFalse,
		template<typename...> class IfTrue
	>
	struct template_switch
	{
		template<typename... Args>
		struct type_template : IfFalse<Args...> {};
	};
	template
	<
		template<typename...> class IfFalse,
		template<typename...> class IfTrue
	>
	struct template_switch< true, IfFalse, IfTrue>
	{
		template<typename... Args>
		struct type_template : IfTrue<Args...> {};
	};

	template<bool... Values>
	using all = is_same<
		sequence< bool_constant<Values>... >,
		sequence< bool_constant<(Values || true)>... >
	>;

	inline constexpr auto not_(  true_type ) { return false_type{}; }
	inline constexpr auto not_( false_type ) { return  true_type{}; }

	template<typename T>
	inline std::decay_t<T> decay( T && );
	// ^ for use in unevaluated contexts only


	template<typename T>
	struct type
	{
		constexpr auto get() const { return T{}; }
	};

	template<typename T, typename U>
	inline constexpr auto operator==( type<T>, type<U> )
	{
		return is_same<T,U>{};
	}

	constexpr auto int_      = type<int>{};
	constexpr auto double_   = type<double>{};


	// operations on sequences of types
	namespace seq
	{
		namespace  impl
		{
			template
			<
				class Seq,
				template<typename...> class Pred
			>
			struct contains;

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred
			>
			struct contains< Seq<Head,Tail...>, Pred >
			{
				static constexpr bool value = Pred<Head>::value ||
					contains<Seq<Tail...>,Pred>::value;
			};

			template
			<
				template<typename...> class Seq,
				template<typename...> class Pred
			>
			struct contains< Seq<>, Pred > : false_type {};


			template<class Seq, typename T>
			struct append;

			template
			<
				template<typename...> class Seq,
				typename... Ts, typename T
			>
			struct append< Seq<Ts...>, T >
			{
				using type = Seq< Ts..., T >;
			};

			
			template<class Seq, typename T>
			struct prepend;

			template
			<
				template<typename...> class Seq,
				typename... Ts, typename T
			>
			struct prepend< Seq<Ts...>, T >
			{
				using type = Seq< T, Ts... >;
			};


			template
			<
				class Seq,
				template<typename...> class Pred,
				template<typename...> class Transf
			>
			struct transform_if;

			template<
				template<typename...> class Seq,
				typename... T,
				template<typename...> class Pred,
				template<typename...> class Transf
			>
			struct transform_if<Seq<T...>, Pred, Transf>
			{
				using type = Seq<
					typename template_switch< Pred<T>::value, identity, Transf>::
						template type_template<T>::type
					...
				>;
			};


			template
			<
				class Seq,
				template<typename...> class Compare
			>
			struct is_uset;

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred
			>
			struct is_uset< Seq<Head,Tail...>, Pred > : bool_constant<
				!contains<Seq<Tail...>, is<Head>::template pred>::value &&
					is_uset< Seq<Tail...>, Pred >::value
			> {};

			template
			<
				template<typename...> class Seq,
				template<typename...> class Pred
			>
			struct is_uset< Seq<>, Pred > : true_type {};  // empty sequence is always a set


			template
			<
				class Seq,
				template<typename...> class Pred,
				bool Found = false,
				typename FoundWhat = void
			>
			struct find_if;

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct find_if< Seq<Head,Tail...>, Pred, false, FoundWhat >
				: find_if< Seq<Tail...>, Pred, Pred<Head>::value, Head > {};

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct find_if< Seq<Head,Tail...>, Pred, true, FoundWhat > : true_type
			{
				using type = FoundWhat;
				static constexpr auto tail_size =
					size_constant< 1 + sizeof...(Tail) >{};
			};

			template
			<
				template<typename...> class Seq,
				template<typename...> class Pred,
				bool Found,
				typename FoundWhat
			>
			struct find_if< Seq<>, Pred, Found, FoundWhat > : false_type
			{
				static constexpr auto tail_size =
					size_constant<0>{};
			};


			template
			<
				class Seq,
				template<typename...> class Pred,
				bool Found = true,
				typename FoundWhat = void
			>
			struct remove_if;

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct remove_if< Seq<Head,Tail...>, Pred, true, FoundWhat >
				: remove_if< Seq<Tail...>, Pred, Pred<Head>::value, Head > {};

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct remove_if< Seq<Head,Tail...>, Pred, false, FoundWhat >
			{
				using type = typename prepend<
					typename
						remove_if< Seq<Tail...>, Pred, Pred<Head>::value, Head >
					::type,
					FoundWhat >::type;
			};

			template
			<
				template<typename...> class Seq,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct remove_if< Seq<>, Pred, true, FoundWhat >
			{
				using type = Seq<>;
			};

			template
			<
				template<typename...> class Seq,
				template<typename...> class Pred,
				typename FoundWhat
			>
			struct remove_if< Seq<>, Pred, false, FoundWhat >
			{
				using type = Seq<FoundWhat>;
			};

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail
			>
			auto front( Seq<Head,Tail...> )
			{
				return identity<Head>{};
			}
		}

		template<class Seq, typename T>
		using append = typename impl::append<Seq,T>::type;

		template
		<
			class Seq,
			template<typename...> class Pred
		>
		using contains = impl::contains<Seq,Pred>;

		template
		<
			class Seq,
			template<typename...> class Pred,
			template<typename...> class Transf
		>
		using transform_if = typename impl::transform_if<Seq,Pred,Transf>::type;

		template
		<
			class Seq,
			template<typename...> class Transf
		>
		using transform = transform_if< Seq, always, Transf >;

		template
		<
			class Seq,
			template<typename...> class Compare = is_same
		>
		using is_uset = impl::is_uset<Seq,Compare>;

		template
		<
			class Seq,
			template<typename...> class Pred
		>
		using find_if = impl::find_if<Seq,Pred>;

		template
		<
			class Seq,
			typename What
		>
		using find = find_if< Seq, is<What>::template pred >;

		template
		<
			class Seq,
			template<typename...> class Pred
		>
		using remove_if = typename impl::remove_if< Seq, Pred >::type;

		template
		<
			class Seq,
			typename What
		>
		using remove = remove_if< Seq, is<What>::template pred >;

		template
		<
			template<typename...> class Seq,
			typename... Ts
		>
		inline constexpr auto size_f( Seq<Ts...> )
		{
			return size_constant< sizeof...(Ts) >{};
		}
		template<class Seq>
		using size = decltype( size_f(Seq{}) );

		
		template<class Seq>
		using front = typename decltype( impl::front(Seq{}) )::type;
		template<class Seq>
		inline constexpr auto front_f( Seq ) { return type<front<Seq>>{}; }
	}

	// operations on unordered sets of types
	namespace uset
	{
		namespace impl
		{
			template
			<
				class Seq, typename T,
				template<typename...> class Compare,
				template<typename...> class Assign
			>
			struct insert
			{
			private:
				template<typename U> struct equiv_to_T : Compare<U,T> {};
				template<typename U> struct assign_T : Assign<U,T> {};
				using s = Seq;
			public:
				using type = std::conditional_t<
					// if there's a T equivalent..
					seq::contains< s, equiv_to_T >::value,
						// ..invoke assignment
						seq::transform_if< s, equiv_to_T, assign_T >,
						// ..else just add T into the set
						seq::append< s, T >
				>;
			};

			template
			<
				class Seq,
				template<typename...> class Compare,
				template<typename...> class Assign
			>
			struct make;

			template
			<
				template<typename...> class Seq,
				typename Head, typename... Tail,
				template<typename...> class Pred,
				template<typename...> class Combine
			>
			struct make< Seq<Head,Tail...>, Pred, Combine >
			{
			private:
				using tailset = typename
					make< Seq<Tail...>, Pred, Combine >
				::type;
			public:
				using type = insert< tailset, Head, Pred, Combine >;
			};

			template
			<
				template<typename...> class Seq,
				template<typename,typename> class Pred,
				template<typename,typename> class Combine
			>
			struct make< Seq<>, Pred, Combine>
			{
				using type = Seq<>;
			};


			template<class SetA, class SetB>
			struct equal;

			template
			<
				template<typename...> class Set,
				typename... A, typename... B
			>
			struct equal< Set<A...>, Set<B...> > : bool_constant<
				sizeof...(A) == sizeof...(B) &&
				all< seq::contains< Set<B...>,
					is<A>::template pred >::value...
				>::value
			>
			{
			#ifdef VERYDEBUG
				static_assert(
					seq::is_uset<Set<A...>>::value &&
					seq::is_uset<Set<B...>>::value,
					"no multisets, please" );
			#endif
			};
		}

		template
		<
			class Seq,
			template<typename...> class Compare = is_same,
			template<typename...> class Assign = assign
		>
		using make = typename impl::make<Seq, Compare, Assign>::type;

		template
		<
			class Set, typename T,
			template<typename...> class Compare = is_same,
			template<typename...> class Assign = assign
		>
		using insert = typename impl::insert<Set, T, Compare, Assign>::type;

		template
		<
			class Set,
			template<typename...> class Transf
		>
		using transform = seq::transform< Set, Transf >;

		template<class SetA, class SetB>
		using equal = impl::equal<SetA, SetB>;

		template<class SetA, class SetB>
		inline constexpr auto equal_f( SetA, SetB )
		{
			return equal< std::decay_t<SetA>, std::decay_t<SetB> >{};
		}

		template
		<
			class Set,
			template<typename...> class Pred
		>
		using find_if = seq::find_if<Set, Pred>;

		template
		<
			class Set,
			template<typename...> class Pred
		>
		using remove_if = seq::remove_if<Set, Pred>;
	}
}

#endif
