cmake_minimum_required(VERSION 3.30)

project(rin C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Git REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(glad STATIC src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC include)

# --- ソースファイルの取得 ---
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 共通コンパイル/リンク設定
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -O0
    -Wall
    -Werror
    -Wextra
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wpedantic
    -Wunreachable-code
    -Wimplicit-fallthrough
    -Wnoexcept
    -D_GLIBCXX_DEBUG
    -D_GLIBCXX_ASSERTIONS
    -D_FORTIFY_SOURCE=2
    -fstack-protector-strong
    -fstack-clash-protection
    -fcf-protection
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fno-omit-frame-pointer
    -fcontracts
    -mtune=native
    -march=native
    -flto
)
target_link_options(project_warnings INTERFACE
    -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow
    -fcontracts
)

# --- ヘッダーオンリーライブラリの定義 ---
add_library(rin_lib INTERFACE)

# ヘッダーディレクトリおよび依存関係の追加（INTERFACE のため INTERFACE キーワードを使用）
target_include_directories(rin_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# メイン実行ファイル ターゲット
add_executable(rin)
target_sources(rin PRIVATE ${MAIN_SOURCES})
target_link_libraries(rin PRIVATE
    rin_lib
    project_warnings
    glad
    glfw
    OpenGL::GL
    glm::glm
)