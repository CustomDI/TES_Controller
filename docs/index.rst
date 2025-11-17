TES Controller Documentation
============================

Welcome to the TES Controller Python client documentation!

The TES Controller is a Python interface for controlling Transition Edge Sensor (TES) and Low Noise Amplifier (LNA) hardware via serial communication.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   installation
   getting-started
   examples
   api

Key Features
------------

- **Simple API**: Easy-to-use interface for TES and LNA control
- **Flexible Channel Selection**: Control single channels, multiple channels, or all channels at once
- **Type-Safe**: Full type hints for better IDE support
- **Error Handling**: Comprehensive error handling with CommandError
- **YAML Configuration**: Load settings from configuration files
- **Context Manager Support**: Clean resource management with ``with`` statements

Quick Start
-----------

.. code-block:: python

   from tes_controller import DeviceController

   # Load from config file
   with DeviceController.from_config('config.yaml') as ctrl:
       # Get data from channel 1
       data = ctrl.tes_get_all(channel=1)
       
       # Set current on all channels
       ctrl.tes_set_current(current_mA=5.0)
       
       # Enable specific channels
       ctrl.tes_enable(channel=[1, 2, 3])

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
