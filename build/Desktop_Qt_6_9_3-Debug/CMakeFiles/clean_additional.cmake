# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/bcrypt_vendor_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/bcrypt_vendor_autogen.dir/ParseCache.txt"
  "CMakeFiles/logistic_appro_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/logistic_appro_autogen.dir/ParseCache.txt"
  "bcrypt_vendor_autogen"
  "logistic_appro_autogen"
  )
endif()
