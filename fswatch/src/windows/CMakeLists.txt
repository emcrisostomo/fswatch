set(LIBRARY_NAME "getopt")

set(header_files "${LIBRARY_NAME}.h")
source_group("Header Files" FILES "${header_files}")

set(src_files "${LIBRARY_NAME}.c")
source_group("Source Files" FILES "${src_files}")

add_library("${LIBRARY_NAME}" "${header_files}" "${src_files}")
