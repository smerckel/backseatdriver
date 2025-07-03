# Makefile for Arduino projects

# Arduino CLI path (adjust if needed)
ARDUINO_CLI = arduino-cli

# Board settings (adjust for your specific board)
BOARD = arduino:avr:uno
PORT = /dev/ttyACM0

# Sketch name
SKETCH = bsd_test.ino

# Compile
compile:
	$(ARDUINO_CLI) compile --fqbn $(BOARD) .

# Upload
upload:
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(BOARD) .

# Clean build files
clean:
	rm -rf build

# All-in-one compile and upload
all: compile upload
