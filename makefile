# Makefile personalizzato per Haiku I2C Bus Manager

# Compilatore e linker
CXX = g++
LD = ld

# Nome del modulo
TARGET = i2c

# File sorgente
SRCS = i2c_driver.cpp i2c_low_level.cpp i2c_transfer.cpp

# File oggetto
OBJS = $(SRCS:.cpp=.o)

# Flags del compilatore
CXXFLAGS = -c -iquote./ -Wno-multichar -D_KERNEL_MODE=1 -O3 -DDEBUG_MAX_LEVEL_FLOW=0 -DDEBUG_MAX_LEVEL_INFO=3

# Flags del linker
LDFLAGS = -r

# Percorso di installazione
INSTALL_DIR = /boot/home/config/non-packaged/add-ons/kernel/drivers/bin

# Regola predefinita
all: $(TARGET)

# Regola per la compilazione
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# Regola per il linking
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Regola per l'installazione
install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp -f $(TARGET) $(INSTALL_DIR)

# Regola per la pulizia
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all install clean