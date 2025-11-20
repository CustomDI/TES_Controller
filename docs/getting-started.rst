Getting Started
===============

This guide will help you get started with the TES Controller Python client.

Basic Usage
-----------

Creating a Controller
~~~~~~~~~~~~~~~~~~~~~

There are two ways to create a ``DeviceController``:

**From a configuration file:**

.. code-block:: python

   from tes_controller import DeviceController

   ctrl = DeviceController.from_config('config.yaml')

**Direct instantiation:**

.. code-block:: python

   from tes_controller import DeviceController

   ctrl = DeviceController(
       port='/dev/ttyACM0',
       baud=115200,
       timeout=1.0,
       num_tes=6,
       num_lna=6
   )

Using Context Managers
~~~~~~~~~~~~~~~~~~~~~~

Always use a context manager to ensure proper cleanup:

.. code-block:: python

   with DeviceController.from_config('config.yaml') as ctrl:
       # Your code here
       data = ctrl.tes_get_all(channel=1)
   # Connection automatically closed

Channel Selection Patterns
---------------------------

The TES Controller API supports flexible channel selection:

Single Channel
~~~~~~~~~~~~~~

.. code-block:: python

   # Get data from channel 1
   data = ctrl.tes_get_all(channel=1)
   
   # Set current on channel 1
   ctrl.tes_set_current(channel=1, current_mA=5.0)

Multiple Specific Channels
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # Get data from channels 1, 3, and 5
   data = ctrl.tes_get_all(channel=[1, 3, 5])
   
   # Set same current on channels 1, 3, 5
   ctrl.tes_set_current(channel=[1, 3, 5], current_mA=5.0)

All Channels
~~~~~~~~~~~~

.. code-block:: python

   # Get data from all channels (returns list)
   data = ctrl.tes_get_all()
   
   # Set same current on all channels
   ctrl.tes_set_current(current_mA=5.0)

Setting Different Values
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # Set different currents on all channels (list must match num_tes)
   ctrl.tes_set_current(current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
   
   # Set different currents on specific channels
   ctrl.tes_set_current(
       channel=[1, 3, 5],
       current_mA=[1.0, 2.0, 3.0]
   )

TES Operations
--------------

Getting Data
~~~~~~~~~~~~

.. code-block:: python

   # Get all parameters
   data = ctrl.tes_get_all(channel=1)
   
   # Get specific parameters
   current = ctrl.tes_get_current(channel=1)
   power = ctrl.tes_get_power(channel=1)
   shunt = ctrl.tes_get_shunt(channel=1)
   bus = ctrl.tes_get_bus(channel=1)
   bits = ctrl.tes_get_bits(channel=1)

Setting Current
~~~~~~~~~~~~~~~

.. code-block:: python

   # Set current in milliamps (0-20.0 mA)
   ctrl.tes_set_current(channel=1, current_mA=5.0)
   
   # Set DAC bits directly (0-0xFFFFF)
   ctrl.tes_set_bits(channel=1, value=0x80000)
   
   # Increment current
   ctrl.tes_inc_current(channel=1, delta=100)
   
   # Decrement current
   ctrl.tes_dec_current(channel=1, delta=50)

Enable/Disable
~~~~~~~~~~~~~~

.. code-block:: python

   # Enable single channel
   ctrl.tes_enable(channel=1)
   
   # Disable all channels
   ctrl.tes_disable()
   
   # Enable multiple channels
   ctrl.tes_enable(channel=[1, 2, 3])

LNA Operations
--------------

LNA operations always require a ``target`` parameter (``'GATE'`` or ``'DRAIN'``).

Getting Data
~~~~~~~~~~~~

.. code-block:: python

   # Get all parameters for GATE
   data = ctrl.lna_get_all(channel=1, target='GATE')
   
   # Get specific parameters
   current = ctrl.lna_get_current(channel=1, target='DRAIN')
   power = ctrl.lna_get_power(channel=1, target='GATE')

Setting DAC Values
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # Set DAC value (0-0xFFFF)
   ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)
   
   # Set all channels
   ctrl.lna_set_dac(target='GATE', value=0x8000)
   
   # Set different values on different channels
   ctrl.lna_set_dac(
       channel=[1, 2, 3],
       target='GATE',
       value=[0x4000, 0x6000, 0x8000]
   )

Physical units and gate polarity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The LNA driver has been extended to accept physical quantities in addition to raw DAC codes. You can pass either an integer DAC code (0-0xFFFF) or a numeric physical value (float) that represents a voltage or current; the driver performs the conversion to DAC units internally.

