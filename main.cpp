#include <iostream>


thread_local const char * gThreadLocalStr {nullptr};

class ScopeGuard {
public:
  ScopeGuard(const char* newStr): old_{gThreadLocalStr} {
    gThreadLocalStr = newStr;
  }

  ~ScopeGuard() {
    gThreadLocalStr = old_;
  }

private:
  const char * old_;
};

void printScope(const char* file, int line, const char * scope = gThreadLocalStr) {
  std::cout << "["<< file <<":" <<line << "] Scope = " << scope << std::endl;
}

#define PRINT_SCOPE() printScope(__FILE__, __LINE__)

void testMain() {
  PRINT_SCOPE();
  {
    ScopeGuard g("level 1 scope");
    PRINT_SCOPE();
    try {
      ScopeGuard g("level 2 scope");
      PRINT_SCOPE();
      {
        ScopeGuard g("level 3 scope");
        PRINT_SCOPE();
        throw "Exception";
      }
    } catch (...) {
      PRINT_SCOPE();
    }
  }
}

int main() {
  ScopeGuard guard("global scope");
  testMain();
  return 0;
}