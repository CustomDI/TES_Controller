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
