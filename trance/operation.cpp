
#include "operation.hpp"

namespace trance
{
  std::ostream& operator<<(std::ostream& os, const Operation& x)
  {
    switch (x.operation()) {
    case Operation::AXIOM:  os << "axiom"; break;
    case Operation::SHIFT:  os << "shift"; break;
    case Operation::REDUCE:  os << "reduce"; break;
    case Operation::REDUCE_LEFT:  os << "reduce-left"; break;
    case Operation::REDUCE_RIGHT: os << "reduce-right"; break;
    case Operation::UNARY:  os << "unary-" << x.closure(); break;
    case Operation::FINAL:  os << "final"; break;
    case Operation::IDLE:   os << "idle"; break;
    }
    return os;

  }
}
