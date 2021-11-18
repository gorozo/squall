// -*- coding: utf-8-unix -*-

#ifndef SQUALL_STACK_OPERATION_HPP_
#define SQUALL_STACK_OPERATION_HPP_

#include <squirrel.h>
#include "squall_exception.hpp"
#include "squall_closure_decl.hpp"

#ifndef ADDITION_DISABLE
#include <type_traits>
#endif

//#include "demangle.hpp"


namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// construct object on stack

template <class T>
void construct_object(HSQUIRRELVM vm, const T& v) {
    SQUserPointer p = sq_newuserdata(vm, sizeof(T*));
    *((T**)p) = new T(v);
    SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
        delete *((T**)p);
        return 0;
    };
    sq_setreleasehook(vm, -1, hook);
}

////////////////////////////////////////////////////////////////
// push
template <class T> inline
void push_aux(HSQUIRRELVM vm, T v) {


    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
        sq_remove(vm, -2);

        sq_setinstanceup(vm, -1, new T(v)); //コピーコンストラクタ
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            delete (T*)p;
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    } else {
        SQUserPointer p = sq_newuserdata(vm, sizeof(T));
        new(p) T(v);
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            ((T*)p)->~T();
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    }

}
template <class T> inline
void push_aux(HSQUIRRELVM vm, T* v) { //vはC++側のポインタを想定してる?
    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
		sq_setinstanceup(vm, -1, v);
        sq_remove(vm, -2);
    } else {
        sq_pushuserpointer(vm, v);
    }
}


template <class R, class... A>
void push_closure(HSQUIRRELVM vm, std::function<R (A...)> v);

template <class R, class... A> inline
void push_aux(HSQUIRRELVM vm, std::function<R (A...)> v) {
    push_closure(vm, v);
}

