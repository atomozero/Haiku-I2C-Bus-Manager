# Haiku I2C Bus Manager

## Overview

This project implements an I2C bus manager for Haiku OS, providing the foundational infrastructure for I2C communications. It's designed to facilitate the development of specific I2C device drivers on Haiku OS.

## Features

- Low-level I2C operations (start, stop, byte send/receive)
- Hardware-level I2C bus management
- Interface for I2C device driver development
- Detailed logging for debugging

## Requirements

- Haiku OS
- GPIO access for SCL and SDA pins

## Building

To compile the bus manager, run the following command in the project directory:

```bash
make
```

## Installation

[Installation instructions to be added]

## Usage

To use this I2C bus manager in your Haiku I2C device driver:

1. Include the necessary headers in your driver code.
2. Initialize the I2C bus using the provided API.
3. Use the `i2c_transfer` function for I2C communications.

Detailed API documentation will be provided soon.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is released under the MIT License. See the `LICENSE` file for details.


---

**Note:** This project is currently in development. API and functionality may change.
