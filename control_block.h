#pragma once

#include <utility>
#include <cstddef>
namespace shared_ptr_detail {
class control_block {
public:
  control_block() = default;
  virtual ~control_block() = default;

  void inc_strong();
  void dec_strong();
  void inc_weak();
  void dec_weak();

  size_t use_count() const;

protected:
  virtual void delete_object() = 0;

private:
  size_t strong_cnt{0};
  size_t weak_cnt{0};
};

template <typename T, typename D>
class ptr_control_block : public control_block, public D {
public:
  explicit ptr_control_block(T* ptr, D&& deleter)
      : D(std::forward<D>(deleter)), ptr(ptr) {}

  ~ptr_control_block() override = default;

  void delete_object() override {
    static_cast<D&> (*this)(ptr);
  }

private:
  T* ptr{nullptr};
};

template <typename T>
class obj_control_block : public control_block {
public:
  template <typename... Args>
  explicit obj_control_block(Args&&... args) {
    new (&obj) T(std::forward<Args>(args)...);
  }

  ~obj_control_block() override = default;

  void delete_object() override {
    reinterpret_cast<T*>(&obj)->~T();
  }

  T* get_ptr() {
    return reinterpret_cast<T*>(&obj);
  }

private:
  std::aligned_storage_t<sizeof(T), alignof(T)> obj;
};
} // namespace
