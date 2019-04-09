#include <type_traits>

/* Forward declaration required by following traits. */
template <typename I, I... is>
struct set;

template <typename I, I... is>
struct list;

/**
 * Since there can only be one parameter pack in a variadic template,
 * these traits only make statements about properties of such a single parameter pack,
 * or its relation to a definite number of other parameters - usually a single one.
 * Statements about relations to other parameter packs can only be made by class templates.
 */
namespace type_traits
{
    /* contains */
    template <typename I>
    constexpr bool contains(I) noexcept {
        return false;
    }

    template <typename I, I i, I... is>
    constexpr bool contains(I j) noexcept {
        return i == j || contains<I, is...>(j);
    }

    /* set */
    template <typename I>
    constexpr bool is_set() noexcept {
        return true;
    };

    template <typename I, I i, I... is>
    constexpr bool is_set() noexcept {
        return !contains<I, is...>(i) && is_set<I, is...>();
    };

    template <typename I, I... is>
    constexpr const bool is_set_v {
        is_set<I, is...>()
    };

    /* everse list */
    template <typename I>
    constexpr bool is_everse_list() noexcept {
        return true;
    }

    template <typename I, I i>
    constexpr bool is_everse_list() noexcept {
        return true;
    }

    template <typename I, I i, I j, I... ks>
    constexpr bool is_everse_list() noexcept {
        return i <= j && is_everse_list<I, j, ks...>();
    }

    template <typename I, I... is>
    constexpr const bool is_everse_list_v {
        is_everse_list<I, is...>()
    };

    /* inverse list */
    template <typename I>
    constexpr bool is_inverse_list() noexcept {
        return true;
    }

    template <typename I, I i>
    constexpr bool is_inverse_list() noexcept {
        return true;
    }

    template <typename I, I i, I j, I... ks>
    constexpr bool is_inverse_list() noexcept {
        return i >= j && is_inverse_list<I, j, ks...>();
    }

    template <typename I, I... is>
    constexpr const bool is_inverse_list_v {
        is_inverse_list<I, is...>()
    };

    /* list */
    template <typename I, I... is>
    constexpr bool is_list() noexcept { return is_everse_list_v<I, is...> || is_inverse_list_v<I, is...>; }

    template <typename I, I... is>
    constexpr const bool is_list_v { is_list<I, is...>() };
}

template <typename I, I... is>
struct bag // or multi_set
{
    static_assert(sizeof...(is) == 0, "Empty bag is not empty.");

    using cdr = bag<I>;

    using set_t = set<I>;

    constexpr static set_t to_set() noexcept {
        return {};
    }
};

template <typename I, I i, I... is>
struct bag<I, i, is...> : protected bag<I, is...>
{
    constexpr static I car { i };

    using cdr = bag<I, is...>;

    using cdr_set = typename cdr::set_t;

    using set_t = std::conditional_t<
        type_traits::contains<I, is...>(i),
        cdr_set,
        typename cdr_set::template prepend<i>
    >;

    constexpr static set_t to_set() noexcept {
        return {};
    }
};

namespace type_traits
{   // This is presumably better than to_set(), as no object is constructed (yet).
    template <typename I, I... is>
    using set_t = typename bag<I, is...>::set_t;
}

    template<typename I>
    using empty_set = set<I>;

/* This general case is only instantiated for the special case of the empty set. */
template <typename I, I... is>
struct set
{
    static_assert(0 == sizeof...(is), "Empty set is not empty.");

    constexpr static std::size_t size() noexcept {
        return 0;
    }

    constexpr static bool empty() noexcept {
        return true;
    }

    using empty_set = empty_set<I>;

    /* This empty set does not contain any item. */
    constexpr static bool contains(I j) noexcept {
        return false;
    }

    /* This empty set is a subset of any set (of the same item type). */
    template <I... js>
    constexpr static bool subset_of() noexcept {
        return true;
    }

    constexpr static bool is_everse_list { true };

    constexpr static bool is_inverse_list { true };

    constexpr static bool is_list { true };

