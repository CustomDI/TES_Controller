Examples
========

This page contains complete, working examples of the TES Controller API.

Basic Example
-------------

Simple single-channel operations:

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   with DeviceController.from_config('config.yaml') as ctrl:
       try:
           # Get channel 1 data
           data = ctrl.tes_get_all(channel=1)
           print(f"Channel 1: {data}")
           
           # Set current
           ctrl.tes_set_current(channel=1, current_mA=5.0)
           
           # Enable the channel
           ctrl.tes_enable(channel=1)
           
       except CommandError as e:
           print(f"Error: {e}")

Multi-Channel Operations
------------------------

Working with multiple channels:

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('config.yaml') as ctrl:
       # Get data from all channels
       all_data = ctrl.tes_get_all()
       for i, data in enumerate(all_data, 1):
           print(f"Channel {i}: {data}")
       
       # Set same current on all channels
       ctrl.tes_set_current(current_mA=5.0)
       
       # Set different currents on each channel
       currents = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
       ctrl.tes_set_current(current_mA=currents)
       
       # Enable specific channels
       ctrl.tes_enable(channel=[1, 3, 5])

Current Sweep
-------------

Sweeping current on a single channel:

.. code-block:: python

   from tes_controller import DeviceController
   import time

   with DeviceController.from_config('config.yaml') as ctrl:
       channel = 1
       start_mA = 0.0
       stop_mA = 10.0
       step_mA = 0.5
       
       # Enable channel
       ctrl.tes_enable(channel=channel)
       
       # Set starting current
       ctrl.tes_set_current(channel=channel, current_mA=start_mA)
       time.sleep(0.1)
       
       # Sweep current
       current = start_mA
       while current <= stop_mA:
           # Set current
           ctrl.tes_set_current(channel=channel, current_mA=current)
           time.sleep(0.05)
           
           # Read back measurements
           data = ctrl.tes_get_current(channel=channel)
           power = ctrl.tes_get_power(channel=channel)
           
           print(f"Current: {current:.2f} mA, "
                 f"Measured: {data.get('current_mA', 'N/A')} mA, "
                 f"Power: {power.get('power_mW', 'N/A')} mW")
           
           current += step_mA
       
       # Disable channel
       ctrl.tes_disable(channel=channel)

Batch Configuration
-------------------

Configure multiple channels with different settings:

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('config.yaml') as ctrl:
       # Configuration for each channel
       config = {
           1: {'current_mA': 5.0, 'enabled': True},
           2: {'current_mA': 3.5, 'enabled': True},
           3: {'current_mA': 4.2, 'enabled': False},
           4: {'current_mA': 6.1, 'enabled': True},
           5: {'current_mA': 2.8, 'enabled': True},
           6: {'current_mA': 5.5, 'enabled': False},
       }
       
       # Apply configuration
       for channel, settings in config.items():
           # Set current
           ctrl.tes_set_current(
               channel=channel,
               current_mA=settings['current_mA']
           )
           
           # Enable or disable
           if settings['enabled']:
               ctrl.tes_enable(channel=channel)
           else:
               ctrl.tes_disable(channel=channel)
       
       # Verify configuration
       print("\nVerifying configuration:")
       for channel in config.keys():
           data = ctrl.tes_get_all(channel=channel)
           print(f"Channel {channel}: {data}")

LNA Configuration
-----------------

Configure LNA channels:

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('config.yaml') as ctrl:
       # Set GATE voltages on all channels
       gate_values = [0x4000, 0x4500, 0x5000, 0x5500, 0x6000, 0x6500]
       ctrl.lna_set_dac(target='GATE', value=gate_values)
       
       # Set DRAIN voltage on specific channels
       ctrl.lna_set_dac(
           channel=[1, 2, 3],
           target='DRAIN',
           value=0x8000
       )
       
       # Enable all GATE channels
       ctrl.lna_enable(target='GATE')
       
       # Enable DRAIN on specific channels
       ctrl.lna_enable(channel=[1, 2, 3], target='DRAIN')
       
       # Read back configuration
       print("\nLNA Configuration:")
       for i in range(1, 7):
           gate_data = ctrl.lna_get_all(channel=i, target='GATE')
           drain_data = ctrl.lna_get_all(channel=i, target='DRAIN')
           print(f"Channel {i}:")
           print(f"  GATE: {gate_data}")
           print(f"  DRAIN: {drain_data}")

Data Acquisition
----------------

Continuous data acquisition from all channels:

.. code-block:: python

   from tes_controller import DeviceController
   import time
   import csv

   with DeviceController.from_config('config.yaml') as ctrl:
       # Setup
       ctrl.tes_enable()  # Enable all channels
       ctrl.tes_set_current(current_mA=5.0)  # Set all to 5 mA
       
       # Data acquisition
       duration_sec = 10
       sample_rate_hz = 10
       interval = 1.0 / sample_rate_hz
       
       with open('data.csv', 'w', newline='') as f:
           writer = csv.writer(f)
           # Write header
           header = ['time'] + [f'ch{i}_current' for i in range(1, 7)] + \
                    [f'ch{i}_power' for i in range(1, 7)]
           writer.writerow(header)
           
           # Acquire data
           start_time = time.time()
           while time.time() - start_time < duration_sec:
               t = time.time() - start_time
               
               # Read all channels
               currents = ctrl.tes_get_current()
               powers = ctrl.tes_get_power()
               
               # Extract values
               row = [t]
               row.extend([c.get('current_mA', 0) for c in currents])
               row.extend([p.get('power_mW', 0) for p in powers])
               
               writer.writerow(row)
               
               # Wait for next sample
               time.sleep(interval)
       
       print(f"Data saved to data.csv")

