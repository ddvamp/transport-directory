cmake_minimum_required (VERSION 3.16.3 FATAL_ERROR)

project (transport-directory VERSION 1.0.1 LANGUAGES CXX)

set (main ${PROJECT_NAME})
set (dir ${PROJECT_SOURCE_DIR})

aux_source_directory ("${dir}/src" SOURCES)

add_executable (${main} ${SOURCES})

set_target_properties (${main} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF

	RUNTIME_OUTPUT_DIRECTORY "${dir}/bin"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${dir}/bin/debug"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${dir}/bin/release"

	#RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time"
	#RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time"
)

target_include_directories (${main} PUBLIC
	"${dir}/include"
)

set (WARNING_FLAGS
	-pedantic
	-pedantic-errors
	-Wall
	-Wextra
	-Wdouble-promotion
	-Wformat=2
	-Wformat-overflow=2
	-Wformat-signedness
	-Wformat-truncation=2
	-Wnull-dereference
	-Wimplicit-fallthrough=5
	-Wmissing-include-dirs
	-Wshift-overflow=2
	-Wswitch-default
	-Wswitch-enum
	-Wunused-const-variable=2
	-Wunused
	-Wuninitialized
	-Wstrict-overflow=5
	-Wstringop-overflow=4
	-Wsuggest-attribute=pure
	-Wno-error=suggest-attribute=pure
	-Wsuggest-attribute=const
	-Wno-error=suggest-attribute=const
	-Wsuggest-attribute=noreturn
	-Wno-error=suggest-attribute=noreturn
	-Wmissing-noreturn
	-Wsuggest-attribute=malloc
	-Wno-error=suggest-attribute=malloc
	-Wsuggest-attribute=format
	-Wno-error=suggest-attribute=format
	-Wmissing-format-attribute
	-Wsuggest-attribute=cold
	-Wno-error=suggest-attribute=cold
	-Walloc-zero
	-Walloca
	-Warith-conversion
	-Warray-bounds=2
	-Wattribute-alias=2
	-Wduplicated-branches
	-Wduplicated-cond
	#-Wsystem-headers
	-Wunknown-pragmas
	-Wtrampolines
	-Wfloat-equal
	-Wshadow
	#-Wframe-larger-than=
	#-Wstack-usage=
	-Wunsafe-loop-optimizations
	-Wundef
	$<$<CONFIG:Release>:-Wunused-macros>
	#-Wno-error=unused-macros
	-Wcast-qual
	-Wcast-align
	-Wcast-align=strict
	-Wconversion
	-Wdate-time
	-Wenum-conversion
	-Wsign-conversion
	-Wlogical-op
	-Wmissing-declarations
	-Wopenmp-simd
	-Wpacked
	#-Wpadded
	-Wredundant-decls
	#-Winline
	#-Winterference-size // gcc-11.2
	-Winvalid-pch
	-Wvector-operation-performance
	-Wdisabled-optimization
	-Wstack-protector
	#-Wabi-tag
	-Wctor-dtor-privacy
	-Winvalid-imported-macros
	-Wnoexcept
	-Wnon-virtual-dtor
	-Wredundant-tags
	#-Weffc++ // сломано для modern C++
	-Wstrict-null-sentinel
	-Wold-style-cast
	-Woverloaded-virtual
	-Wsign-promo
	-Wmismatched-tags
	-Wzero-as-null-pointer-constant
	-Wplacement-new=2
	-Wcatch-value=3
	-Wconditionally-supported
	-Wextra-semi
	-Wsuggest-final-types
	-Wsuggest-final-methods
	-Wsuggest-override
	-Wuseless-cast
	-fstrict-enums
)

set (INSTRUMENTATION_FLAGS
	-pg
	-fsanitize=address
	-fsanitize=pointer-compare
	-fsanitize=pointer-subtract
	-fsanitize=leak
	-fsanitize=undefined
	-fno-sanitize-recover
	-fstack-protector-strong
)
	
target_compile_options (${main} PUBLIC
	$<$<CONFIG:Debug>:-ggdb3>
	$<$<CONFIG:Debug>:-Og>
	$<$<CONFIG:Release>:-O3>
	$<$<CONFIG:Release>:-march=native>
	-Werror
	-Wfatal-errors
	${WARNING_FLAGS}
	$<$<CONFIG:Debug>:${INSTRUMENTATION_FLAGS}>
	$<$<CONFIG:Debug>:-fstack-usage>
	$<$<CONFIG:Debug>:-fverbose-asm>
	$<$<CONFIG:Debug>:-save-temps>
	#-ftime-report
	#-time
	#-fno-pretty-templates
	#-fopt-info-inline-all
)

target_link_options (${main} PUBLIC
	$<$<CONFIG:Debug>:${INSTRUMENTATION_FLAGS}>
	#$<$<CONFIG:Debug>:LINKER:--print-memory-usage>
	$<$<CONFIG:Debug>:LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${main}.map>
)

set (DEBUG_DEFINITIONS
	DEBUG
	_GLIBCXX_DEBUG
	_GLIBCXX_DEBUG_PEDANTIC
	_FORTIFY_SOURCE=2
)

target_compile_definitions (${main} PUBLIC
	$<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>
)
