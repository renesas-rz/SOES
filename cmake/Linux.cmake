if(RPI_VARIANT)
  set (SOES_DEMO applications/raspberry_lan9252demo)
  set(HAL_SOURCES
	${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.c
	${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.h
	)
else()
  set(SOES_DEMO applications/linux_lan9252demo)
  set(HAL_SOURCES
	${SOES_SOURCE_DIR}/soes/hal/linux-lan9252/esc_hw.c
	)
endif()

if(RENESAS_RZT2H_ESC)
  set (SOES_DEMO applications/renesas_rzt2h_demo)
  set(HAL_SOURCES
	${SOES_SOURCE_DIR}/soes/hal/renesas-rzt2h-esc/esc_hw.c
	${SOES_SOURCE_DIR}/soes/hal/renesas-rzt2h-esc/esc_hw.h
	)

  include_directories(
	${SOES_SOURCE_DIR}/soes/hal/renesas-rzt2h-esc
	)
endif()

include_directories(
  ${SOES_SOURCE_DIR}/soes/include/sys/gcc
  ${SOES_SOURCE_DIR}/${SOES_DEMO}
  )

# Common compile flags
add_compile_options(-Wall -Wextra -Wconversion -Wno-unused-parameter -Werror)
