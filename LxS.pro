# -------------------------------------------------
# Project created by QtCreator 2012-10-22T17:04:49
# -------------------------------------------------
QT -= core \
    gui
TARGET = LxS
TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH += Include
LIBS += -pthread
SOURCES += Source/ls_log.c \
    Source/ls_list.c \
    Source/ls_common.c \
    Source/ls_uart.c \
    Source/ls_gen.c \
    Source/ls_agent.c \
    Source/ls_cli.c \
    Source/ls_tcp_server.c \
    Source/ls_modbus_client.c \
    Source/ls_udp_server.c
HEADERS += Include/ls_types.h \
    Include/ls_log.h \
    Include/ls_list.h \
    Include/ls_common.h \
    Include/ls_uart.h \
    Include/ls_gen.h \
    Include/ls_agent.h \
    Include/ls_include.h \
    Include/ls_cli.h \
    Include/ls_tcp_server.h \
    Include/ls_modbus_client.h \
    Include/ls_udp_server.h
