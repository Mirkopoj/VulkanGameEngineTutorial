#pragma once

#include <vector>
#include "../lve/lve_device.hpp"

namespace lve {

class ComputeSystem {
  public:
   ComputeSystem(LveDevice &device);
   ComputeSystem(ComputeSystem &&) = delete;
   ComputeSystem(const ComputeSystem &) = delete;
   ComputeSystem &operator=(ComputeSystem &&) = delete;
   ComputeSystem &operator=(const ComputeSystem &) = delete;
   ~ComputeSystem();

  private:
   LveDevice &lveDevice;
};
}  // namespace lve
