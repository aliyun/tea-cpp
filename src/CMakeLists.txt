# 1. Source files - Explicitly listing files is preferred, but GLOB is kept if you have many files
file(GLOB_RECURSE CPP_FILES "*.cpp")

# 2. Add library - The target name is inherited from PROJECT_NAME in the root
add_library(${PROJECT_NAME} ${CPP_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# 3. Public header directories
# Using CMAKE_CURRENT_SOURCE_DIR to ensure paths are absolute and robust
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# 4. Link dependencies 
# We assume the root CMakeLists.txt has already found these packages.
# Using Targets (::) ensures that include directories and compile definitions are inherited.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
)

# 5. Conditional dependencies for static builds
if(NOT BUILD_SHARED_LIBS)
  # ZLIB (Required for static curl or compressed responses)
  find_package(ZLIB QUIET)
  if(TARGET ZLIB::ZLIB)
    target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
  endif()
  
  # Linux static curl dependencies
  # Ubuntu/Debian's static libcurl is built with many optional features that require additional libraries
  # Set CURL_AUTO_DEPS=OFF if you're using a minimal/custom libcurl build
  if(CURL_AUTO_DEPS AND UNIX AND NOT APPLE)
    # nghttp2 - HTTP/2 support
    find_library(NGHTTP2_LIBRARY NAMES nghttp2)
    if(NGHTTP2_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${NGHTTP2_LIBRARY})
    endif()
    
    # libssh - SSH/SFTP support
    find_library(SSH_LIBRARY NAMES ssh)
    if(SSH_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${SSH_LIBRARY})
    endif()
    
    # libpsl - Public Suffix List support
    find_library(PSL_LIBRARY NAMES psl)
    if(PSL_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${PSL_LIBRARY})
    endif()
    
    # GSS-API/Kerberos support
    find_library(GSSAPI_LIBRARY NAMES gssapi_krb5)
    if(GSSAPI_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${GSSAPI_LIBRARY})
    endif()
    
    # Kerberos 5
    find_library(KRB5_LIBRARY NAMES krb5)
    if(KRB5_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${KRB5_LIBRARY})
    endif()
    
    # LDAP support
    find_library(LDAP_LIBRARY NAMES ldap)
    find_library(LBER_LIBRARY NAMES lber)
    if(LDAP_LIBRARY AND LBER_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${LDAP_LIBRARY} ${LBER_LIBRARY})
    endif()
    
    # IDN support (internationalized domain names)
    find_library(IDN2_LIBRARY NAMES idn2)
    if(IDN2_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${IDN2_LIBRARY})
    endif()
    
    # Brotli compression
    find_library(BROTLIDEC_LIBRARY NAMES brotlidec)
    find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon)
    if(BROTLIDEC_LIBRARY AND BROTLICOMMON_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${BROTLIDEC_LIBRARY} ${BROTLICOMMON_LIBRARY})
    endif()
    
    # zstd compression
    find_library(ZSTD_LIBRARY NAMES zstd)
    if(ZSTD_LIBRARY)
      target_link_libraries(${PROJECT_NAME} PUBLIC ${ZSTD_LIBRARY})
    endif()
  endif()
endif()

# UUID (Linux specific)
if(UNIX AND NOT APPLE)
  # Try modern PkgConfig first for uuid
  find_package(PkgConfig QUIET)
  pkg_check_modules(UUID uuid)
  if(UUID_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${UUID_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIRS})
  else()
    # Fallback to manual search
    find_path(UUID_INCLUDE_DIR uuid/uuid.h)
    find_library(UUID_LIBRARY uuid)
    if(UUID_INCLUDE_DIR AND UUID_LIBRARY)
      target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIR})
      target_link_libraries(${PROJECT_NAME} PRIVATE ${UUID_LIBRARY})
    endif()
  endif()
endif()

# macOS specific Frameworks
if(APPLE)
  target_link_libraries(${PROJECT_NAME} PUBLIC "-framework CoreFoundation" "-framework Security")
endif()

# Windows specific libraries
if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# 6. Target Properties and Compiler Definitions
set_target_properties(${PROJECT_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  SOVERSION ${PROJECT_VERSION_MAJOR}
  VERSION ${PROJECT_VERSION}
)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE DARABONBA_CORE_BUILD)
  # For CMake 3.13, we ensure the MD/MDd runtime is set
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:/MD>
    $<$<CONFIG:Debug>:/MDd>
  )
endif()
