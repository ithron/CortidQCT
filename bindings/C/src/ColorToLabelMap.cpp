#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <cassert>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_ColorToLabelMap
CQCT_createColorToLabelMap() {
  return static_cast<CQCT_ColorToLabelMap>(
      constructObject<ColorToLabelMaps::CustomMap>());
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_colorToLabelMapLoadFromFile(CQCT_ColorToLabelMap map, const char *filename,
                                 CQCT_Error *error) {
  assert(map != nullptr);

  try {

    map->impl_.objPtr->loadFromFile(filename);

    return true;
  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(*error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(*error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_colorToLabelMapEntryCount(CQCT_ColorToLabelMap map) {
  assert(map != nullptr);

  return map->impl_.objPtr->table.size();
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_colorToLabelMapCopyEntries(
    CQCT_ColorToLabelMap map, unsigned int **bufferPtr) {
  assert(map != nullptr);
  assert(bufferPtr != nullptr);

  auto const &mapRef = *(map->impl_.objPtr);

  auto const size = mapRef.table.size() * 4 * sizeof(unsigned int);

  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<unsigned int *>(malloc(size));
  }

  auto ptr = *bufferPtr;
  for (auto &&kv : mapRef.table) {
    auto const &color = kv.first;
    auto const label = static_cast<unsigned int>(kv.second);

    *ptr++ = static_cast<unsigned int>(color[0]);
    *ptr++ = static_cast<unsigned int>(color[1]);
    *ptr++ = static_cast<unsigned int>(color[2]);
    *ptr++ = label;
  }

  assert(std::distance(*bufferPtr, ptr) == size);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void
CQCT_colorToLabelMapSetEntries(CQCT_ColorToLabelMap map, size_t count,
                               const unsigned int *entries) {
  assert(map != nullptr);
  assert(count == 0 || entries != nullptr);

  if (count == 0) return;

  auto &mapRef = *(map->impl_.objPtr);

  mapRef.table.clear();

  for (auto i = 0u; i < 4 * count; i += 4) {
    auto color = ColorRGB{static_cast<std::uint8_t>(entries[i + 0]),
                          static_cast<std::uint8_t>(entries[i + 1]),
                          static_cast<std::uint8_t>(entries[i + 2])};

    auto label =
        static_cast<ColorToLabelMaps::CustomMap::LabelType>(entries[i + 3]);

    mapRef.table.emplace(std::make_pair(color, label));
  }
}

