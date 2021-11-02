#ifndef SQUALL_VMSTD_HPP_
#define SQUALL_VMSTD_HPP_

#include <cstring>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>

#include "squall_vm.hpp"
#include <stdarg.h>

namespace squall {

namespace detail {

template <class T>
void pf(HSQUIRRELVM v, const T* s, ...);

template <>
void inline pf<char>(HSQUIRRELVM v, const char* s, ...)  {
    va_list arglist;
    va_start(arglist, s);
    vprintf(s, arglist);
    va_end(arglist);
}

template <>
void inline pf<wchar_t>(HSQUIRRELVM v, const wchar_t* s, ...)  {
    va_list arglist;
    va_start(arglist, s);
    vwprintf(s, arglist);
    va_end(arglist);
}

}


class VMStd : public VM {
public:
    VMStd(int stack_size = 1024) : VM(stack_size) {
        sqstd_seterrorhandlers(handle());
        sq_setprintfunc(handle(), &detail::pf<SQChar>, &detail::pf<SQChar>);
    }
#ifndef ADDITION_DISABLE
    VMStd(VM& parentVM, int stack_size = 1024) : VM(parentVM,stack_size) {
        //ルートテーブルへ登録するやつはもうやらなくていいはず
        sqstd_seterrorhandlers(handle());
    }
#endif

    void dofile(const SQChar* filename) {
        keeper k(handle());
        sq_pushroottable(handle());
        if (!SQ_SUCCEEDED(sqstd_dofile(handle(), filename, 0, 1))) {
            throw squirrel_error("dofile failed");
        }
    }

#ifndef ADDITION_DISABLE 
    void loadfile(const SQChar* filename) {
        auto v = handle();
        keeper k(handle());
        sq_pushroottable(v);
        if (SQ_SUCCEEDED(sqstd_loadfile(v, filename,1))) {
            sq_push(v, -2);
            if (SQ_SUCCEEDED(sq_call(v, 1, 0, SQTrue))) {
                sq_remove(v, -1); //removes the closure
                return;
            }
            sq_pop(v, 1);//removesthe closure;
            return;
        }

        throw squirrel_error("dofile failed");
    }


#endif

    void dostring(const SQChar* source) {
        keeper k(handle());
        if (!SQ_SUCCEEDED(sq_compilebuffer(handle(), source, static_cast<int>(scstrlen(source) * sizeof(SQChar)), _SC("dostring"), 1))) {
            throw squirrel_error("dostring compile failed");
        }

        sq_pushroottable(handle());

        if (!SQ_SUCCEEDED(sq_call(handle(), 1, SQFalse, SQTrue))) {
            throw squirrel_error("dostring call failed");
        }
    }

private:

};

}

#endif // SQUALL_VMSTD_HPP_
