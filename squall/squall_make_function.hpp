#ifndef MAKE_FUNCTION_HPP_
#define MAKE_FUNCTION_HPP_

#include <functional>

namespace squall {

namespace detail {

template <typename F>
struct functor_traits;

template <class T, class C, class... A>
struct functor_traits<T (C::*)(A...)> {
    typedef T type(A...);
};

template <class T, class C, class... A>
struct functor_traits<T (C::*)(A...) const> {
    typedef T type(A...);
};
 
//------------------------
template <typename C, bool Shared = false>
struct shared_type {
    typedef C* type;
    typedef const C* ctype;
};

template <typename C>
struct shared_type<C, true> {
    typedef std::shared_ptr<C> type;
    typedef std::shared_ptr<const C> ctype;
};

//------------------------------------

template <typename F,bool Shared = false>
struct function_traits
    : public functor_traits<decltype(&F::operator())> {
};


template <class T, class C, class... A,bool Shared>
struct function_traits<T (C::*)(A...),Shared> {
   // typedef T type(C*, A...);
    typedef T type(typename shared_type<C,Shared>::type, A...);
};

template <class T, class C, class... A,bool Shared>
struct function_traits<T (C::*)(A...) const,Shared> {
    //typedef T type(const C*, A...);
    typedef T type(typename shared_type<C,Shared>::ctype, A...);
};
 
template <class T, class... A,bool Shared>
struct function_traits<T (*)(A...),Shared> {
    typedef T type(A...);
};

}

template <class F,bool Shared = false>
auto //std::function<typename detail::function_traits<F>::type>
to_function(F f) {
    return std::function<typename detail::function_traits<F,Shared>::type>(f);
}

}

#endif // MAKE_FUNCTION_HPP_
