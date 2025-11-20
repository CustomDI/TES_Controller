# TES Controller Quick Reference

## Installation
```bash
pip install -r requirements.txt
cp config_example.yaml config.yaml
# Edit config.yaml with your serial port
```

## Basic Usage
```python
from tes_controller import DeviceController

with DeviceController.from_config('config.yaml') as ctrl:
    # Your code here
    pass
```

## Channel Selection Patterns

| Code | Returns | Description |
|------|---------|-------------|
| `ctrl.tes_get_all(channel=1)` | `dict` | Single channel |
| `ctrl.tes_get_all(channel=[1,3,5])` | `list[dict]` | Specific channels |
| `ctrl.tes_get_all()` | `list[dict]` | All channels |

## TES Operations

### Getters
```python
ctrl.tes_get_all(channel=1)      # All parameters
ctrl.tes_get_current(channel=1)  # Current
ctrl.tes_get_power(channel=1)    # Power
ctrl.tes_get_shunt(channel=1)    # Shunt voltage
ctrl.tes_get_bus(channel=1)      # Bus voltage
ctrl.tes_get_bits(channel=1)     # DAC bits
```

### Setters
```python
# Single channel
ctrl.tes_set_current(channel=1, current_mA=5.0)

# All channels, same value
ctrl.tes_set_current(current_mA=5.0)

# All channels, different values
ctrl.tes_set_current(current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])

# Specific channels, same value
ctrl.tes_set_current(channel=[1,3,5], current_mA=5.0)

# Specific channels, different values
ctrl.tes_set_current(channel=[1,3], current_mA=[1.0, 2.0])
```

### Control
```python
ctrl.tes_enable(channel=1)        # Enable channel 1
ctrl.tes_enable(channel=[1,2,3])  # Enable channels 1,2,3
ctrl.tes_enable()                 # Enable all

ctrl.tes_disable(channel=1)       # Disable channel 1
ctrl.tes_disable()                # Disable all
```

### Incremental Adjustments
```python
ctrl.tes_inc_current(channel=1, delta=100)  # Increment by 100
ctrl.tes_dec_current(channel=1, delta=50)   # Decrement by 50
ctrl.tes_set_bits(channel=1, value=0x80000) # Set DAC bits directly
```

## LNA Operations

### Always specify target: 'GATE' or 'DRAIN'

```python
# Get data
ctrl.lna_get_all(channel=1, target='GATE')
ctrl.lna_get_current(channel=1, target='DRAIN')
ctrl.lna_get_power(channel=1, target='GATE')

# Set DAC
ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)
ctrl.lna_set_dac(target='GATE', value=0x8000)  # All channels

# Enable/Disable
ctrl.lna_enable(channel=1, target='GATE')
ctrl.lna_enable(target='GATE')  # All channels
ctrl.lna_disable(channel=1, target='DRAIN')
```

Note on physical units and polarity
----------------------------------

`lna_set_dac` accepts either raw DAC integers (0-0xFFFF) or numeric physical values (floats) representing voltages or currents; the driver converts them to DAC codes. When using the ``GATE`` target, pass positive values — the driver will apply the correct negative polarity to the hardware.

Quick examples
--------------

```python
# Raw DAC code
ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)

# Physical value (volts or mA depending on driver)
ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)

# Multiple channels, different values
ctrl.lna_set_dac(target='GATE', value=[1.0, 1.1, 1.2, 1.3, 1.4, 1.5])
```

Additional LNA setters
----------------------

The `DeviceController` also exposes convenience setters that accept physical units:

```python
# Set voltage (volts) on channel 1
ctrl.lna_set_voltage(channel=1, target='GATE', voltage_V=1.2)

# Set current (mA) on multiple channels
ctrl.lna_set_current(target='GATE', current_mA=[0.5, 1.0, 1.5, 2.0, 2.5, 3.0])
```

These wrappers call `LnaController.set_voltage(...)` and `LnaController.set_current(...)` for
the bound per-channel controllers.

## DAC Operations

```python
ctrl.dac_set(value=0x8000)  # Set global DAC
ctrl.dac_get()              # Get global DAC value
```

## Error Handling

```python
from tes_controller.drivers import CommandError

try:
    data = ctrl.tes_get_all(channel=1)
except CommandError as e:
    print(f"Command error: {e}")
except ValueError as e:
    print(f"Invalid parameter: {e}")
```

## Common Patterns

### Initialize All Channels
```python
ctrl.tes_disable()  # Disable all
ctrl.tes_set_current(current_mA=0.0)  # Set all to 0
```

### Configure Multiple Channels
```python
ctrl.tes_set_current(
    channel=[1, 2, 3],
    current_mA=[5.0, 6.0, 7.0]
)
ctrl.tes_enable(channel=[1, 2, 3])
```

### Read All Channels
```python
data = ctrl.tes_get_current()  # Returns list of dicts
for i, d in enumerate(data, 1):
    print(f"Channel {i}: {d['current_mA']} mA")
```

### Current Sweep
```python
for current in [1.0, 2.0, 3.0, 4.0, 5.0]:
    ctrl.tes_set_current(channel=1, current_mA=current)
    time.sleep(0.1)
    data = ctrl.tes_get_current(channel=1)
    print(data)
```

## Direct Controller Access

```python
# Get direct access to channel controller
tes1 = ctrl.get_tes_controller(channel=1)

# No need to specify channel anymore
tes1.enable()
tes1.set_current(current_mA=5.0)
data = tes1.get_all()
```

## Configuration File (config.yaml)

```yaml
port: /dev/ttyACM0  # Your serial port
baud: 115200
timeout: 1.0
num_tes: 6
num_lna: 6
```

## Documentation

- **Full Docs**: https://tes-controller.readthedocs.io/
- **Examples**: `python/example.py`
- **API Reference**: See docs or use `help(ctrl.tes_set_current)`

## Command Line

```bash
# Run example
python example.py

# Build docs
cd docs && make html

# Find serial port (macOS)
ls /dev/tty.*

# Find serial port (Linux)
ls /dev/ttyACM* /dev/ttyUSB*
```
