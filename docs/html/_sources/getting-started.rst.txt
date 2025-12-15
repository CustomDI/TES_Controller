Getting Started
===============

This guide walks through core workflows using the TES Controller Python client.

Creating a Controller
---------------------

There are two ways to build a :class:`~tes_controller.controller.DeviceController`.

From a configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   from tes_controller import DeviceController

   ctrl = DeviceController.from_config('python/config.yaml')

Direct instantiation
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   from tes_controller import DeviceController

   ctrl = DeviceController(
       port='/dev/ttyACM0',
       baud=115200,
       timeout=1.0,
       num_tes=6,
       num_lna=6,
   )

Using Context Managers
----------------------

The controller manages a serial connection. Using a ``with`` block ensures ports
close even if an exception occurs:

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       data = ctrl.tes_get_all(channel=1)
       print(data)

Channel Selection Patterns
--------------------------

Most high-level helpers accept flexible channel selectors.

Single channel
~~~~~~~~~~~~~~

.. code-block:: python

   ctrl.tes_get_all(channel=1)
   ctrl.tes_set_current(channel=1, current_mA=5.0)
   ctrl.tes_enable(channel=1)

Multiple specific channels
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   ctrl.tes_get_all(channel=[1, 3, 5])
   ctrl.tes_set_current(channel=[1, 3, 5], current_mA=5.0)
   ctrl.tes_enable(channel=[1, 3, 5])

All channels
~~~~~~~~~~~~

.. code-block:: python

   ctrl.tes_get_all()
   ctrl.tes_set_current(current_mA=5.0)
   ctrl.tes_enable()

Per-channel values
~~~~~~~~~~~~~~~~~~

Provide a list whose length matches ``num_tes`` (or ``num_lna`` for LNA helpers).

.. code-block:: python

   ctrl.tes_set_current(current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
   ctrl.tes_set_current(channel=[1, 3, 5], current_mA=[1.0, 2.0, 3.0])

TES Operations
--------------

Getters
~~~~~~~

.. code-block:: python

   ctrl.tes_get_all(channel=1)
   ctrl.tes_get_current(channel=1)
   ctrl.tes_get_power(channel=1)
   ctrl.tes_get_shunt(channel=1)
   ctrl.tes_get_bus(channel=1)
   ctrl.tes_get_bits(channel=1)

Setters
~~~~~~~

.. code-block:: python

   ctrl.tes_set_current(channel=1, current_mA=5.0)
   ctrl.tes_set_bits(channel=1, value=0x80000)
   ctrl.tes_inc_current(channel=1, delta=100)
   ctrl.tes_dec_current(channel=1, delta=50)

Enable/Disable
~~~~~~~~~~~~~~

.. code-block:: python

   ctrl.tes_enable(channel=1)
   ctrl.tes_disable(channel=1)
   ctrl.tes_enable()
   ctrl.tes_disable()

LNA Operations
--------------

Target selection
~~~~~~~~~~~~~~~~

Pass ``target='GATE'`` or ``target='DRAIN'`` with every LNA call.

.. code-block:: python

   ctrl.lna_get_all(channel=1, target='GATE')
   ctrl.lna_get_current(channel=1, target='DRAIN')

Setting DAC values
~~~~~~~~~~~~~~~~~~

``lna_set_dac`` accepts raw DAC codes (``0`` – ``0xFFFF``) or physical values. When
you provide a float, the driver performs the conversion and applies the proper
hardware polarity, so pass positive numbers even for gate voltages.

.. code-block:: python

   ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)
   ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)  # volts/mA
   ctrl.lna_set_dac(target='GATE', value=[1.0, 1.1, 1.2, 1.3, 1.4, 1.5])

Convenience setters
~~~~~~~~~~~~~~~~~~~

The high-level controller forwards convenience wrappers to per-channel drivers.

.. code-block:: python

   ctrl.lna_set_voltage(channel=1, target='GATE', voltage_V=1.2)
   ctrl.lna_set_current(target='GATE', current_mA=[0.5, 1.0, 1.5, 2.0, 2.5, 3.0])

DAC Operations
--------------

The global flux ramp DAC does not use channel selectors.

.. code-block:: python

   ctrl.flux_ramp_set(0x8000)
   ctrl.flux_ramp_get()

Error Handling
--------------

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   with DeviceController.from_config('python/config.yaml') as ctrl:
       try:
           data = ctrl.tes_get_all(channel=1)
       except CommandError as exc:
           print(f"Hardware command failed: {exc}")
       except ValueError as exc:
           print(f"Invalid parameter: {exc}")

Direct Controller Access
------------------------

For repeated operations against a single channel, keep a reference to the
underlying driver instance.

.. code-block:: python

   tes1 = ctrl.get_tes_controller(channel=1)
   tes1.enable()
   tes1.set_current(current_mA=5.0)
   data = tes1.get_all()

Return Types
------------

Single-channel helpers return a single ``dict``. Multi-channel helpers return a
list of ``dict`` objects, one per channel.

Next Steps
----------

- Continue to :doc:`usage` for workflow recipes
- Explore :doc:`examples` for complete scripts
- Read :doc:`api` for full reference material