When specifying values for the ``GATE`` target, pass them as positive numbers (for example, ``1.2`` to indicate 1.2 V or 1.2 mA depending on the driver mode). The driver will map these positive inputs to the correct negative polarity required by the hardware. This means you always pass positive numbers in code even though the actual gate voltages/currents applied to the device will be negative.

Examples
~~~~~~~~

The following examples show the different ways to call ``lna_set_dac``:

.. code-block:: python

   # 1) Raw DAC code (integer)
   # Set GATE DAC on channel 1 to raw code 0x8000
   ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)

   # 2) Physical voltage (float)
   # Set GATE to 1.2 V (driver converts to DAC code and applies negative polarity)
   ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)

   # 3) Physical current (float) for multiple channels
   # Provide a list of currents for channels 1..6 (must match num_lna)
   currents_mA = [0.5, 1.0, 1.5, 2.0, 2.5, 3.0]
   ctrl.lna_set_dac(target='GATE', value=currents_mA)

   # 4) Specific channels with different physical values
   ctrl.lna_set_dac(channel=[1,3], target='GATE', value=[1.2, 0.8])

Notes:

- The driver will accept either integer DAC codes or numeric physical values (floats).
- For float values, the interpretation (volts vs milliamps) depends on the driver configuration/conversion; consult your hardware-specific settings.
- Always provide positive numbers for ``GATE`` values — the driver inverts polarity as needed when applying to the hardware.

LNA convenience setters
~~~~~~~~~~~~~~~~~~~~~~~

In addition to the raw `lna_set_dac(...)` interface, the high-level `DeviceController`
exposes convenience wrappers that accept physical units and forward to the per-channel
`LnaController` methods:

- ``lna_set_voltage(channel, target, voltage_V)`` — accepts a single float or list of floats
   in volts (0.0–5.0 V) and applies the corresponding DAC setting for the selected channel(s).
- ``lna_set_current(channel, target, current_mA)`` — accepts a single float or list of floats
   in milliamps (0.0–64.0 mA) and applies the corresponding DAC setting for the selected channel(s).

These convenience methods are implemented on `DeviceController` and simply call the
bound `LnaController.set_voltage(...)` / `LnaController.set_current(...)` methods.

Developer note
--------------

While updating the docs I found a small driver bug: `LnaController.get_shunt` previously called
``self.channel._check_target(target)`` which is incorrect and will raise an attribute error; it
should call ``self._check_target(target)``. The codebase in this repository has been updated to
fix that call. This does not change the public API but is important for internal correctness.

Enable/Disable
~~~~~~~~~~~~~~

.. code-block:: python

   # Enable GATE on channel 1
   ctrl.lna_enable(channel=1, target='GATE')
   
   # Enable DRAIN on all channels
   ctrl.lna_enable(target='DRAIN')
   
   # Disable GATE on specific channels
   ctrl.lna_disable(channel=[1, 2, 3], target='GATE')

DAC Operations
--------------

The global DAC has no channel parameter:

.. code-block:: python

   # Set global DAC value
   ctrl.dac_set(value=0x8000)
   
   # Get global DAC value
   data = ctrl.dac_get()

Error Handling
--------------

Always wrap operations in try/except blocks:

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   with DeviceController.from_config('config.yaml') as ctrl:
       try:
           data = ctrl.tes_get_all(channel=1)
           print(f"Success: {data}")
       except CommandError as e:
           print(f"Error: {e}")
       except ValueError as e:
           print(f"Invalid parameter: {e}")

Direct Controller Access
-------------------------

For advanced use, you can access per-channel controllers directly:

.. code-block:: python

   # Get the TesController for channel 1
   tes1 = ctrl.get_tes_controller(channel=1)
   
   # Call methods directly (no channel parameter needed)
   data = tes1.get_all()
   tes1.set_current(current_mA=5.0)
   tes1.enable()

This is useful when you need to perform many operations on the same channel
without repeatedly specifying the channel number.

Return Types
------------

**Single Channel Operations:**
  Return a single ``Dict[str, Any]``

**Multiple Channel Operations:**
  Return a ``List[Dict[str, Any]]``

.. code-block:: python

   # Single dict
   data = ctrl.tes_get_all(channel=1)
   print(data['current_mA'])
   
   # List of dicts
   data_list = ctrl.tes_get_all()
   for i, data in enumerate(data_list, 1):
       print(f"Channel {i}: {data['current_mA']} mA")

Next Steps
----------

- See :doc:`examples` for complete working examples
- See :doc:`api` for detailed API reference
