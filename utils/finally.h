#ifndef _CHECKED_IO_HPP_
#define _CHECKED_IO_HPP_

namespace dbImpl {
  namespace _internal {
    template <typename F>
    struct FinalAction {
      FinalAction(F f) : clean_{f} {}
      ~FinalAction() { clean_(); }
      F clean_;
    };
  }

  //utility function used to make sure a Callable will be called when the
  //current scope is exited.
  template <typename F> _internal::FinalAction<F> finally(F f) {
    return _internal::FinalAction<F>(f);
  }
}

#endif
