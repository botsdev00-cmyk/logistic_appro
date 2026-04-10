# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project")
  file(MAKE_DIRECTORY "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project")
endif()
file(MAKE_DIRECTORY
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project-build"
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-install"
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/tmp"
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project-stamp"
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src"
  "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/bots/semuliki_logistic/build/Desktop_Qt_6_9_3-Debug/libsodium-build/src/libsodium_project-stamp${cfgdir}") # cfgdir has leading slash
endif()
