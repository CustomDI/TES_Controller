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

