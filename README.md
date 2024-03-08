# Sumty

`sumty` is a header-only C++20 library that provides "better" sum types.
The C++ standard library provides `std::variant`, `std::optional`, and
`std::exepcted` (as of C++23), but these have some limitiations that
`sumty` aims to remove.

`sumty` provides three sum types that each correspond to one of the
aforementioned standard library types:

* `sumty::variant` - improved `std::variant`
* `sumty::option` - improved `std::optional`
* `sumty::result` - improved `std::expected`

Aside from an extended set of monadic member functions, `sumty` makes
two main improvements over the standard library equivalents:

* References and `void` are supported as alternative types
  * E.g. `sumty::option<int&>`
  * E.g. `sumty::variant<void, float&, bool>`.
* Special case size optimizations are applied
  * E.g. `sizeof(sumty::variant<std::monostate, std::monostate>) == sizeof(bool)`
  * E.g. `sizeof(sumty::option<int&>) == sizeof(void*)`
  * E.g. `sizeof(sumty::result<void, void>) == sizeof(bool)`

## Why Would I Use `option<T&>` Instead of `T*`?
In general, maybe you wouldn't. However, when you make frequent use of
`std::optional<T>` in generic code, you often end up with cases where you
want to support returning an optional of any user defined type. But if
they want to return a reference, you're either out of luck, or you have
to make a specialization that returns `T*` instead. With `sumty::option`
no specialization is needed for references.

Here is a contrived example:
```cpp
template <typename F>
sumty::option<std::invoke_result_t<F>> call_if(bool cond, F func) {
    if (cond) {
        return func();
    } else {
        return sumty::none;
    }
}
```

`call_if` accounts for `func` returning a reference automatically, and
the returned value has the same representation as a raw pointer, so
there's no convenience penalty.

_However_, if you also want to support `func` returning `void`, we do
still have to add a little extra due to limitations of `void` in the
language:
```cpp
template <typename F>
sumty::option<std::invoke_result_t<F>> call_if(bool cond, F func) {
    if (cond) {
        if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
            func();
            return sumty::some<void>();
        } else {
            return func();
        }
    } else {
        return sumty::none;
    }
}
```

No matter what you do, `void` will cause at least a little headache.
But by using `sumty::option<T>`, the `call_if` function can support
`void` without needing a different signature, and only requiring
five extra lines to internally specialize around `void`.

`sumty::variant` and `sumty::result` have similar benefits over their
STL counterparts.