    /* This empty set cannot be equal to a non-empty set of 'js'. */
    template <I... js>
    constexpr static bool equals() noexcept {
        return 0 == sizeof...(js);
    }

    template <I... js>
    constexpr bool operator ==(set<I, js...> const &) const noexcept {
        return equals<js...>();
    }

    template <I... js>
    constexpr bool operator !=(set<I, js...> const &) const noexcept {
        return !equals<js...>();
    }

    /* Add a single element to this empty set. */
    template <I j>
    using prepend = set<I, j>;

    template <I j>
    using append = set<I, j>;

    /* The union of this empty set with another set is the other set. */
    template <I... js>
    using union_t = set<I, js...>;

    template <I... js>
    constexpr union_t<js...> operator +(set<I, js...> const & that) const noexcept {
        return that;
    }

    /* Subtracting a single element from this empty set does not change it. */
    template <I j>
    using subtract = empty_set;
    
    /* Subtracting this empty set from another set does not change that set. */
    template <I... js>
    using difference = set<I, js...>;

    template <I... js>
    constexpr difference<js...> operator -(set<I, js...> const & that) const noexcept {
        return that;
    }

    template <I... js>
    using intersection = empty_set;

    template <I... js>
    constexpr intersection<js...> operator *(set<I, js...> const &) const noexcept {
        return {};
    }

    template <I supremum>
    using smaller_than = empty_set;

    template <I supremum>
    using larger_than = empty_set;

    using quick_sort_t = empty_set;

    constexpr static quick_sort_t quick_sort() noexcept { return {}; }
};

template <typename I, I i>
using element = set<I, i>;

template <typename I, I i, I... is>
struct set<I, i, is...> : protected set<I, is...>
{
    static_assert(type_traits::is_set<I, i, is...>, "Set is not a set.");
    static_assert(!type_traits::contains<I, is...>(i), "Duplicate in set.");

    constexpr static std::size_t size() noexcept {
        return 1 + sizeof...(is);
    }

    constexpr static bool empty() noexcept {
        return false;
    }

    using empty_set = empty_set<I>;

    template <std::size_t n>
    constexpr static I get() noexcept {
        static_assert(n < size(), "Index out of range.");
        if constexpr (n == 0) {
            return i;
        } else {
            return set<I, is...>::template get<n-1>();
        }
    }

    constexpr static I head { i };

    using tail = set<I, is...>;

    constexpr static bool contains(I j) noexcept {
        return type_traits::contains<I, i, is...>(j);
    }

    constexpr static bool is_everse_list { type_traits::is_everse_list_v<I, i, is...> };

    constexpr static bool is_inverse_list { type_traits::is_inverse_list_v<I, i, is...> };

    constexpr static bool is_list { type_traits::is_list_v<I, i, is...> };

    /**
     * There is no guarantee that 'js' form a set, but a subset can be in anything.
     */
    template <I... js>
    constexpr static bool subset_of() noexcept {
        return type_traits::contains<I, js...>(i) 
            && set<I, is...>::template subset_of<js...>();
    }

    /* Equal sets are subsets of each other. */
    template <I... js>
    constexpr static bool equals() noexcept {
        return subset_of<js...>()
            && set<I, js...>::template subset_of<i, is...>();
    }

    template <I... js>
    constexpr bool operator ==(set<I, js...> const &) const noexcept {
        return equals<js...>();
    }

    template <I... js>
    constexpr bool operator !=(set<I, js...> const &) const noexcept {
        return !equals<js...>();
    }

    template <I j>
    using append = std::conditional_t<contains(j),
        set<I, i, is...>,
        set<I, i, is..., j>
    >;

    template <I j>
    using prepend = std::conditional_t<contains(j),
        set<I, i, is...>,
        set<I, j, i, is...>
    >;

    // TODO Make sure that 'js' form a set.
    template <I... js>
    using union_t = typename set<I, is...>::template union_t<js...>::template prepend<i>;

    template <I... js>
    constexpr union_t<js...> operator +(set<I, js...> const &) const noexcept {
        return {};
    }

