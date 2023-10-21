cmake_minimum_required(VERSION 3.10)

project(S_GIT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(.)

aux_source_directory(. MAIN_LIST)

aux_source_directory(./cache CACHE_LIST)
aux_source_directory(./object OBJECT_LIST)

aux_source_directory(./commit COMMIT_LIST)
aux_source_directory(./init INIT_LIST)
aux_source_directory(./logCommand LOG_LIST)
aux_source_directory(./status STATUS_LIST)
aux_source_directory(./tag TAG_LIST)
aux_source_directory(./branch BRANCH_LIST)
aux_source_directory(./checkout CHECKOUT_LIST)
aux_source_directory(./ls-tree LS_TREE_LIST)
aux_source_directory(./merge MERGE_LIST)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(s-git ${MAIN_LIST}
	${CACHE_LIST} ${OBJECT_LIST}
	${COMMIT_LIST}
	${INIT_LIST}
	${LOG_LIST}
	${STATUS_LIST}
	${TAG_LIST}
	${BRANCH_LIST}
	${CHECKOUT_LIST}
	${LS_TREE_LIST}
	${MERGE_LIST}
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries(s-git stdc++fs)
endif()
