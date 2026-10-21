file(GLOB hdrs CONFIGURE_DEPENDS "*.h*")

add_library(stack_queue INTERFACE)

target_include_directories(stack_queue INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(stack_queue_headers SOURCES ${hdrs})
