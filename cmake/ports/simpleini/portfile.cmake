# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO powerof3/simpleini
    REF 2328c20c7d11d88774f55441367196d8e90db418
    SHA512 7069b0c5d0b02b9992096ceb03b855acbfc5fe3715e2337f778bfcf9834251d2a5122d78b492dde4e70b8f2840be34fc2f9bafef9e4709a8d8c3da6bfba7270b
    HEAD_REF master
)

# Install codes
set(SIMPLEINI_SOURCE ${SOURCE_PATH}/SimpleIni.h
                     ${SOURCE_PATH}/ConvertUTF.h
                     ${SOURCE_PATH}/ConvertUTF.c
)

file(INSTALL ${SIMPLEINI_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
# Install sample
#file(INSTALL ${SOURCE_PATH}/snippets.cpp DESTINATION ${CURRENT_PACKAGES_DIR}/share/sample)

file(INSTALL ${SOURCE_PATH}/LICENCE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
