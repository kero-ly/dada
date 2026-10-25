# The contracts themselves live in /contracts (see docs/adr/0001-layering.md).
# This wrapper only gives them a component name that reads well in REQUIRES.
idf_component_register(INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../contracts")
