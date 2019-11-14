cmake_minimum_required(VERSION 3.2)
project(create_project)


set(SOURCE_FILES
	../cmake.cpp
	../cmake.h
	../codeblocks.cpp
	../codeblocks.h
	../create_project.cpp
	../create_project.h
	../msbuild.cpp
	../msbuild.h
	../msvc.cpp
	../msvc.h
	../visualstudio.cpp
	../visualstudio.h
	../xcode.cpp
	../xcode.h
    )

add_executable(create_project ${SOURCE_FILES})
if (WIN32)
    target_link_libraries(create_project rpcrt4 advapi32)
endif ()
