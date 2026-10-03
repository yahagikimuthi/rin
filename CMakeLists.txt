cmake_minimum_required(VERSION 3.30)

project(rin
    VERSION   1.0.0
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 依存パッケージの探索 ---
find_package(Git REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)

# --- C言語用 GLAD ライブラリターゲット ---
add_library(glad STATIC src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# --- ライブラリ定義 (rin_lib) ---
# 他のプロジェクトから FetchContent されるメインのターゲット
add_library(rin_lib INTERFACE)
add_library(rin::rin_lib ALIAS rin_lib) # 外部から rin::rin_lib で呼び出せるようにエイリアスを作成

# ヘッダーディレクトリの設定
target_include_directories(rin_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(rin_lib INTERFACE
    glad
    glfw
    OpenGL::GL
    glm::glm
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# 単体開発（トップレベルプロジェクト）時のみ有効化する設定
if(PROJECT_IS_TOP_LEVEL)
    # 警告オプションやデバッグ用フラグの設定
    add_library(project_warnings INTERFACE)
    target_compile_options(project_warnings INTERFACE
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
        -fcf-protection
        -mtune=native
        -march=native
        -flto
        -fcontracts
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>
        $<$<CONFIG:Debug>:-D_GLIBCXX_ASSERTIONS>
        $<$<CONFIG:Debug>:-D_FORTIFY_SOURCE=2>
        $<$<CONFIG:Debug>:-fstack-protector-strong>
        $<$<CONFIG:Debug>:-fstack-clash-protection>
        $<$<CONFIG:Debug>:-fsanitize=undefined,float-divide-by-zero,float-cast-overflow>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
    target_link_options(project_warnings INTERFACE
        -fcontracts
        $<$<CONFIG:Debug>:-fsanitize=undefined,float-divide-by-zero,float-cast-overflow>
        $<$<CONFIG:Debug>:-fsanitize=address>
    )

    # ソースファイルの取得とスタンドアロン実行ファイルの作成
    file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    add_executable(rin.exe)
    target_sources(rin.exe PRIVATE ${MAIN_SOURCES})
    target_link_libraries(rin.exe PRIVATE
        rin_lib
        project_warnings
    )

    target_include_directories(rin.exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()