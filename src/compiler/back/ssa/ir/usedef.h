#ifndef TINYLANG_BACK_SSA_IR_USEDEF_H_
#define TINYLANG_BACK_SSA_IR_USEDEF_H_

// reference: LLVM version 1.3

#include <memory>
#include <vector>
#include <ostream>
#include <forward_list>
#include <cstddef>
#include <cassert>

namespace tinylang::back::ssa {

// interafce of all SSAs
class SSAInterface {
 public:
  virtual ~SSAInterface() = default;

  // dump the content of SSA value to output stream
  virtual void Dump(std::ostream &os) = 0;
};

class Value;
class User;
class Use;

using SSAPtr = std::shared_ptr<Value>;
using SSAPtrList = std::vector<SSAPtr>;

// a SSA value
class Value : public SSAInterface {
 public:
  Value() {}

  // add a use reference to current value
  void AddUse(Use *use) { uses_.push_front(use); }
  // remove use reference from current value
  void RemoveUse(Use *use) { uses_.remove(use); }
  // replace current value by another value
  void ReplaceBy(const SSAPtr &value);

  const std::forward_list<Use *> &uses() const { return uses_; }

 private:
  // singly-linked list for 'Use'
  std::forward_list<Use *> uses_;
};

// a bidirectional reference between SSA users and values
class Use {
 public:
  explicit Use(const SSAPtr &value, User *user)
      : value_(value), user_(user) {}
  // copy constructor
  Use(const Use &use)
      : value_(use.value_), user_(use.user_) {
    if (value_) value_->AddUse(this);
  }
  // move constructor
  Use(Use &&use) noexcept
      : value_(std::move(use.value_)), user_(use.user_) {
    if (value_) {
      value_->RemoveUse(&use);
      value_->AddUse(this);
    }
  }
  // destructor
  ~Use() {
    if (value_) value_->RemoveUse(this);
  }

  void set_value(const SSAPtr &value);

  const SSAPtr &value() const { return value_; }
  User *user() const { return user_; }

 private:
  SSAPtr value_;
  User *user_;
};

// a SSA user which can use other values
class User : public Value {
 public:
  User() : Value() {}

  // preallocate some space for values
  void reserve(std::size_t size) { operands_.reserve(size); }
  // add new value to current user
  void push_back(const SSAPtr &value) {
    operands_.push_back(Use(value, this));
  }

  // access value in current user
  Use &operator[](std::size_t pos) { return operands_[pos]; }
  // access value in current user (const)
  const Use &operator[](std::size_t pos) const { return operands_[pos]; }

  // begin iterator
  auto begin() { return operands_.begin(); }
  // end iterator
  auto end() { return operands_.end(); }
  // count of elements in current user
  std::size_t size() const { return operands_.size(); }
  // return true if no value in current user
  bool empty() const { return operands_.empty(); }

 private:
  std::vector<Use> operands_;
};

}  // namespace tinylang::back::ssa

#endif  // TINYLANG_BACK_SSA_IR_USEDEF_H_
