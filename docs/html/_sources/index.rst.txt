TES Controller Documentation
============================

Welcome to the TES Controller Python client documentation!

The TES Controller is a Python interface for controlling Transition Edge Sensor
(TES) and Low Noise Amplifier (LNA) hardware via serial communication.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   installation
   getting-started
   usage
   examples
   api
   hardware-guide

Key Features
------------

- **Simple API**: High-level helpers for TES, LNA, and flux ramp control
- **Flexible Channel Selection**: Address individual channels or batches with one call
- **Typed Interfaces**: Rich type hints for IDE autocomplete and static analysis
- **Comprehensive Error Handling**: Deterministic `CommandError` hierarchy for driver faults
- **Config Driven**: Optional YAML configuration keeps serial parameters out of code
- **Context Manager Support**: Clean resource management with ``with`` statements

Quick Start
-----------

.. code-block:: python

   from tes_controller import DeviceController

   # Load settings from YAML
   with DeviceController.from_config('python/config.yaml') as ctrl:
      ctrl.tes_enable(channel=[1, 2, 3])
      ctrl.tes_set_current(channel=[1, 2, 3], current_mA=[5.0, 4.5, 4.0])
      reading = ctrl.tes_get_all(channel=1)
      print(reading)

   # Or construct manually when you need to override serial defaults
   ctrl = DeviceController(port='/dev/ttyACM0', baud=115200, timeout=1.0)
   try:
      ctrl.tes_set_current(current_mA=5.0)
   finally:
      ctrl.close()

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
