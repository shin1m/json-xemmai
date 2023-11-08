add_library(json MODULE json.cc)
target_link_libraries(json xemmai)
set_property(TARGET json PROPERTY PREFIX "")
