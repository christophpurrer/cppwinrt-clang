#pragma once
#ifdef __clang__
#include <future>

//template <class _Fty, class... _ArgTypes>
//[[nodiscard]] future<_Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>> async(
//    launch _Policy, _Fty&& _Fnarg, _ArgTypes&&... _Args) {
//
//    using _Ret = _Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>;
//    using _Ptype = typename _P_arg_type<_Ret>::type;
//    _Promise<_Ptype> _Pr(
//        _Get_associated_state<_Ret>(_Policy, _Fake_no_copy_callable_adapter<_Fty, _ArgTypes...>(
//        _Get_associated_state<_Ret>(_Policy, _Fake_no_copy_callable_adapter<_Fty, _ArgTypes...>(
//            ::std::forward<_Fty>(_Fnarg), ::std::forward<_ArgTypes>(_Args)...)));
//
//    return future<_Ret>(_Pr._Get_state_for_future(), _Nil());
//}
//
//template <class _Fty, class... _ArgTypes>
//[[nodiscard]] future<_Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>> async(_Fty&& _Fnarg, _ArgTypes&&... _Args) {
//
//    return ::std::async(launch::async | launch::deferred, ::std::forward<_Fty>(_Fnarg), ::std::forward<_ArgTypes>(_Args)...);
//}


namespace std::experimental {
    template <class _Ty, class... _ArgTypes>
    struct coroutine_traits<std::future<_Ty>, _ArgTypes...> {

        struct promise_type {
            promise<_Ty> _MyPromise;

            future<_Ty> get_return_object() {
                return _MyPromise.get_future();
            }

            suspend_never initial_suspend() const noexcept {
                return {};
            }

            suspend_never final_suspend() const noexcept {
                return {};
            }

            template <class _Ut>
            void return_value(_Ut&& _Value) {
                _MyPromise.set_value(::std::forward<_Ut>(_Value));
            }

            void unhandled_exception() {
                _MyPromise.set_exception(::std::current_exception());
            }
        };
    };

    template <class... _ArgTypes>
    struct coroutine_traits<std::future<void>, _ArgTypes...> {

        struct promise_type {
            promise<void> _MyPromise;

            future<void> get_return_object() {
                return _MyPromise.get_future();
            }

            suspend_never initial_suspend() const noexcept {
                return {};
            }

            suspend_never final_suspend() const noexcept {
                return {};
            }

            void return_void() {
                _MyPromise.set_value();
            }

            void unhandled_exception() {
                _MyPromise.set_exception(::std::current_exception());
            }
        };
    };

    template <class _Ty>
    struct _Future_awaiter {
        future<_Ty>& _Fut;

        bool await_ready() const {
            return _Fut._Is_ready();
        }

        void await_suspend(experimental::coroutine_handle<> _ResumeCb) {

            thread _WaitingThread([&_Fut = _Fut, _ResumeCb]() mutable {
                _Fut.wait();
                _ResumeCb();
                });
            _WaitingThread.detach();
        }

        decltype(auto) await_resume() {
            return _Fut.get();
        }
    };
}

template <class _Ty>
auto operator co_await(std::future<_Ty>&& _Fut) {
    return std::experimental::_Future_awaiter<_Ty>{_Fut};
}

template <class _Ty>
auto operator co_await(std::future<_Ty>& _Fut) {
    return std::experimental::_Future_awaiter<_Ty>{_Fut};
}

#endif