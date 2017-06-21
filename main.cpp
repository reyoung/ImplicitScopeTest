#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>

struct Variable {};
using VariablePtr = std::weak_ptr<Variable>;

class Scope {
public:
  Scope() = default;
  Scope(const std::shared_ptr<Scope>& scope):parent_{scope} {}

  VariablePtr getVar(const std::string& name) const {
    auto it = vars_.find(name);
    if (it != vars_.end()) {
      return it->second;
    } else if (parent_ != nullptr) {
      return parent_->getVar(name);
    } else {
      return VariablePtr();  // nullptr
    }
  }

  VariablePtr createOrGetVar(const std::string& name) {
    VariablePtr ptr = getVar(name);
    if (ptr.lock() != nullptr) {
      return ptr;
    } else {
      vars_[name].reset(new Variable());
      return vars_.at(name);
    }
  }

  bool remove(const std::string& name) {
    auto it = vars_.find(name);
    if (it != vars_.end()) {
      vars_.erase(it);
      return true;
    } else {
      return false;
    }
  }

private:
  std::unordered_map<std::string, std::shared_ptr<Variable>> vars_;
  std::shared_ptr<Scope> parent_{nullptr};
};

thread_local std::shared_ptr<Scope>* gScope = nullptr;

std::shared_ptr<Scope>& threadLocalScope() {
  if (gScope == nullptr) {
    gScope = new std::shared_ptr<Scope>;
  }
  return *gScope;
}

class ScopeGuard final{
public:
  ScopeGuard(const std::shared_ptr<Scope>& scope) {
    old_ = threadLocalScope();
    threadLocalScope() = scope;
  }

  ~ScopeGuard() {
    threadLocalScope() = old_;
  }

  ScopeGuard(const ScopeGuard& o) = delete;
  ScopeGuard& operator= (const ScopeGuard& o) = delete;
private:
  std::shared_ptr<Scope> old_;
};

class NewScope final {
public:
  NewScope(): guard_(std::make_shared<Scope>(threadLocalScope())) {
    curScope_ = threadLocalScope();
  }

  Scope& scope() const {
    return *curScope_;
  }

private:
  ScopeGuard guard_;
  std::shared_ptr<Scope> curScope_;
};

VariablePtr createOrGetVariable(const std::string& name) {
  return threadLocalScope()->createOrGetVar(name);
}


int main() {
  NewScope globalScope;

  auto x = createOrGetVariable("X");
  auto y = createOrGetVariable("Y");

  {
    NewScope localScope;
    auto tmp = createOrGetVariable("TMP");
  }

  /// Should be true, because tmp are created in local scope
  /// so get "TMP" from globalScope should return nullptr.
  std::cout << (globalScope.scope().getVar("TMP").lock() == nullptr) << std::endl;

  return 0;
}