cmake_minimum_required(VERSION 3.0)

project(constExprSmallPtParallel)

set(CMAKE_CXX_STANDARD 20)

set(WIDTH 64)
set(HEIGHT 48)
set(SAMPS 128)
set(TargetList)

add_compile_options(-fconstexpr-ops-limit=335544320)
add_compile_options(-ftemplate-depth=786435)
add_compile_options(-export)
add_compile_options(-export_dir=${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-O3)

add_executable(smallpt mainPt.cpp)
target_compile_definitions(smallpt PRIVATE WIDTH=${WIDTH} HEIGHT=${HEIGHT} SAMPS=${SAMPS})

math(EXPR WIDTH_IT "${WIDTH}-1")
foreach(W RANGE ${WIDTH_IT})
    math(EXPR HEIGHT_IT "${HEIGHT}-1")
    foreach(H RANGE ${HEIGHT_IT})
        add_library(p_${W}_${H} OBJECT smallpt_parallel.cpp)
        target_compile_definitions(p_${W}_${H} PRIVATE WIDTH=${WIDTH} HEIGHT=${HEIGHT} SAMPS=${SAMPS} W=${W} H=${H})
        target_sources(smallpt PRIVATE $<TARGET_OBJECTS:p_${W}_${H}>)
    endforeach(H RANGE HEIGHT)
endforeach(W RANGE WIDTH)