Error Recovery
--------------

Robust error handling and recovery:

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError
   import time

   def safe_operation(ctrl, channel, target_mA, max_retries=3):
       """Safely set current with retries."""
       for attempt in range(max_retries):
           try:
               ctrl.tes_set_current(channel=channel, current_mA=target_mA)
               
               # Verify
               time.sleep(0.1)
               data = ctrl.tes_get_current(channel=channel)
               measured = data.get('current_mA', 0)
               
               if abs(measured - target_mA) < 0.1:
                   print(f"Channel {channel}: Successfully set to {target_mA} mA")
                   return True
               else:
                   print(f"Channel {channel}: Verification failed "
                         f"(target={target_mA}, measured={measured})")
                   
           except CommandError as e:
               print(f"Channel {channel}: Attempt {attempt + 1} failed: {e}")
               if attempt < max_retries - 1:
                   time.sleep(0.5)
       
       print(f"Channel {channel}: Failed after {max_retries} attempts")
       return False

   with DeviceController.from_config('config.yaml') as ctrl:
       # Try to set currents with error recovery
       for channel in range(1, 7):
           safe_operation(ctrl, channel, target_mA=5.0)

Using Direct Controller Access
-------------------------------

For intensive operations on a single channel:

.. code-block:: python

   from tes_controller import DeviceController
   import time

   with DeviceController.from_config('config.yaml') as ctrl:
       # Get direct access to channel 1 controller
       tes1 = ctrl.get_tes_controller(channel=1)
       
       # No need to specify channel repeatedly
       tes1.enable()
       tes1.set_current(current_mA=1.0)
       
       # Incremental adjustment
       for i in range(100):
           tes1.inc_current(delta=10)
           time.sleep(0.01)
           
           if i % 10 == 0:
               data = tes1.get_current()
               print(f"Step {i}: {data}")
       
       tes1.disable()

Complete Application Example
-----------------------------

A complete application with initialization, operation, and cleanup:

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError
   import time
   import argparse

   def initialize_system(ctrl):
       """Initialize all channels to safe state."""
       print("Initializing system...")
       
       # Disable all channels
       ctrl.tes_disable()
       ctrl.lna_disable(target='GATE')
       ctrl.lna_disable(target='DRAIN')
       
       # Set safe defaults
       ctrl.tes_set_current(current_mA=0.0)
       ctrl.lna_set_dac(target='GATE', value=0x0000)
       ctrl.lna_set_dac(target='DRAIN', value=0x0000)
       
       print("System initialized")

   def run_measurement(ctrl, config):
       """Run measurement with given configuration."""
       print(f"Running measurement with config: {config}")
       
       # Apply TES configuration
       ctrl.tes_set_current(current_mA=config['tes_current'])
       ctrl.tes_enable(channel=config['tes_channels'])
       
       # Apply LNA configuration
       ctrl.lna_set_dac(target='GATE', value=config['lna_gate'])
       ctrl.lna_enable(target='GATE')
       
       # Wait for settling
       time.sleep(config.get('settling_time', 1.0))
       
       # Acquire data
       data = ctrl.tes_get_all()
       return data

   def shutdown_system(ctrl):
       """Safely shutdown the system."""
       print("Shutting down system...")
       ctrl.tes_disable()
       ctrl.lna_disable(target='GATE')
       ctrl.lna_disable(target='DRAIN')
       print("System shutdown complete")

   def main():
       parser = argparse.ArgumentParser(description='TES Controller Application')
       parser.add_argument('--config', default='config.yaml',
                          help='Configuration file path')
       args = parser.parse_args()
       
       try:
           with DeviceController.from_config(args.config) as ctrl:
               # Initialize
               initialize_system(ctrl)
               
               # Run measurements
               configs = [
                   {
                       'tes_current': 5.0,
                       'tes_channels': [1, 2, 3],
                       'lna_gate': 0x8000,
                       'settling_time': 1.0,
                   },
                   {
                       'tes_current': 7.5,
                       'tes_channels': [4, 5, 6],
                       'lna_gate': 0xA000,
                       'settling_time': 1.0,
                   },
               ]
               
               for i, config in enumerate(configs, 1):
                   print(f"\n=== Measurement {i}/{len(configs)} ===")
                   data = run_measurement(ctrl, config)
                   print(f"Results: {data}")
               
               # Shutdown
               shutdown_system(ctrl)
               
       except CommandError as e:
           print(f"Command error: {e}")
       except Exception as e:
           print(f"Unexpected error: {e}")

   if __name__ == '__main__':
       main()
