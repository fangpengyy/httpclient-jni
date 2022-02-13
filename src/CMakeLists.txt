cmake_minimum_required(VERSION 3.2)

INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(/usr/lib/jvm/java-8-openjdk-amd64/include/linux)
INCLUDE_DIRECTORIES(/usr/lib/jvm/java-8-openjdk-amd64/include)


set(LOCAL_SOURCE_FILES 
     utils.cpp
     socketfun.cpp
     buffile.cpp
     httpconn.cpp
     log.cpp
     main.cpp
     httpmng.cpp
     HttpM_ReqHttp.cpp
) 

if (exectype)
    add_executable(${EXEC_NAME} ${LOCAL_SOURCE_FILES})
else()
    add_library(${EXEC_NAME}  SHARED ${LOCAL_SOURCE_FILES} )
endif()

target_link_libraries(${EXEC_NAME} -ldl -lpthread)
target_link_libraries(${EXEC_NAME} -lssl -lcrypto)



install(TARGETS ${EXEC_NAME} DESTINATION ${EXECSVR_INSTALL_BIN_DIR})


