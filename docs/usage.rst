Usage Guide
===========

Installation
------------

Install the library and its dependencies into your environment:

.. code-block:: bash

   pip install -r python/requirements.txt

Quick Start
-----------

Create a controller from a YAML configuration file:

.. code-block:: python

   from tes_controller import DeviceController

    with DeviceController.from_config("python/config.yaml") as ctrl:
       ctrl.tes_set_current(channel=1, current_mA=5.0)
       reading = ctrl.tes_get_all(channel=1)
       print(reading)

Construct the controller manually when you want to override serial settings:

.. code-block:: python

   from tes_controller import DeviceController

   ctrl = DeviceController(
       port="/dev/ttyACM0",
       baud=115200,
       timeout=1.0,
       num_tes=6,
       num_lna=6,
   )

   try:
       ctrl.tes_enable(channel=[1, 2, 3])
       ctrl.tes_set_current(channel=[1, 2, 3], current_mA=[5.0, 4.5, 4.0])
   finally:
       ctrl.close()

Configuration File
------------------

The helper :meth:`tes_controller.controller.DeviceController.from_config`
method reads a YAML file with the following keys:

.. code-block:: yaml

   port: /dev/ttyACM0
   baud: 115200
   timeout: 1.0
   num_tes: 6
   num_lna: 6

Only the values you need to override are required. Missing values fall back to
sensible defaults.

Channel Selection Patterns
--------------------------

The high-level helpers accept flexible selectors so you can address a single
channel, an explicit subset, or the entire bank with the same call signature.

=============================== =============================================================== ==================
Call                             Effect                                                            Return Type
=============================== =============================================================== ==================
``ctrl.tes_get_all(channel=1)``  Query a single TES channel                                         ``dict``
``ctrl.tes_get_all()``           Query every TES channel                                            ``list[dict]``
``ctrl.tes_set_current(5.0)``    Apply the same current to every TES channel                        ``list[dict]``
``ctrl.tes_set_current(channel=[1, 3], current_mA=[1.0, 2.0])``  Pair channel list with value list  ``list[dict]``
=============================== =============================================================== ==================

See :doc:`getting-started` for a deeper walk-through of these patterns.

Frequently Used Helpers
-----------------------

TES control
~~~~~~~~~~~

.. code-block:: python

    ctrl.tes_get_current(channel=1)
    ctrl.tes_set_current(channel=1, current_mA=5.0)
    ctrl.tes_inc_current(channel=1, delta=100)
    ctrl.tes_enable(channel=[1, 2, 3])
    ctrl.tes_disable()

LNA control
~~~~~~~~~~~

Always include ``target='GATE'`` or ``target='DRAIN'``.

.. code-block:: python

    ctrl.lna_set_dac(channel=1, target='GATE', value=0x8000)
    ctrl.lna_set_dac(channel=1, target='GATE', value=1.2)  # physical units
    ctrl.lna_set_voltage(channel=1, target='GATE', voltage_V=1.2)
    ctrl.lna_enable(target='GATE')

Global DAC
~~~~~~~~~~

.. code-block:: python

    ctrl.flux_ramp_set(0x8000)
    ctrl.flux_ramp_get()

More comprehensive scripts are available in :doc:`examples`.
