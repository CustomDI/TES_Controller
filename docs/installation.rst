Installation
============

Requirements
------------

- Python 3.7+
- Serial port access for hardware communication

Install from Source
-------------------

1. Clone the repository:

.. code-block:: bash

   git clone https://github.com/pahorton/TES_Controller.git
   cd TES_Controller/python

2. Install the required dependencies:

.. code-block:: bash

   pip install -r requirements.txt

Dependencies
------------

The TES Controller requires the following Python packages:

- ``pyserial`` - For serial communication
- ``pyyaml`` - For configuration file parsing

These are automatically installed when you run ``pip install -r requirements.txt``.

Configuration
-------------

1. Copy the example configuration file:

.. code-block:: bash

   cp config_example.yaml config.yaml

2. Edit ``config.yaml`` with your serial port settings:

.. code-block:: yaml

   port: /dev/ttyACM0  # Your serial port
   baud: 115200
   timeout: 1.0
   num_tes: 6
   num_lna: 6

Serial Port Configuration
--------------------------

macOS
~~~~~

Serial ports are typically named:

- ``/dev/tty.usbmodem*`` - USB CDC devices
- ``/dev/tty.usbserial*`` - USB serial adapters

To find your device:

.. code-block:: bash

   ls /dev/tty.*

Linux
~~~~~

Serial ports are typically named:

- ``/dev/ttyACM0`` - USB CDC devices
- ``/dev/ttyUSB0`` - USB serial adapters

To find your device:

.. code-block:: bash

   ls /dev/tty*

You may need to add your user to the ``dialout`` group:

.. code-block:: bash

   sudo usermod -a -G dialout $USER

Windows
~~~~~~~

Serial ports are named ``COM1``, ``COM2``, etc.

Check Device Manager to find the correct COM port number.

Verifying Installation
-----------------------

Run the example script to verify your installation:

.. code-block:: bash

   python example.py

If successful, you should see output from your connected TES/LNA hardware.
