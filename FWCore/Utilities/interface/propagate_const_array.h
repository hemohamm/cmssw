#ifndef FWCore_Utilities_interface_propagate_const_array_h
#define FWCore_Utilities_interface_propagate_const_array_h
// -*- C++ -*-
//
// Package:     FWCore/Utilities
// Class  :     propagate_const_array
// Description: Propagate const to array-like objects. Based on C++ experimental std::propagate_const.
//              Unlike std::propagate_const, propagate_const_array can only be used with complete types.

// system include files
#include <type_traits>
#include <utility>

// user include files

// forward declarations

namespace edm {

  namespace impl {

    // check if a type T has a subscript operator T[N]
    template <typename, typename = void>
    struct has_subscript_operator : std::false_type {};

    template <typename T>
    struct has_subscript_operator<T, std::void_t<decltype(std::declval<T&>()[0])>> : std::true_type {};

    template <typename T>
    constexpr auto has_subscript_operator_v = has_subscript_operator<T>::value;

    // for a type T, return the type of the return value of the subscript operator T[N]
    template <typename T, typename = void, typename = void>
    struct subscript_type {};

    // the specialisations for arrays allow supporting incomplete types
    template <typename T>
    struct subscript_type<T[]> {
      using type = T;
    };

    template <typename T, int N>
    struct subscript_type<T[N]> {
      using type = T;
    };

    // for non-array types that implement the subscript operator[], a complete type is needed
    template <typename T>
    struct subscript_type<T, std::enable_if_t<not std::is_array_v<T>>, std::enable_if_t<has_subscript_operator_v<T>>> {
      using type = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;
    };

    template <typename T>
    using subscript_type_t = typename subscript_type<T>::type;

  }  // namespace impl

  template <typename T>
  class propagate_const_array;

  template <typename T>
  std::decay_t<T>& get_underlying(propagate_const_array<T>&);
  template <typename T>
  std::decay_t<T> const& get_underlying(propagate_const_array<T> const&);

  template <typename T>
  class propagate_const_array {
  public:
    friend std::decay_t<T>& get_underlying<T>(propagate_const_array<T>&);
    friend std::decay_t<T> const& get_underlying<T>(propagate_const_array<T> const&);

    template <typename U>
    friend class propagate_const_array;

    //using element_type = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;
    using element_type = typename impl::subscript_type_t<T>;

    propagate_const_array() = default;

    propagate_const_array(propagate_const_array<T>&&) = default;
    propagate_const_array(propagate_const_array<T> const&) = delete;
    template <typename U>
    propagate_const_array(U&& iValue) : m_value(std::forward<U>(iValue)) {}

    propagate_const_array<T>& operator=(propagate_const_array&&) = default;
    propagate_const_array<T>& operator=(propagate_const_array<T> const&) = delete;

    template <typename U>
    propagate_const_array& operator=(propagate_const_array<U>& other) {
      static_assert(std::is_convertible_v<std::decay_t<U>, std::decay_t<T>>,
                    "Cannot assign propagate_const_array<> of incompatible types");
      m_value = other.m_value;
      return *this;
    }

    template <typename U>
    propagate_const_array& operator=(U&& iValue) {
      m_value = std::forward<U>(iValue);
      return *this;
    }

    // ---------- const member functions ---------------------
    element_type const* get() const { return &m_value[0]; }
    element_type const& at(size_t pos) const { return m_value[pos]; }
    element_type const& operator[](size_t pos) const { return this->at(pos); }

    operator element_type const*() const { return this->get(); }

    // ---------- member functions ---------------------------
    element_type* get() { return &m_value[0]; }
    element_type& at(size_t pos) { return m_value[pos]; }
    element_type& operator[](size_t pos) { return this->at(pos); }

    operator element_type*() { return this->get(); }

  private:
    // ---------- member data --------------------------------
    std::decay_t<T> m_value;
  };

  template <typename T>
  std::decay_t<T>& get_underlying(propagate_const_array<T>& iP) {
    return iP.m_value;
  }

  template <typename T>
  std::decay_t<T> const& get_underlying(propagate_const_array<T> const& iP) {
    return iP.m_value;
  }

}  // namespace edm

#endif  // FWCore_Utilities_interface_propagate_const_array_h