    /* As each element of a set is unique, it only needs to be eliminated once. */
    template <I j>
    using subtract = std::conditional_t<i == j,
        set<I, is...>,
        typename set<I, is...>::template subtract<j>::template prepend<i>
    >;

    /**
     * The difference that remains when this set is subtracted from the one of 'js'.
     * The elements of this set are eliminated from 'js' one by one, starting with 'i'.
     */
    template <I... js>
    using difference = typename set<I, js...>::template subtract<i>::template difference<is...>;

    template <I... js>
    constexpr typename set<I, js...>::template difference<i, is...>
    operator -(set<I, js...> const &) const noexcept {
        return {};
    }

    template <I... js>
    using intersection = std::conditional_t<type_traits::contains<I, js...>(i),
        typename set<I, is...>::template intersection<js...>::template prepend<i>,
        typename set<I, is...>::template intersection<js...>
    >;

    template <I... js>
    constexpr intersection<js...> operator *(set<I, js...> const &) const noexcept {
        return {};
    }

    template <I supremum>
    using smaller_than = std::conditional_t<i < supremum,
        typename set<I, is...>::template smaller_than<supremum>::template prepend<i>,
        typename set<I, is...>::template smaller_than<supremum>
    >;
    
    template <I infimum>
    using larger_than = std::conditional_t<infimum < i,
        typename set<I, is...>::template larger_than<infimum>::template prepend<i>,
        typename set<I, is...>::template larger_than<infimum>
    >;

    constexpr static auto quick_sort() noexcept {
        return smaller_than<i>::quick_sort() + set<I, i>{} + larger_than<i>::quick_sort();
    }

};

using namespace type_traits;

int main() {
    using set_t = set<short, -1, 3, 4>;
    using union_t = set_t::union_t<0, 3, 2>;
    constexpr const auto some_set(set_t{} + set<short, -2, -5>{});
    constexpr const bool subset { set_t::subset_of<-1, 3, 5, 4>() };
    constexpr const bool equal { set_t::equals<3, 4, -1>() };
    constexpr bool const a { set_t::contains(3) };
    constexpr bool const b { set_t::contains(4) };
    constexpr bool const c { contains<unsigned, 1, 3, 4, 7, 0>(1) };

    constexpr auto sorted(set<int, 4, 1, 7, 3, 2, 6, 5>::quick_sort());

    return a && b && c && subset && equal 
        && set<int, 2, 1, 0>::get<2>() == 0
        && union_t::contains(2) && !union_t::contains(-2)
        && some_set.contains(-5) && !some_set.contains(5) 
        && is_set<int, 1, 2, 3>() && !is_set_v<int, 1, 1, 2>
        && is_everse_list_v<int, 1, 1, 2> && !is_everse_list_v<int, 3, 2, 2, 0>
        && !is_inverse_list_v<int, 1, 1, 2> && is_inverse_list_v<int, 3, 2, 2, 0>
        && is_list_v<int, 1, 1, 2> && is_list_v<int, 3, 2, 1> && !is_list_v<int, 3, 1, 2>
        && set<int, 1, 2>{} + set<int, 3, 2>{} == set<int, 2, 3, 1>{}
        && set<int, 1, 2>{} + set<int, 3, 2>{} != set<int, 3, 1>{}
        && set<int, 1, 3, 2>{} - set<int, 2, 1>{} == set<int, 3>{}
        && type_traits::set_t<int>{} == set<int>{} 
        && type_traits::set_t<int, 0, 1, 1>{} == set<int, 1, 0>{}
        && set<int, 1, 2, 3>{} * set<int, 4, 3, 2>{} == set<int, 3, 2>{}
        && set<int, 1, 2, 4, 3>::smaller_than<2>{} == set<int, 1>{}
        && set<int, 1, 2, 4, 3>::larger_than<2>{} == set<int, 3, 4>{}
        && sorted.is_everse_list && sorted == set<int, 7, 1, 2, 3, 4, 6, 5>{};
}
