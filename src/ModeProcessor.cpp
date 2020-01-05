#include <helgoboss-learn/ModeProcessor.h>

namespace helgoboss::internal {
  double alignToStepSize(double value, double stepSize) {
    if (stepSize == -1) {
      return value;
    } else {
      // Round to closest multiple of step size
      return std::round(value / stepSize) * stepSize;
    }
  }
}