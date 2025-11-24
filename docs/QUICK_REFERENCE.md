# TES Controller Quick Reference

## Installation

```bash
pip install -r python/requirements.txt
cp python/config_example.yaml python/config.yaml
```

## Basic Pattern

```python
from tes_controller import DeviceController

with DeviceController.from_config('python/config.yaml') as ctrl:
    ctrl.tes_enable(channel=1)
    ctrl.tes_set_current(channel=1, current_mA=5.0)
    print(ctrl.tes_get_all(channel=1))
```

## Channel Selection Cheatsheet

| Call | Description | Return Type |
|------|-------------|-------------|
| `ctrl.tes_get_all(channel=1)` | Single channel | `dict` |
| `ctrl.tes_get_all(channel=[1, 3, 5])` | Subset of channels | `list[dict]` |
| `ctrl.tes_get_all()` | All channels | `list[dict]` |
| `ctrl.tes_set_current(current_mA=5.0)` | Same value on all channels | `list[dict]` |
| `ctrl.tes_set_current(channel=[1, 3], current_mA=[1.0, 2.0])` | Pair channel list with value list | `list[dict]` |

## TES Helpers

```python
ctrl.tes_set_current(channel=1, current_mA=5.0)
ctrl.tes_inc_current(channel=1, delta=100)
ctrl.tes_dec_current(channel=1, delta=50)
ctrl.tes_set_bits(channel=1, value=0x80000)
ctrl.tes_get_current(channel=1)
ctrl.tes_get_power(channel=1)
```

## LNA Helpers

Always supply `target='GATE'` or `target='DRAIN'`.

```python
ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)
ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)    # physical units
ctrl.lna_enable(target='GATE')
ctrl.lna_disable(channel=[1, 2, 3], target='GATE')
ctrl.lna_set_voltage(channel=1, target='GATE', voltage_V=1.2)
ctrl.lna_set_current(target='GATE', current_mA=[0.5, 1.0, 1.5, 2.0, 2.5, 3.0])
```

## Flux Ramp DAC

```python
ctrl.flux_ramp_set(0x8000)
ctrl.flux_ramp_get()
```

## Error Handling

```python
from tes_controller.drivers import CommandError

try:
    ctrl.tes_set_current(channel=1, current_mA=5.0)
except CommandError as exc:
    print(f"Hardware command failed: {exc}")
```

## CLI Snippets

```bash
# Build documentation
make -C docs html

# Live rebuild during authoring
sphinx-autobuild docs docs/_build/html
```
