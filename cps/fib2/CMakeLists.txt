file(GLOB fib_sources *.c)
add_executable(fib2 ${fib_sources})
target_link_libraries(fib2 stacks)
add_test(fib2 fib2)
set_target_properties(fib2 PROPERTIES FOLDER CPS)
