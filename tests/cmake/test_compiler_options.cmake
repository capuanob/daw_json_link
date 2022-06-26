
option( DAW_JSON_USE_SANITIZERS "Enable address and undefined sanitizers" OFF )
option( DAW_WERROR "Enable WError for test builds" OFF )
option( DAW_ALLOW_SSE42 "EXPERIMENTAL: Enable WError for test builds" OFF )
option( DAW_JSON_COVERAGE "Enable code coverage(gcc/clang)" OFF )

if( DAW_ALLOW_SSE42 )
    add_compile_definitions( DAW_ALLOW_SSE42 )
endif()

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
    if( DAW_JSON_COVERAGE )
        add_compile_options( -fprofile-instr-generate -fcoverage-mapping )
        add_link_options( -fprofile-instr-generate -fcoverage-mapping )
    endif()
    if( MSVC )
        message( STATUS "Clang-CL ${CMAKE_CXX_COMPILER_VERSION} detected" )
        add_definitions( -DNOMINMAX -DD_WIN32_WINNT=0x0601 )
        set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG /permissive- /EHsc" )
        set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 -DNDEBUG /permissive- /EHsc" )
        if( DAW_WERROR )
            add_compile_options( /WX )
        endif()
        if( DAW_ALLOW_SSE42 )
            message( STATUS "Using /arch:AVX2" )
            add_compile_options( /arch:AVX2 )
        endif()
    else()
        message( STATUS "Clang ${CMAKE_CXX_COMPILER_VERSION} detected" )
        add_compile_options(
                -pedantic
                -Weverything
                -ftemplate-backtrace-limit=0
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-covered-switch-default
                -Wno-documentation
                -Wno-exit-time-destructors
                -Wno-float-equal
                -Wno-missing-prototypes
                -Wno-newline-eof
                -Wno-padded
                -Wno-unused-template
                -Wno-weak-vtables
                # This is for when specializing things like tuple_size and each implementer gets to choose struct/class
                -Wno-mismatched-tags
        )
        if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
            AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14 )
            add_compile_options(
                    -Wno-c++20-compat
            )
        endif()
        if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" OR
            CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10.0.0 )
            add_compile_options( -Wno-poison-system-directories )
        endif()
        if( DAW_WERROR )
            if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang"
                OR CMAKE_CXX_COMPILER_VERSION LESS 13.0.0 )
                add_compile_options( -Werror -pedantic-errors )
            endif()
            # Cannot add trapv for testing, it breaks 128bit processing on clang/libc++
            # https://bugs.llvm.org/show_bug.cgi?id=16404
            string( FIND "$ENV{CXXFLAGS}" "-stdlib=libc++" HAS_LIBCXX )
            if( HAS_LIBCXX EQUAL -1 )
                add_compile_options( -ftrapv )
            endif()
        endif()
        if( CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" )
            if( NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" )
                if( CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10.0.0 )
                    message( STATUS "Adding Intel JCC bugfix" )
                    add_compile_options( -mbranches-within-32B-boundaries )
                endif()
            endif()
        endif()
        if( DAW_ALLOW_SSE42 )
            message( STATUS "Using -march=native" )
            add_compile_options( -march=native )
        endif()
        set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG" )
        set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -DNDEBUG" )

        if( DAW_JSON_USE_SANITIZERS )
            message( STATUS "Using sanitizers" )
            #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=null")
            #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=null")
            set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=undefined" )
            set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=address" )
            set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined" )
            set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" )
        endif()
        if( CMAKE_BUILD_TYPE STREQUAL "coverage" OR CODE_COVERAGE )
            set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-instr-generate -fcoverage-mapping" )
            set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-instr-generate -fcoverage-mapping" )
        endif()
    endif()
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
    if( DAW_JSON_COVERAGE )
        add_compile_options( -fprofile-instr-generate -fcoverage-mapping )
    endif()
    message( STATUS "g++ ${CMAKE_CXX_COMPILER_VERSION} detected" )
    add_compile_options( --param max-gcse-memory=260000000
                         -Wall
                         -Wextra
                         -pedantic
                         -Wpedantic
                         -Wconversion
                         -Wduplicated-cond
                         -Wlogical-op
                         -Wold-style-cast
                         -Wshadow
                         -Wzero-as-null-pointer-constant
                         )
    #-Wno-deprecated-declarations
    if( DAW_WERROR )
        add_compile_options( -Werror -pedantic-errors -ftrapv )
    endif()
    #-Wdisabled-optimization)
    if( CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" )
        if( CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 9.0.0 )
            if( LINUX )
                message( STATUS "Adding Intel JCC bugfix" )
                add_compile_options( -Wa,-mbranches-within-32B-boundaries )
            endif()
        endif()
    endif()
    if( DAW_ALLOW_SSE42 )
        message( STATUS "Using -march=native" )
        add_compile_options( -march=native )
    endif()
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -DNDEBUG" )

    if( DAW_JSON_USE_SANITIZERS )
        message( STATUS "Using sanitizers" )
        #UBSAN makes constexpr code paths not constexpr	on gcc9-11
        #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=undefined")
        set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=address" )
        #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
        set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" )
    endif()
elseif( MSVC )
    message( STATUS "MSVC detected" )
    add_definitions( -DNOMINMAX -DD_WIN32_WINNT=0x0601 )
    add_compile_options( "/permissive-" )
    add_compile_options( "/wd4146" )
    add_compile_options( "/bigobj" )
    add_compile_options( "/w14868" )
    # Ensure that string pooling is enabled. Otherwise it breaks constexpr string literals.
    # This affects debug modes by default, but optionally Release
    # https://developercommunity.visualstudio.com/t/codegen:-constexpr-pointer-to-trailing-z/900648
    add_compile_options( "/GF" )
    if( DAW_WERROR )
        if( CMAKE_CXX_FLAGS MATCHES "/W[0-4]" )
            string( REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
        else()
            add_compile_options( /W4 )
        endif()
        add_compile_options( /wd4127 /wd4141 /WX )
        add_definitions( -D_CRT_SECURE_NO_WARNINGS )
    endif()
    if( DAW_JSON_USE_SANITIZERS )
        add_compile_options( /fsanitize=address )
    endif()
else()
    message( STATUS "Unknown compiler id ${CMAKE_CXX_COMPILER_ID}" )
endif()

