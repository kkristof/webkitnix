list(APPEND JavaScriptCore_SOURCES
    jit/ExecutableAllocatorFixedVMPool.cpp
    jit/ExecutableAllocator.cpp
)

list(APPEND JavaScriptCore_LIBRARIES
    ${ICU_I18N_LIBRARIES}
)

list(APPEND JavaScriptCore_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${WTF_DIR}/wtf/gobject
)