template <> inline
void push_aux<std::int8_t>(HSQUIRRELVM vm, std::int8_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int16_t>(HSQUIRRELVM vm, std::int16_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int32_t>(HSQUIRRELVM vm, std::int32_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int64_t>(HSQUIRRELVM vm, std::int64_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint8_t>(HSQUIRRELVM vm, std::uint8_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint16_t>(HSQUIRRELVM vm, std::uint16_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint32_t>(HSQUIRRELVM vm, std::uint32_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint64_t>(HSQUIRRELVM vm, std::uint64_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<float>(HSQUIRRELVM vm, float v) {
    sq_pushfloat(vm, v);
}
template <> inline
void push_aux<bool>(HSQUIRRELVM vm, bool v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <> inline
void push_aux<string_wrapper>(HSQUIRRELVM vm, string_wrapper v) {
    sq_pushstring(vm, v, -1);
}

#ifdef ADDITION_DISABLE

template <class T>
void push(HSQUIRRELVM vm, const T& v) {
    push_aux(vm, wrap_type(v));
}

#else

//インスタンス生成してプッシュ
template <class T>
void push_returnvalue(HSQUIRRELVM vm, const T& v) {
    push_aux(vm, wrap_type(v));
}

//参照をプッシュ


template <class T> inline
void push_auxr(HSQUIRRELVM vm, T& v) {

    push_aux(vm, &v);
}


template <class T> inline
void push_auxr(HSQUIRRELVM vm, T*& v) {
    push_aux(vm, v);
}

/*
template <class T> inline
void push_auxr(HSQUIRRELVM vm, std::shared_ptr<T>& v) {
    push_aux(vm, v.get());
}
*/


template <class R, class... A> inline
void push_auxr(HSQUIRRELVM vm, std::function<R(A...)> v) {
    push_closure(vm, v);
}

template <> inline
void push_auxr<std::int8_t>(HSQUIRRELVM vm, std::int8_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::int16_t>(HSQUIRRELVM vm, std::int16_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::int32_t>(HSQUIRRELVM vm, std::int32_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::int64_t>(HSQUIRRELVM vm, std::int64_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::uint8_t>(HSQUIRRELVM vm, std::uint8_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::uint16_t>(HSQUIRRELVM vm, std::uint16_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::uint32_t>(HSQUIRRELVM vm, std::uint32_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<std::uint64_t>(HSQUIRRELVM vm, std::uint64_t& v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_auxr<float>(HSQUIRRELVM vm,float& v) {
    sq_pushfloat(vm, v);
}
template <> inline
void push_auxr<bool>(HSQUIRRELVM vm, bool& v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <> inline
void push_auxr<string_wrapper>(HSQUIRRELVM vm, string_wrapper& v) {
    sq_pushstring(vm, v, -1);
}

template<> inline
void push_auxr(HSQUIRRELVM vm, SQObject& obj) {
    sq_pushobject(vm, obj);
 
}

template <class T>
void push(HSQUIRRELVM vm, T& v) {

   //wrap_typeはアドレスを保持しない
   push_auxr(vm, v);
}



#endif



////////////////////////////////////////////////////////////////
// fetch
enum class FetchContext {
    Argument,
    ReturnValue,
    TableEntry,
    YieldedValue,
};

template <FetchContext> string fetch_context_string();

template <>
inline string fetch_context_string<FetchContext::Argument>() {
    return _SC("argument");
}

template <>
inline string fetch_context_string<FetchContext::ReturnValue>() {
    return _SC("return value");
}

template <>
inline string fetch_context_string<FetchContext::TableEntry>() {
    return _SC("table entry");
}

template <>
inline string fetch_context_string<FetchContext::YieldedValue>() {
    return _SC("yielded value");
}

template <FetchContext FC>
void check_argument_type(HSQUIRRELVM vm, SQInteger index, SQObjectType t) {
    SQObjectType at = sq_gettype(vm, index);
    if (at != t) {
        throw squirrel_error(
            fetch_context_string<FC>() + _SC(" must be ") + get_type_text(t) +
            _SC(", actual value is ") + get_type_text(at));
    }
}

template <class T, FetchContext FC, class F>
T getdata(HSQUIRRELVM vm, SQInteger index, SQObjectType t, F f) {
    check_argument_type<FC>(vm, index, t);
    T r;
    f(vm, index, &r);
    return r;
}

template <class T, FetchContext FC>
struct Fetch { 
public:
    static T doit(HSQUIRRELVM vm, SQInteger index) {

#ifndef ADDITION_DISABLE
        auto t = sq_gettype(vm, index);
        if (t == OT_INSTANCE) {
            //OT_INSTANCE --> T
            SQUserPointer rr;
            sq_getinstanceup(vm, index, &rr, NULL);
            return *((T*)rr);
        }
#endif
            //OT_USERDATA --> T
            check_argument_type<FC>(vm, index, OT_USERDATA);
            SQUserPointer r;
            sq_getuserdata(vm, index, &r, NULL);
            //return **((T**)r);
            return *((T*)r); //shared_ptrの場合はこれでいいみたいだけど。。
        
    }
};




#ifndef ADDITION_DISABLE
template<FetchContext FC>
struct Fetch<tagSQObject, FC> {
    typedef tagSQObject T;
    static T doit(HSQUIRRELVM vm, SQInteger index) {
        auto t = sq_gettype(vm, index);

        if (t != OT_INSTANCE && t != OT_TABLE) {
            throw squirrel_error("must be OT_INSTANCE or OT_TABLE.");
        }
        
        T tag;
        if (!SQ_SUCCEEDED(sq_getstackobj(vm, index, &tag))) {
            throw squirrel_error("error");
        }

        return tag;
    }
};
#endif

template <class T, FetchContext FC>
struct Fetch<T*, FC> { 
    static T* doit(HSQUIRRELVM vm, SQInteger index)
    {
        HSQOBJECT sqo;
        if (klass_table(vm).find_klass_object<T>(sqo)) {
            //OT_INSTANCE --> T*

            check_argument_type<FC>(vm, index, OT_INSTANCE);
            SQUserPointer r;
            sq_getinstanceup(vm, index, &r, NULL);


#if 0//#ifndef ADDITION_DISABLE　//-->コンストラクタ呼び出し時に移動
            

            if (!(std::is_pointer<T>::value||std::is_const<T>::value)) { //仮。多分判定方法間違ってる
                if (r == nullptr) {
               
                    //追加 
                    //<暫定仕様> "constructor"を関数定義する必要がある。
                    //その後、テーブル関数がよばれるので、コンストラクタが２回呼ばれる。
                    r = new T(); //後でテーブル定義の関数（コンストラクタ）も呼ばれる。
                    sq_setinstanceup(vm, index, r);
                    SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
                        delete (T*)p;
                        return 0;
                    };
                    sq_setreleasehook(vm, index, hook);

                }
                
            }
#endif
            return (T*)r;
        } else {
            //OT_USERPOINTER --> T*
            return (T*)getdata<SQUserPointer, FC>(
                vm, index, OT_USERPOINTER, sq_getuserpointer);
        }
    }
};



/*
template <class T, FetchContext FC>
struct Fetch<std::shared_ptr<T>, FC> {
public:
   // static  std::shared_ptr<T> doit(HSQUIRRELVM vm, SQInteger index) {
     static  T* doit(HSQUIRRELVM vm, SQInteger index) {
        // Fetch<T, FC>::doit(vm, index);
         return nullptr; //kari
    }
};
*/



template <class R, class... A, FetchContext FC>
struct Fetch<std::function<R (A...)>, FC> {
    static std::function<R (A...)> doit(HSQUIRRELVM vm, SQInteger index) {
        auto t = sq_gettype(vm, index);
        if (t == OT_NATIVECLOSURE || t == OT_CLOSURE) {
            return Closure<R (A...)>(vm, index);
        } else {
            throw squirrel_error("value must be closure or native closure");
        }
    }
};

template <FetchContext FC, class Int>
struct FetchInt {
    static Int doit(HSQUIRRELVM vm, SQInteger index) {
        return static_cast<Int>(
            getdata<SQInteger, FC>(vm, index, OT_INTEGER, sq_getinteger));
    }
};

template <FetchContext FC>
struct Fetch<std::int8_t, FC> : public FetchInt<FC, std::int8_t> {};
template <FetchContext FC>
struct Fetch<std::int16_t, FC> : public FetchInt<FC, std::int16_t> {};
template <FetchContext FC>
struct Fetch<std::int32_t, FC> : public FetchInt<FC, std::int32_t> {};
template <FetchContext FC>
struct Fetch<std::int64_t, FC> : public FetchInt<FC, std::int64_t> {};
template <FetchContext FC>
struct Fetch<std::uint8_t, FC> : public FetchInt<FC, std::uint8_t> {};
template <FetchContext FC>
struct Fetch<std::uint16_t, FC> : public FetchInt<FC, std::uint16_t> {};
template <FetchContext FC>
struct Fetch<std::uint32_t, FC> : public FetchInt<FC, std::uint32_t> {};
template <FetchContext FC>
struct Fetch<std::uint64_t, FC> : public FetchInt<FC, std::uint64_t> {};

template <FetchContext FC>
struct Fetch<float, FC> {
    static float doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<SQFloat, FC>(
            vm, index, OT_FLOAT, sq_getfloat);
    }
};

template <FetchContext FC>
struct Fetch<bool, FC> {
    static bool doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<SQBool, FC>(
            vm, index, OT_BOOL, sq_getbool);
    }
};

template <FetchContext FC>
struct Fetch<string_wrapper, FC> {
    static string_wrapper doit(HSQUIRRELVM vm, SQInteger index)
    {
        return getdata<const SQChar*, FC>(
            vm, index, OT_STRING, sq_getstring);
    }
};



template <class T, FetchContext FC>
typename wrapped_type<T>::wrapper_type
fetch(HSQUIRRELVM vm, SQInteger index) {
    return Fetch<typename wrapped_type<T>::wrapper_type, FC>::doit(vm, index);
}


}

}

#endif // SQUALL_STACK_OPERATION_HPP_
