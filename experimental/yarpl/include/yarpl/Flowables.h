#pragma once

#include <limits>

#include "Flowable.h"

namespace yarpl {
namespace flowable {

class Flowables {
 public:
  static Reference<Flowable<int64_t>> range(int64_t start, int64_t end) {
    auto lambda = [ start, end, i = start ](
        Subscriber<int64_t> & subscriber, int64_t requested) mutable {
      int64_t emitted = 0;
      bool done = false;

      while (i < end && emitted < requested) {
        subscriber.onNext(i++);
        ++emitted;
      }

      if (i >= end) {
        subscriber.onComplete();
        done = true;
      }

      return std::make_tuple(requested, done);
    };

    return Flowable<int64_t>::create(std::move(lambda));
  }

  template <typename T>
  static Reference<Flowable<T>> just(const T& value) {
    auto lambda = [value](Subscriber<T>& subscriber, int64_t) {
      // # requested should be > 0.  Ignoring the actual parameter.
      subscriber.onNext(value);
      subscriber.onComplete();
      return std::make_tuple(static_cast<int64_t>(1), true);
    };

    return Flowable<T>::create(std::move(lambda));
  }

  template <typename T>
  static Reference<Flowable<T>> just(std::initializer_list<T> list) {
    auto lambda = [ list, it = list.begin() ](
        Subscriber<T> & subscriber, int64_t requested) mutable {
      int64_t emitted = 0;
      bool done = false;

      while (it != list.end() && emitted < requested) {
        subscriber.onNext(*it++);
        ++emitted;
      }

      if (it == list.end()) {
        subscriber.onComplete();
        done = true;
      }

      return std::make_tuple(static_cast<int64_t>(emitted), done);
    };

    return Flowable<T>::create(std::move(lambda));
  }

  template <typename T>
  static Reference<Flowable<T>> cycle(const T& value) {
    auto lambda = [value](Subscriber<T>& subscriber, int64_t requested) {
      int64_t emitted = 0;
      while (emitted < requested) {
        subscriber.onNext(value);
        ++emitted;
      }
      return std::make_tuple(emitted, false);
    };

    return Flowable<T>::create(std::move(lambda));
  }

  template <typename T>
  static Reference<Flowable<T>> cycle(std::initializer_list<T> list) {
    auto lambda = [ list, it = list.begin() ](
        Subscriber<T> & subscriber, int64_t requested) mutable {
      int64_t emitted = 0;
      while (emitted < requested) {
        subscriber.onNext(*it++);
        ++emitted;
        if (it == list.end()) {
          it = list.begin();
        }
      }

      return std::make_tuple(emitted, false);
    };
    return Flowable<T>::create(std::move(lambda));
  }

  template <
      typename T,
      typename OnSubscribe,
      typename = typename std::enable_if<std::is_callable<
          OnSubscribe(Reference<Subscriber<T>>),
          void>::value>::type>
  static Reference<Flowable<T>> fromPublisher(OnSubscribe&& function) {
    return Reference<Flowable<T>>(new FromPublisherOperator<T, OnSubscribe>(
        std::forward<OnSubscribe>(function)));
  }

  template <typename T>
  static Reference<Flowable<T>> empty() {
    auto lambda = [](Subscriber<T>& subscriber, int64_t) {
      subscriber.onComplete();
      return std::make_tuple(static_cast<int64_t>(0), true);
    };
    return Flowable<T>::create(std::move(lambda));
  }

  template <typename T>
  static Reference<Flowable<T>> error(const std::exception_ptr ex) {
    auto lambda = [ex](Subscriber<T>& subscriber, int64_t) {
      subscriber.onError(ex);
      return std::make_tuple(static_cast<int64_t>(0), true);
    };
    return Flowable<T>::create(std::move(lambda));
  }

  template <typename T, typename ExceptionType>
  static Reference<Flowable<T>> error(const ExceptionType& ex) {
    auto lambda = [ex](Subscriber<T>& subscriber, int64_t) {
      subscriber.onError(std::make_exception_ptr(ex));
      return std::make_tuple(static_cast<int64_t>(0), true);
    };
    return Flowable<T>::create(std::move(lambda));
  }

 private:
  Flowables() = delete;
};

} // flowable
} // yarpl
