#ifndef SQUALL_VM_HPP_
#define SQUALL_VM_HPP_

#include <cassert>
#include <functional>
#include <cstdint>
#include "squall_klass_table.hpp"
#include "squall_exception.hpp"
#include "squall_stack_operation.hpp"
#include "squall_call.hpp"
#include "squall_defun.hpp"
#include "squall_make_function.hpp"
#include "squall_table_base.hpp"
#include "squall_closure.hpp"

namespace squall {

namespace detail {

class VMImp { // for destruct order
public:

#ifdef ADDITION_DISABLE
    VMImp(int stack_size = 1024) { vm_ = sq_open(stack_size); }
#else
    VMImp(int stack_size = 1024) { 
        vm_ = sq_open(stack_size);
    }
    VMImp(HSQUIRRELVM parentVm, int stack_size = 1024) {
        parent_ = parentVm;
        vm_ = sq_newthread(parentVm, stack_size); //子VMとして作成。rootTable　共通

        sq_resetobject(&vm_obj);
        sq_getstackobj(parentVm, -1, &vm_obj);
        sq_addref(parentVm, &vm_obj);
        
        sq_pop(parentVm, 1);

    }
#endif

    ~VMImp() {
#ifndef ADDITION_DISABLE
        if (parent_) {
            sq_release(parent_, &vm_obj);
            return;
        }
#endif
        sq_close(vm_);
    }
    HSQUIRRELVM handle() { return vm_; }
private:
    HSQUIRRELVM vm_;
#ifndef ADDITION_DISABLE
    HSQUIRRELVM parent_ = nullptr;
    HSQOBJECT vm_obj; //OT_THREAD実体

#endif

};

}

////////////////////////////////////////////////////////////////
// VM interface
class VM {
public:
    VM(int stack_size = 1024) : imp_(stack_size) {
        HSQUIRRELVM vm = handle();

#ifdef ADDITION_DISABLE
        sq_setforeignptr(vm, &klass_table_);
#else
        sq_setsharedforeignptr(vm, &klass_table_); //子VMで共有する。
#endif

        sq_pushroottable(vm);
        sq_getstackobj(vm, -1, &root_);
        sq_pop(vm, -1);

        root_table_.reset(new TableBase(vm, root_));
    }

#ifndef ADDITION_ENABLE
    VM(VM& p, int stack_size = 2024) :parentVM(&p),imp_(p.handle(), stack_size) {
        //クラステーブルとルートテーブルは親のを使う
      //  auto vm = handle();
     //   sq_setsharedforeignptr(vm, &klass_table());
    }
    ~VM() {}

#else
    ~VM() { sq_setforeignptr(handle(), 0); } 
#endif

#ifdef ADDITION_DISABLE
#ifdef _MSC_VER
    //Avoiding Visual C ++ bugs
    template <class R, typename... Args>
    R call(const string& name, Args... args) {
        return root_table_.get()->call<R>(name, args...);
    }
#else
    template <class R, typename... Args>
    R call(const string& name, Args... args) {
        return root_table_->call<R>(name, args...);
    }
#endif

    template <class F>
    void defun(const string& name, F f) {
        root_table_->defun(name, f);
    }

    void defraw(const string& s, SQInteger (*f)(HSQUIRRELVM)) {
        root_table_->defraw(s, f);
    }

    template <class... T>
    Coroutine co_call(const string& name, T... args) {
        return root_table_->co_call(name, args...);
    }


    TableBase& root_table() { return *root_table_.get(); }
    KlassTable& klass_table() { return klass_table_; }
#else

#ifdef _MSC_VER
    //Avoiding Visual C ++ bugs
    template <class R, typename... Args>
    R call(const string& name, Args... args) {
        return root_table().call<R>(handle(),name, args...);
    }
#else
    template <class R, typename... Args>
     call(const string& name, Args... args) {
        return root_table().call<R>(handle(),name, args...);
    }
#endif

    template <class F>
    void defun(const string& name, F f) {
        root_table().defun(handle(),name, f);
    }

    void defraw(const string& s, SQInteger(*f)(HSQUIRRELVM)) {
        root_table().defraw(handle(),s, f);
    }

    template <class... T>
    Coroutine co_call(const string& name, T... args) {
        return root_table().co_call(handle(),name, args...);
    }


    TableBase& root_table() { return parentVM? parentVM->root_table():*root_table_.get(); }
    KlassTable& klass_table() { return parentVM?parentVM->klass_table():klass_table_; }

    VM* rootVM() {
        auto* ret = this;
        while (ret->parentVM) {
            ret = ret->parentVM;
        }
        return ret;
    }
    HSQUIRRELVM rootHandle() {
        return rootVM()->handle();
    }

#endif

    HSQUIRRELVM handle() { return imp_.handle(); }

private:
    detail::VMImp               imp_;
    HSQOBJECT                   root_;
    KlassTable                  klass_table_;
    std::unique_ptr<TableBase>  root_table_;

#ifndef ADDITION_DISABLE
    VM* parentVM = nullptr;
#endif
    
};

}

#endif // SQUALL_VM_HPP

