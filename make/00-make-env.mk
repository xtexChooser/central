SHELL = bash
.SHELLFLAGS += -e
.ONESHELL:

MAKE_FLAGS = -j4 --silent --no-builtin-rules --output-sync=target

TOUCH ?= touch
MKDIR ?= mkdir
PRINTF ?= printf
