
add_executable (soes-demo
  main.c
  slave_objectlist.c
  )
target_link_libraries(soes-demo LINK_PUBLIC soes)
