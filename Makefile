CC = arm-none-eabi-gcc
BIN := RTOSDemo.axf

BUILD_DIR := build

FREERTOS_DIR_REL := .
#FreeRTOS
FREERTOS_DIR := $(FREERTOS_DIR_REL)
#$(abspath $(FREERTOS_DIR_REL))
KERNEL_DIR := $(FREERTOS_DIR)/Source

FREERTOS_PLUS_DIR_REL := FreeRTOS-Plus
FREERTOS_PLUS_DIR := $(FREERTOS_PLUS_DIR_REL)
# $(abspath $(FREERTOS_PLUS_DIR_REL))

SOURCE_FILES += init/startup.c  syscall.c main.c uart.c
SOURCE_FILES += $(KERNEL_DIR)/portable/GCC/ARM_CM3/port.c
SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/timers.c
SOURCE_FILES += $(KERNEL_DIR)/event_groups.c
SOURCE_FILES += ${KERNEL_DIR}/portable/MemMang/heap_3.c

# ringbuffer
SOURCE_FILES += $(FREERTOS_DIR)/Util/lwrb/lwrb.c
SOURCE_FILES += $(FREERTOS_DIR)/Util/lwrb/lwrb_ex.c

SOURCE_FILES += $(FREERTOS_DIR)/Util/buffer/buffer.c
SOURCE_FILES += $(FREERTOS_DIR)/Util/log.c

# slip serial
SOURCE_FILES += $(FREERTOS_DIR)/serial/slip.c


# fs
# SOURCE_FILES += $(FREERTOS_DIR)/fs/lfs.c
# SOURCE_FILES += $(FREERTOS_DIR)/fs/lfs_util.c
# SOURCE_FILES += $(FREERTOS_DIR)/fs/bd/lfs_filebd.c
# SOURCE_FILES += $(FREERTOS_DIR)/fs/bd/lfs_rambd.c
# SOURCE_FILES += $(FREERTOS_DIR)/fs/bd/lfs_sdhcidb.c

INCLUDE_DIRS += -I$(FREERTOS_DIR)
# INCLUDE_DIRS += -I$(FREERTOS_DIR)/fs
INCLUDE_DIRS += -I$(FREERTOS_DIR)/Util
INCLUDE_DIRS += -I$(FREERTOS_DIR)/Util/lwrb
INCLUDE_DIRS += -I$(FREERTOS_DIR)/Util/buffer
INCLUDE_DIRS += -I$(FREERTOS_DIR)/CMSIS
INCLUDE_DIRS += -I$(KERNEL_DIR)/include
INCLUDE_DIRS += -I$(KERNEL_DIR)/serial
INCLUDE_DIRS += -I$(KERNEL_DIR)/portable/GCC/ARM_CM3

ifeq ($(FULL_DEMO), 1)
    SOURCE_FILES += main_full.c
    SOURCE_FILES += $(KERNEL_DIR)/stream_buffer.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/AbortDelay.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/BlockQ.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/blocktim.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/countsem.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/death.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/dynamic.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/EventGroupsDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/flop.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/GenQTest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/integer.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/IntSemTest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/MessageBufferAMP.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/MessageBufferDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/PollQ.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/QPeek.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/QueueOverwrite.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/QueueSet.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/QueueSetPolling.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/recmutex.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/semtest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/StaticAllocation.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/StreamBufferDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/StreamBufferInterrupt.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/TaskNotify.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Minimal/TimerDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Common/Full/print.c

    INCLUDE_DIRS += -I$(FREERTOS_DIR)/Common/include
    INCLUDE_DIRS += -I${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include/

    CFLAGS := -DmainCREATE_FULL_DEMO_ONLY=1
else
    SOURCE_FILES += main_blinky.c

    CFLAGS := -DmainCREATE_SIMPLE_BLINKY_DEMO_ONLY=1
endif

DEFINES :=  -DQEMU_SOC_MPS2 -DHEAP3

LDFLAGS = -T ./scripts/mps2_m3.ld -specs=nano.specs --specs=rdimon.specs -lc -lrdimon
LDFLAGS += -Xlinker -Map=${BUILD_DIR}/output.map

CFLAGS += -nostartfiles -mthumb -mcpu=cortex-m3 -Wno-error=implicit-function-declaration
CFLAGS += -Wno-builtin-declaration-mismatch 
#-Werror
CFLAGS += -Wall -Wextra

ifeq ($(DEBUG), 1)
    CFLAGS += -ggdb3 -Og
    # CFLAGS += -g
else
    CFLAGS += -O3
endif
    CFLAGS += -fstrict-aliasing -Wstrict-aliasing -Wno-error=address-of-packed-member

OBJ_FILES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

CPPFLAGS += $(DEFINES)
CFLAGS += $(INCLUDE_DIRS)

.PHONY: clean

#$(BIN) 

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES)
	$(CC) -ffunction-sections -fdata-sections $(CFLAGS) $(LDFLAGS) $+ -o $(@)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

INCLUDES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.d)
-include $(INCLUDES)

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@

clean:
	-rm -rf build

