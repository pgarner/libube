#
# Download kissfft
# The output is always linked to "kissfft" to avoid version variables
#
set(KISSSRC "http://sourceforge.net/projects/kissfft/files/kissfft/v1_3_0")
set(KISSFFT "kiss_fft130")
set(KISSDIR "${CMAKE_CURRENT_SOURCE_DIR}/${KISSFFT}")
if (NOT EXISTS "${KISSDIR}")
  message(STATUS "Downloading and linking ${KISSFFT}")
  file(DOWNLOAD
    "${KISSSRC}/${KISSFFT}.tar.gz"
    "${KISSDIR}.tar.gz"
    )
  execute_process(COMMAND
    tar -C ${CMAKE_CURRENT_SOURCE_DIR} -zxf ${KISSDIR}.tar.gz
    )
  execute_process(COMMAND
    ln -sf ${KISSDIR} ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
    )
endif()
