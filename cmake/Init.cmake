cmake_minimum_required(VERSION 3.8)

# Setup Hunter
include(${CMAKE_CURRENT_LIST_DIR}/HunterGate.cmake)

HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.22.36.tar.gz"
  SHA1 "66b28c3e88dd81833851181149df1075c62e8790"
  FILEPATH "${CMAKE_CURRENT_LIST_DIR}/Hunter/config.cmake"
)

