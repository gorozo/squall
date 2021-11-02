// 2015/01/23 Naoyuki Hirayama

/*!
@file      squall_defvar.hpp
@brief      <brief>

<description>
*/

#ifndef SQUALL_DEFVAR_HPP_
#define SQUALL_DEFVAR_HPP_

#include <squirrel.h>
#include <cstring>
#include "squall_stack_operation.hpp"
#include "squall_defun.hpp"

namespace squall {

namespace detail {

template <typename C,bool Shared>
C* get_class_pointer(HSQUIRRELVM vm, int index = 1) {

    if (Shared) {
        std::shared_ptr<C>* s;
        sq_getinstanceup(vm, 1, (SQUserPointer*)&s, nullptr);

        return s->get();
    }
    else {
        C* p;
        sq_getinstanceup(vm, 1, (SQUserPointer*)&p, nullptr);
        return p;
    }

}

template <class C, class V, bool Shared> inline
SQInteger var_getter(HSQUIRRELVM vm) {
    /*
    C* p = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*)&p, nullptr);
    */
    auto* p = get_class_pointer<C, Shared>(vm);

    
    typedef V C::*M;
    M* mp = nullptr;
    sq_getuserdata(vm, -1, (SQUserPointer*)&mp, nullptr);
    M m = *mp;

    push(vm, p->*m);

    return 1;
}



template <class C, class V, bool Shared> inline
SQInteger var_setter(HSQUIRRELVM vm) {

    /*
  C* p = nullptr;
  sq_getinstanceup(vm, 1, (SQUserPointer*)&p, nullptr);
  */
    auto* p = get_class_pointer<C, Shared>(vm);

		
    typedef V C::*M;
    M* mp = nullptr;
    sq_getuserdata(vm, -1, (SQUserPointer*)&mp, nullptr);
    M m = *mp;

    p->*m = fetch<V, FetchContext::Argument>(vm, 2);

    return 0;
}



template <class M> inline
void bind_accessor(
    HSQUIRRELVM     vm,
    HSQOBJECT       table,
    const string&   name,
    M               var,
    SQFUNCTION      f) {

    sq_pushobject(vm, table);
    sq_pushstring(vm, name.data(), name.length());

    SQUserPointer vp = sq_newuserdata(vm, sizeof(var));
    memcpy(vp, &var, sizeof(var));

    sq_newclosure(vm, f, 1);
    sq_newslot(vm, -3, false);
    sq_pop(vm, 1);
}

template <class C, class V,bool Shared> inline
void defvar_local_const(
    HSQUIRRELVM     vm,
    HSQOBJECT       getter_table,
    const string&   name,
    const V C::*    var) {

    bind_accessor(vm, getter_table, name, var, &var_getter<C, V,Shared>);
}

template <class C, class V,bool Shared> inline
void defvar_local(
    HSQUIRRELVM     vm,
    HSQOBJECT       getter_table,
    HSQOBJECT       setter_table,
    const string&   name,
    V C::*          var
    ) {


    bind_accessor(vm, getter_table, name, var, &var_getter<C, V,Shared>);
    bind_accessor(vm, setter_table, name, var, &var_setter<C, V,Shared>);
}

template <class V, class C>
void defprop(
    HSQUIRRELVM                 vm,
    HSQOBJECT                   getter_table,
    const string&               name,
    std::function<V (const C*)> gf) {

    defun_local(vm, getter_table, name, gf);
}

template <class V, class C>
void defprop(
    HSQUIRRELVM                 vm,
    HSQOBJECT                   getter_table,
    HSQOBJECT                   setter_table,
    const string&               name,
    std::function<V (const C*)> gf,
    std::function<void (C*,V)>  sf) {

    defun_local(vm, getter_table, name, gf);
    defun_local(vm, setter_table, name, sf);
}

}

}

#endif // SQUALL_DEFVAR_HPP_
