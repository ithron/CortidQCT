#include <CL/opencl.h>

#include <gsl/gsl>

#include <array>
#include <iostream>
#include <vector>

int main(int, char **) {
  using namespace gsl;

  cl_uint platformCount;
  clGetPlatformIDs(0, NULL, &platformCount);

  auto platformIDs =
      std::vector<cl_platform_id>(narrow<std::size_t>(platformCount));
  clGetPlatformIDs(platformCount, platformIDs.data(), nullptr);

  for (auto &&id : platformIDs) {
    std::array<char, 4096> name, vendor, profile, version;
    std::array<char, 1024 * 1024> extensions;
    clGetPlatformInfo(id, CL_PLATFORM_NAME, name.size(), name.data(), nullptr);
    clGetPlatformInfo(id, CL_PLATFORM_VENDOR, vendor.size(), vendor.data(),
                      nullptr);
    clGetPlatformInfo(id, CL_PLATFORM_PROFILE, profile.size(), profile.data(),
                      nullptr);
    clGetPlatformInfo(id, CL_PLATFORM_VERSION, version.size(), version.data(),
                      nullptr);
    clGetPlatformInfo(id, CL_PLATFORM_EXTENSIONS, extensions.size(),
                      extensions.data(), nullptr);

    auto const nameStr = std::string(name.data());
    auto const vendorStr = std::string(vendor.data());
    auto const profileStr = std::string(profile.data());
    auto const versionStr = std::string(version.data());

    std::cout << "Platform " << nameStr << " (" << vendorStr << ") version "
              << versionStr << " [" << profileStr << "]" << std::endl;
  }

  return EXIT_SUCCESS;
}
