#pragma once

#include "control_block.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

template <typename T>
class shared_ptr {
  template <typename _>
  friend class weak_ptr;

  template <typename _>
  friend class shared_ptr;

public:
  shared_ptr() noexcept = default;

  explicit shared_ptr(std::nullptr_t) noexcept : shared_ptr() {}

  ~shared_ptr() {
    if (block) {
      block->dec_strong();
    }
  }

  template <typename Y, typename D = std::default_delete<Y>,
      typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  explicit shared_ptr(Y* ptr, D&& deleter = D()) : ptr(ptr) {
    try {
      block = new shared_ptr_detail::ptr_control_block<Y, D>(ptr, std::forward<D>(deleter));
    } catch (...) {
      deleter(ptr);
      throw;
    }
    block->inc_strong();
  }

  shared_ptr(shared_ptr const& other) noexcept : shared_ptr(other.block, other.ptr) {}

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  shared_ptr(shared_ptr<Y> const& other) noexcept : shared_ptr(other.block, other.ptr) {}

  shared_ptr(shared_ptr&& other) noexcept : shared_ptr(other.block, other.ptr) {
    other.reset();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  shared_ptr(shared_ptr<Y>&& other) noexcept : shared_ptr(other.block, other.ptr) {
    other.reset();
  }

  shared_ptr& operator=(shared_ptr const& other) noexcept {
    if (&other != this) {
      auto tmp = shared_ptr(other);
      swap(*this, tmp);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  shared_ptr& operator=(shared_ptr<Y> const& other) noexcept {
    if (&other != this) {
      auto tmp = shared_ptr(other);
      swap(*this, tmp);
    }
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    if (&other != this) {
      auto tmp = shared_ptr(std::move(other));
      swap(*this, tmp);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  shared_ptr& operator=(shared_ptr<Y>&& other) noexcept {
    if (&other != this) {
      auto tmp = shared_ptr(std::move(other));
      swap(*this, tmp);
    }
    return *this;
  }

  // aliasing constructors
  template <typename Y>
  shared_ptr(shared_ptr<Y> const& other, T* ptr) noexcept : shared_ptr(other.block, ptr) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept : shared_ptr(other.block, ptr) {
    other.reset();
  }

  void reset() noexcept {
    auto tmp = shared_ptr();
    swap(*this, tmp);
  }

  template <typename Y, typename D = std::default_delete<Y>,
      typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  void reset(Y* new_ptr, D&& deleter = D()) {
    auto tmp = shared_ptr<T>(new_ptr, std::forward<D>(deleter));
    swap(*this, tmp);
  }

  T* get() const noexcept {
    return ptr;
  }
  operator bool() const noexcept {
    return get() != nullptr;
  }
  T& operator*() const noexcept {
    return *get();
  }
  T* operator->() const noexcept {
    return get();
  }

  std::size_t use_count() const noexcept {
    if (block) {
      return block->use_count();
    }
    return 0;
  }

  friend bool operator==(shared_ptr const& a, shared_ptr const& b) {
    return a.ptr == b.ptr;
  }

  friend bool operator!=(shared_ptr const& a, shared_ptr const& b) {
    return a.ptr != b.ptr;
  }

  friend bool operator==(shared_ptr const& a, std::nullptr_t) {
    return a.ptr == nullptr;
  }

  friend bool operator==(std::nullptr_t, shared_ptr const& a) {
    return a.ptr == nullptr;
  }

  friend bool operator!=(shared_ptr const& a, std::nullptr_t) {
    return a.ptr != nullptr;
  }

  friend bool operator!=(std::nullptr_t, shared_ptr const& a) {
    return a.ptr != nullptr;
  }

private:
  T* ptr{nullptr};
  shared_ptr_detail::control_block* block{nullptr};

  shared_ptr(shared_ptr_detail::control_block* block, T* ptr) noexcept : ptr(ptr), block(block) {
    if (block) {
      block->inc_strong();
    }
  }

  friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
    std::swap(a.ptr, b.ptr);
    std::swap(a.block, b.block);
  }

  template <typename Y, typename... Args>
  friend shared_ptr<Y> make_shared(Args&&... args);
};

template <typename T>
class weak_ptr {
public:
  weak_ptr() noexcept = default;

  ~weak_ptr() {
    if (block) {
      block->dec_weak();
    }
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr(shared_ptr<Y>const& other) noexcept : weak_ptr(other.block, other.ptr) {}

  weak_ptr(weak_ptr const& other) noexcept : weak_ptr(other.block, other.ptr) {}

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr(weak_ptr<Y> const& other) noexcept : weak_ptr(other.block, other.ptr) {}

  weak_ptr(weak_ptr&& other) noexcept : weak_ptr(other.block, other.ptr) {
    other.reset();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr(weak_ptr<Y>&& other) noexcept : weak_ptr(other.block, other.ptr) {
    other.reset();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr& operator=(shared_ptr<Y>const& other) noexcept {
    auto tmp = weak_ptr(other);
    swap(*this, tmp);
    return *this;
  }

  weak_ptr& operator=(weak_ptr const& other) noexcept {
    if (&other != this) {
      auto tmp = weak_ptr(other);
      swap(*this, tmp);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr& operator=(weak_ptr<Y> const& other) noexcept {
    if (&other != this) {
      auto tmp = weak_ptr(other);
      swap(*this, tmp);
    }
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    if (&other != this) {
      auto tmp = weak_ptr(std::move(other));
      swap(*this, tmp);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    if (&other != this) {
      auto tmp = weak_ptr(std::move(other));
      swap(*this, tmp);
    }
    return *this;
  }

  shared_ptr<T> lock() const noexcept {
    if (block && block->use_count() != 0) {
      return shared_ptr<T>(block, ptr);
    }
    return shared_ptr<T>();
  }

private:
  T* ptr{nullptr};
  shared_ptr_detail::control_block* block{nullptr};

  weak_ptr(shared_ptr_detail::control_block* block, T* ptr) noexcept : ptr(ptr), block(block) {
    if (block) {
      block->inc_weak();
    }
  }

  void reset() noexcept {
    auto tmp = weak_ptr();
    swap(*this, tmp);
  }

  friend void swap(weak_ptr& a, weak_ptr& b) noexcept {
    std::swap(a.ptr, b.ptr);
    std::swap(a.block, b.block);
  }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  auto* block = new shared_ptr_detail::obj_control_block<T>(std::forward<Args>(args)...);
  return shared_ptr<T>(block, block->get_ptr());
}
