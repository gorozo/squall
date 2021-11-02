#ifndef SQUALL_KLASS_HPP_
#define SQUALL_KLASS_HPP_

#include <functional>

#include "squall_vm.hpp"
#include "squall_defun.hpp"
#include "squall_defvar.hpp"

namespace squall {

template <class C, bool  Shared = false,class Base = void>
class Klass {
public:
    Klass(VM& vm, const string& name) : vm_(vm){
#ifdef ADDITION_DISABLE
        imp_ = vm.klass_table().add_klass<C, Base>(vm_.handle(), name);
#else 
        imp_ = vm.klass_table().add_klass<C, Base>(vm_.rootHandle(), name);
        //コンストラクタは自動で追加
        defctor();
#endif

    }
    ~Klass() { imp_.lock()->close(); }

    Klass(const Klass&) = delete;
    Klass(const Klass&&) = delete;
    void operator=(const Klass&) = delete;
    void operator=(const Klass&&) = delete;
    
    template <class F>
    Klass& func(const SQChar* name, F f) {
        func(string(name), f);
        return *this;
    }

    template <class F>
    Klass& func(const string& name, F f) {
        detail::defun_local(
            vm_.handle(),
            imp_.lock()->get_klass_object(),
            name,
            to_function<F,Shared>(f));
        return *this;
    }
#ifndef ADDITION_DISABLE
    Klass& defctor() {
        //デフォルトコンストラクタでnewするデリゲートを登録する
        detail::defctor<C>(
            vm_.handle(),
            imp_.lock()->get_klass_object()
        );
        return *this;
    }
#endif

    template <class V>
    Klass& var(const string& name, V C::* r) {
        auto& imp = *imp_.lock();
        detail::defvar_local<C,V,Shared>(
            vm_.handle(),
            imp.get_getter_table(),
            imp.get_setter_table(),
            name,
            r
            );
        return *this;
    }

    template <class V>
    Klass& var(const string& name, const V C::* r) {
        detail::defvar_local_const<C,V,Shared>(
            vm_.handle(),
            imp_.lock()->get_getter_table(),
            name,
            r);
        return *this;
    }

    template <class GF>
    Klass& prop(const string& name, GF gf) {
        auto& imp = *imp_.lock();
        detail::defprop(
            vm_.handle(),
            imp.get_getter_table(),
            name,
            to_function<GF,Shared>(gf));
        return *this;
    }

    template <class GF, class SF>
    Klass& prop(const string& name, GF gf, SF sf) {
        auto& imp = *imp_.lock();
        detail::defprop(
            vm_.handle(),
            imp.get_getter_table(),
            imp.get_setter_table(),
            name,
            to_function<GF,Shared>(gf),
            to_function<SF,Shared>(sf));
        return *this;
    }
    
private:
    VM& vm_;
    std::weak_ptr<detail::KlassImp<C>> imp_;


};

}

#endif // SQUALL_KLASS_HPP_
