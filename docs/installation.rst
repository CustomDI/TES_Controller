Installation
============

Requirements
------------

- Python 3.8 or newer
- Serial port access for hardware communication

Install from Source
-------------------

1. Clone the repository:

.. code-block:: bash

   git clone https://github.com/CustomDI/TES_Controller.git
   cd TES_Controller

2. Install the runtime dependencies:

.. code-block:: bash

   pip install -r python/requirements.txt

3. (Optional) Install the documentation toolchain:

.. code-block:: bash

   pip install -r docs/requirements.txt

Configuration
-------------

1. Copy the example configuration file:

.. code-block:: bash

   cp python/config_example.yaml python/config.yaml

2. Edit ``python/config.yaml`` with your serial port settings:

.. code-block:: yaml

   port: /dev/ttyACM0  # Your serial port
   baud: 115200
   timeout: 1.0
   num_tes: 6
   num_lna: 6

Serial Port Notes
-----------------

macOS
~~~~~

Serial ports typically appear as ``/dev/tty.usbmodem*`` or ``/dev/tty.usbserial*``. To list devices:

.. code-block:: bash

   ls /dev/tty.*

Linux
~~~~~

Common serial devices are ``/dev/ttyACM0`` and ``/dev/ttyUSB0``. To list them:

.. code-block:: bash

   ls /dev/ttyACM* /dev/ttyUSB*

If required, add your user to the ``dialout`` group and log out/in:

.. code-block:: bash

   sudo usermod -a -G dialout "$USER"

Windows
~~~~~~~

Serial ports are named ``COM1``, ``COM2`` and so on. Check the Device Manager to find the active COM port.

Verifying Installation
----------------------

Run the bundled example to confirm connectivity:

.. code-block:: bash

   cd python
   python example.py

If the port is configured correctly, you should see YAML responses echoed to the console.
