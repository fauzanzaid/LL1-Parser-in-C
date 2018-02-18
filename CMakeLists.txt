cmake_minimum_required(VERSION 3.5)
project( ParserLL1 VERSION 0.1.1 )

add_library(ParserLL1 STATIC src/ParserLL1.c)

target_include_directories( ParserLL1 PUBLIC ${PROJECT_SOURCE_DIR}/include )
target_sources( ParserLL1 PRIVATE ${PROJECT_SOURCE_DIR}/src/ParserLL1 )
target_sources( ParserLL1 PUBLIC ${PROJECT_SOURCE_DIR}/include/ParserLL1 )

set_target_properties(ParserLL1
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

if(NOT TARGET LinkedList)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/LinkedList ${CMAKE_SOURCE_DIR}/ext/LinkedList/build/bin)
endif(NOT TARGET LinkedList)
target_link_libraries(ParserLL1 LinkedList)

if(NOT TARGET HashTable)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/HashTable ${CMAKE_SOURCE_DIR}/ext/HashTable/build/bin)
endif(NOT TARGET HashTable)
target_link_libraries(ParserLL1 HashTable)

if(NOT TARGET BitSet)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/BitSet ${CMAKE_SOURCE_DIR}/ext/BitSet/build/bin)
endif(NOT TARGET BitSet)
target_link_libraries(ParserLL1 BitSet)

if(NOT TARGET ParseTree)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/ParseTree ${CMAKE_SOURCE_DIR}/ext/ParseTree/build/bin)
endif(NOT TARGET ParseTree)
target_link_libraries(ParserLL1 ParseTree)
