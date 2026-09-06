#Makefile for STM32f411retx project

#Project Configuration
Target = sentinel-node
Build_Dir = ./build
Common = .
Hardware = STM32F411RE

#Common is a path to linker script, satrtup file, and header files

#Source Files
C_Sources = $(wildcard app/*.c) \
            $(wildcard drivers/*.c) \
			$(wildcard platform/*.c)
			
ASM_Sources = $(Common)/platform/startup_stm32f411retx.s

#Toolchain 
Prefix = arm-none-eabi-
CC = $(Prefix)gcc
AS = $(Prefix)gcc -x assembler-with-cpp
LD = $(Prefix)gcc
OBJCOPY = $(Prefix)objcopy
OBJDUMP = $(Prefix)objdump
SIZE = $(Prefix)size
GDB = $(Prefix)gdb

#------------------------------------------------------------------------------------------------

#MCU Flags
#-mcpu=cortex-m4 --> targets the cortex m4 core
#-mthumb --> uses the thumb instructions for function calls
#-mfloat-abi=soft --> uses the soft floating point ABI
MCU = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

#Compiler Flags
CFLAGS = $(MCU)
CFLAGS += -std=c11 
CFLAGS += -Wall -Wextra -Wpedantic -Wshadow
CFLAGS += -Wdouble-promotion -Wformat=2 -Wformat-truncation
CFLAGS += -fno-common -ffunction-sections -fdata-sections
CFLAGS += -Os
CFLAGS += -g3 -gdwarf-2 #all the deatils of this flags can be found in the 00-toolchain-setup/Makefile

#Include Paths
CFLAGS += -I $(Common)/vendor/CMSIS/Include
CFLAGS += -I $(Common)/vendor/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -DSTM32F411xE

#Assembler Flags
ASFLAGS += $(MCU) -g3 -gdwarf-2

#Linker Flags
LDSCRIPT = $(Common)/platform/stm32f411retx_flash.ld
LDFLAGS = $(MCU)
LDFLAGS += -T $(LDSCRIPT)
LDFLAGS += --specs=nosys.specs  # No OS system calls
LDFLAGS += -Wl,--gc-sections    # Remove unused code sections
LDFLAGS += -Wl,-Map=$(Build_Dir)/$(Target).map,--cref #generate a map file and put it in the build directory
LDFLAGS += -lnosys

#Object Files
OBJECTS = $(addprefix $(Build_Dir)/, $(notdir $(C_Sources:.c=.o)))  #replace the .c with.o at the end of the end each file in the C_Sources directory
OBJECTS += $(addprefix $(Build_Dir)/, $(notdir $(ASM_Sources:.s=.o)))


#Vpaths

vpath %.c  $(sort $(dir $(C_Sources)))
vpath %.s  $(sort $(dir $(ASM_Sources)))

#Phony Commands - It's a protection
.PHONY: all clean flash openocd debug size disasm

#Build Rules

all: $(Build_Dir)/$(Target).bin $(Build_Dir)/$(Target).hex
	@echo "=== Build Complete ==="
	@$(SIZE) $(Build_Dir)/$(Target).elf

#Compile C
$(Build_Dir)/%.o: %.c | $(Build_Dir)
	@echo "Compiling:  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

#Assemble ASM
$(Build_Dir)/%.o: %.s | $(Build_Dir)
	@echo "Asembling:  $<"
	@$(AS) $(ASFLAGS) -c $< -o $@
#Link
$(Build_Dir)/$(Target).elf: $(OBJECTS)
	@echo "Linking:  $<"
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@
#ELF --> Binary
$(Build_Dir)/$(Target).bin: $(Build_Dir)/$(Target).elf | $(Build_Dir)
	@echo "Generating Binary:  $<"
	@$(OBJCOPY) -O binary $< $@

#ELF --> HEX
$(Build_Dir)/$(Target).hex: $(Build_Dir)/$(Target).elf | $(Build_Dir)
	@echo "Generating HEX:  $<"
	@$(OBJCOPY) -O ihex $< $@
#Create Build Directory
$(Build_Dir):
	@mkdir -p $@

#Flash via OpenOCD
# Flash base address on STM32F4 = 0x08000000 (see linker script & RM0383 §3.3).
# Order of args inside `program`: <file> [offset] verify reset exit
flash: $(Build_Dir)/$(Target).bin | $(Build_Dir)
	@echo "Flashing the $< to the $(Hardware) board!!!"
	openocd -f $(Common)/openocd.cfg -c "program $< 0x08000000 verify reset exit"

# ---- Start OpenOCD debug server ----
openocd:
	openocd -f $(Common)/tools/openocd.cfg

#Launch GDB
debug: $(Build_Dir)/$(Target).elf
	$(GDB) -ex "target remote :3333" \
	       -ex "monitor reset halt" \
	       -ex "break main" \
	       -ex "continue" \
	       $(Build_Dir)/$(Target).elf

#Show Memory Usage
size: $(Build_Dir)/$(Target).elf
	@echo "Memory Usage:  $<"
	$(SIZE) --format=berkeley $<

#Disassemble
disasm: $(Build_Dir)/$(Target).elf
	@echo "Disassembling $< into $(Build_Dir)/$(Target).dis"
	$(OBJDUMP) -d -S $< > $(Build_Dir)/$(Target).dis

#make disasm runs objdump -d -S to disassemble the ELF — turn the machine code back into human-readable ARM assembly

#Clean
clean:
	@echo "Cleaning up the build directory"
	@rm -rf $(Build_Dir)