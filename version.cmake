execute_process(
    COMMAND git -C ${GIT_CWD} describe --always --abbrev=8 --dirty
    OUTPUT_VARIABLE GIT_DESCRIBE
    ERROR_QUIET)

if ("${GIT_DESCRIBE}" STREQUAL "")
    set(GIT_DESCRIBE "unknown")
else()
    string(STRIP "${GIT_DESCRIBE}" GIT_DESCRIBE)
endif()

set(CONTENTS "namespace ${VERSION_NAMESPACE} { const char *version() { return \"${GIT_DESCRIBE}\";} }")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version-backup/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version-backup/version.cpp EXISTING_CONTENTS)
else()
    set(EXISTING_CONTENTS "")
endif()

if ("${CONTENTS}" STREQUAL "${EXISTING_CONTENTS}")
    #message("Restoring ${VERSION_NAMESPACE} version.cpp backup because it is still current with \"${GIT_DESCRIBE}\"")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/version-backup/version.cpp DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
else()
    #message("Generating ${VERSION_NAMESPACE} version.cpp with \"${GIT_DESCRIBE}\"")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp "${CONTENTS}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version-backup/version.cpp "${CONTENTS}")
endif()
