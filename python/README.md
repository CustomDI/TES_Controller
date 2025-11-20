# TES Controller Python Client

A Python interface for controlling Transition Edge Sensor (TES) and Low Noise Amplifier (LNA) hardware via serial communication.

## 📚 Documentation

**Full documentation is available at:** [ReadTheDocs](https://tes-controller.readthedocs.io/) *(after deployment)*

Or build locally:
```bash
cd docs
make html
open _build/html/index.html
```

## Features

- **Simple API**: Easy-to-use interface for TES and LNA control
- **Flexible Channel Selection**: Control single channels, multiple channels, or all channels at once
- **Type-Safe**: Full type hints for better IDE support
- **Error Handling**: Comprehensive error handling with `CommandError`
- **YAML Configuration**: Load settings from configuration files
- **Context Manager Support**: Clean resource management with `with` statements

## Requirements

- Python 3.7+
- Serial port access for hardware communication

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/TES_Controller.git
cd TES_Controller/python
```

2. Install dependencies:
```bash
pip install -r requirements.txt
```

3. Configure your serial port:
```bash
cp config_example.yaml config.yaml
# Edit config.yaml with your port settings
```

## Quick Start

### Basic Usage

```python
from tes_controller import DeviceController

# Load from config file
with DeviceController.from_config('config.yaml') as ctrl:
    # Get data from channel 1
    data = ctrl.tes_get_all(channel=1)
    print(data)
    
    # Set current on channel 1
    ctrl.tes_set_current(channel=1, current_mA=5.0)
    
    # Enable the channel
    ctrl.tes_enable(channel=1)
```

### Multi-Channel Operations

```python
with DeviceController.from_config('config.yaml') as ctrl:
    # Get data from all channels
    all_data = ctrl.tes_get_all()
    
    # Set same current on all channels
    ctrl.tes_set_current(current_mA=5.0)
    
    # Set different currents on each channel
    ctrl.tes_set_current(current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
    
    # Enable specific channels
    ctrl.tes_enable(channel=[1, 3, 5])
```

### LNA Operations

```python
with DeviceController.from_config('config.yaml') as ctrl:
    # Set GATE voltage on all channels
    ctrl.lna_set_dac(target='GATE', value=0x8000)
    
    # Enable DRAIN on specific channels
    ctrl.lna_enable(channel=[1, 2, 3], target='DRAIN')
    
    # Get GATE data from channel 1
    data = ctrl.lna_get_all(channel=1, target='GATE')
```

Note on new LNA DAC functionality
---------------------------------

The LNA driver now supports setting DAC outputs using either raw DAC codes (integers, e.g. 0x8000) or physical quantities (floats) for convenience. When a float is provided the driver will convert the supplied value into the appropriate DAC code (for example a voltage in volts or a current in milliamps) according to the configured conversion in the driver.

Important: when using the `GATE` target, provide the physical current/voltage as a positive number — the driver maps gate values to the correct (negative) polarity applied to the hardware. In other words, pass positive numbers in your code even though the actual gate voltage/current on the device is negative.

Examples
~~~~~~~~

```python
# Using a raw DAC code (integer)
ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)

# Using a physical voltage (float) - driver converts and applies negative polarity
ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)  # 1.2 V (passed positive)

# Set multiple channels to different physical values (list length must equal num_lna)
ctrl.lna_set_dac(target='GATE', value=[1.0, 1.1, 1.2, 1.3, 1.4, 1.5])

# Set specific channels to the same physical value
ctrl.lna_set_dac(channel=[1,3,5], target='GATE', value=0.8)
```

LNA voltage/current convenience methods
---------------------------------------

In addition to `lna_set_dac(...)`, the `DeviceController` exposes convenience methods that accept
physical units and route to the underlying per-channel driver methods:

- `lna_set_voltage(channel, target, voltage_V)` — accepts a float (or list of floats) in volts (0.0–5.0 V).
- `lna_set_current(channel, target, current_mA)` — accepts a float (or list of floats) in milliamps (0.0–64.0 mA).

These are available on `DeviceController` and call the corresponding `LnaController.set_voltage(...)`
and `LnaController.set_current(...)` methods for the bound channel(s).

Developer note: while reviewing the code I noticed a small issue in `LnaController.get_shunt` where it
incorrectly calls `self.channel._check_target(target)` instead of `self._check_target(target)`. This is a
minor bug that should be fixed in the driver (it doesn't change the documented API names above).

## Channel Selection Patterns

The API supports flexible channel selection:

| Pattern | Example | Description |
|---------|---------|-------------|
| **Single channel** | `ctrl.tes_get_all(channel=1)` | Returns single dict |
| **Multiple channels** | `ctrl.tes_get_all(channel=[1,3,5])` | Returns list of dicts |
| **All channels** | `ctrl.tes_get_all()` | Returns list of dicts |
| **Same value, all channels** | `ctrl.tes_set_current(current_mA=5.0)` | Sets all to 5.0 mA |
| **Different values, all channels** | `ctrl.tes_set_current(current_mA=[1,2,3,4,5,6])` | Sets each to different value |
| **Different values, specific channels** | `ctrl.tes_set_current(channel=[1,3], current_mA=[1.0,2.0])` | Sets channels 1,3 to 1.0,2.0 |

## Examples

See the full example script:
```bash
python example.py
```

For more examples, see the [Examples Documentation](https://tes-controller.readthedocs.io/en/latest/examples.html).

## API Overview

### DeviceController Methods

**TES Operations:**
- `tes_get_all()`, `tes_get_current()`, `tes_get_power()`, `tes_get_shunt()`, `tes_get_bus()`, `tes_get_bits()`
- `tes_set_current()`, `tes_set_bits()`, `tes_inc_current()`, `tes_dec_current()`
- `tes_enable()`, `tes_disable()`

**LNA Operations:**
- `lna_get_all()`, `lna_get_current()`, `lna_get_power()`, `lna_get_shunt()`, `lna_get_bus()`
- `lna_set_dac()`, `lna_enable()`, `lna_disable()`

Additional LNA setters (convenience wrappers accepting physical units):
- `lna_set_voltage()`, `lna_set_current()`

**DAC Operations:**
- `dac_set()`, `dac_get()`

See the [API Reference](https://tes-controller.readthedocs.io/en/latest/api.html) for complete details.

## Error Handling

Always wrap operations in try/except blocks:

```python
from tes_controller import DeviceController
from tes_controller.drivers import CommandError

with DeviceController.from_config('config.yaml') as ctrl:
    try:
        data = ctrl.tes_get_all(channel=1)
        print(f"Success: {data}")
    except CommandError as e:
        print(f"Command error: {e}")
    except ValueError as e:
        print(f"Invalid parameter: {e}")
```

## Files

- `tes_controller/controller.py` — High-level `DeviceController` interface
- `tes_controller/drivers.py` — `TesController`, `LnaController`, `DacController` drivers
- `tes_controller/serial_client.py` — Serial communication layer
- `example.py` — Complete usage examples
- `config_example.yaml` — Example configuration file

## Hardware Communication

The controller communicates with the Arduino sketch via serial at 115200 baud. Commands are sent as text, and responses are returned as YAML-formatted blocks that are automatically parsed into Python dictionaries.

## Contributing

Contributions are welcome! Please see the documentation for development guidelines.

## License

*(Add your license here)*

## Support

- **Documentation**: https://tes-controller.readthedocs.io/
- **Issues**: https://github.com/yourusername/TES_Controller/issues
- **Examples**: See `example.py` and the [Examples Documentation](https://tes-controller.readthedocs.io/en/latest/examples.html)

