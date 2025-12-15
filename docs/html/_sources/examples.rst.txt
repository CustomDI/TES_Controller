Examples
========

The snippets below illustrate complete, runnable patterns built on top of the
high-level :class:`tes_controller.controller.DeviceController` API.

Basic Example
-------------

.. code-block:: python

   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   with DeviceController.from_config('python/config.yaml') as ctrl:
       try:
           reading = ctrl.tes_get_all(channel=1)
           print(f"Channel 1 state: {reading}")

           ctrl.tes_set_current(channel=1, current_mA=5.0)
           ctrl.tes_enable(channel=1)
       except CommandError as exc:
           print(f"Hardware command failed: {exc}")

Multi-Channel Operations
------------------------

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       all_data = ctrl.tes_get_all()
       for index, data in enumerate(all_data, start=1):
           print(f"Channel {index}: {data}")

       ctrl.tes_set_current(current_mA=5.0)

       ctrl.tes_set_current(
           current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
       )

       ctrl.tes_enable(channel=[1, 3, 5])

Current Sweep
-------------

.. code-block:: python

   import time
   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       channel = 1
       for target in [0.5, 1.0, 1.5, 2.0, 2.5]:
           ctrl.tes_set_current(channel=channel, current_mA=target)
           time.sleep(0.1)
           readback = ctrl.tes_get_current(channel=channel)
           print(f"Commanded {target:.2f} mA -> {readback.get('current_mA', 'N/A')} mA")

Batch Configuration
-------------------

.. code-block:: python

   from tes_controller import DeviceController

   configuration = {
       1: {"current_mA": 5.0, "enabled": True},
       2: {"current_mA": 3.5, "enabled": True},
       3: {"current_mA": 4.2, "enabled": False},
       4: {"current_mA": 6.1, "enabled": True},
       5: {"current_mA": 2.8, "enabled": True},
       6: {"current_mA": 5.5, "enabled": False},
   }

   with DeviceController.from_config('python/config.yaml') as ctrl:
       for channel, settings in configuration.items():
           ctrl.tes_set_current(channel=channel, current_mA=settings["current_mA"])
           if settings["enabled"]:
               ctrl.tes_enable(channel=channel)
           else:
               ctrl.tes_disable(channel=channel)

       for channel in configuration:
           print(f"Channel {channel}: {ctrl.tes_get_all(channel=channel)}")

LNA Configuration
-----------------

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       ctrl.lna_set_dac(target='GATE', value=[0x4000, 0x4500, 0x5000, 0x5500, 0x6000, 0x6500])
       ctrl.lna_set_dac(channel=[1, 2, 3], target='DRAIN', value=0x8000)

       ctrl.lna_enable(target='GATE')
       ctrl.lna_enable(channel=[1, 2, 3], target='DRAIN')

       for channel in range(1, 7):
           gate = ctrl.lna_get_all(channel=channel, target='GATE')
           drain = ctrl.lna_get_all(channel=channel, target='DRAIN')
           print(f"Channel {channel}: gate={gate} drain={drain}")

       ctrl.lna_set_voltage(channel=1, target='GATE', voltage_V=1.2)
       ctrl.lna_set_current(target='GATE', current_mA=[0.5, 1.0, 1.5, 2.0, 2.5, 3.0])

Flux Ramp DAC
-------------

.. code-block:: python

   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       ctrl.flux_ramp_set(0x8000)
       print(ctrl.flux_ramp_get())

Data Acquisition
----------------

.. code-block:: python

   import csv
   import time
   from tes_controller import DeviceController

   with DeviceController.from_config('python/config.yaml') as ctrl:
       ctrl.tes_enable()
       ctrl.tes_set_current(current_mA=5.0)

       sample_rate_hz = 10
       duration_sec = 5
       interval = 1.0 / sample_rate_hz

       with open('tes_data.csv', 'w', newline='') as handle:
           writer = csv.writer(handle)
           header = ['time_s']
           header.extend([f'ch{idx}_current_mA' for idx in range(1, ctrl.num_tes + 1)])
           header.extend([f'ch{idx}_power_mW' for idx in range(1, ctrl.num_tes + 1)])
           writer.writerow(header)

           start = time.time()
           while time.time() - start < duration_sec:
               elapsed = time.time() - start
               currents = ctrl.tes_get_current()
               powers = ctrl.tes_get_power()
               row = [elapsed]
               row.extend([entry.get('current_mA', 0.0) for entry in currents])
               row.extend([entry.get('power_mW', 0.0) for entry in powers])
               writer.writerow(row)
               time.sleep(interval)

Robust Error Handling
---------------------

.. code-block:: python

   import time
   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   def safe_set_current(ctrl, channel, current_mA, attempts=3):
       for attempt in range(1, attempts + 1):
           try:
               ctrl.tes_set_current(channel=channel, current_mA=current_mA)
               time.sleep(0.1)
               actual = ctrl.tes_get_current(channel=channel)
               print(f"Channel {channel}: target {current_mA} mA -> actual {actual.get('current_mA', 'N/A')} mA")
               return True
           except CommandError as exc:
               print(f"Attempt {attempt}/{attempts} failed: {exc}")
       return False

   with DeviceController.from_config('python/config.yaml') as ctrl:
       for channel in range(1, ctrl.num_tes + 1):
           safe_set_current(ctrl, channel, current_mA=5.0)

Complete Workflow Script
------------------------

.. code-block:: python

   import argparse
   import time
   from tes_controller import DeviceController
   from tes_controller.drivers import CommandError

   def initialize(ctrl):
       ctrl.tes_disable()
       ctrl.tes_set_current(current_mA=0.0)
       ctrl.lna_disable(target='GATE')
       ctrl.lna_disable(target='DRAIN')
       ctrl.lna_set_dac(target='GATE', value=0x0000)
       ctrl.lna_set_dac(target='DRAIN', value=0x0000)

   def run_profile(ctrl, profile):
       ctrl.tes_set_current(current_mA=profile['tes_current'])
       ctrl.tes_enable(channel=profile['tes_channels'])
       ctrl.lna_set_dac(target='GATE', value=profile['lna_gate'])
       ctrl.lna_enable(target='GATE')
       time.sleep(profile.get('settling_time', 1.0))
       return ctrl.tes_get_all()

   def shutdown(ctrl):
       ctrl.tes_disable()
       ctrl.lna_disable(target='GATE')
       ctrl.lna_disable(target='DRAIN')

   def main():
       parser = argparse.ArgumentParser(description='TES Controller workflow')
       parser.add_argument('--config', default='python/config.yaml')
       args = parser.parse_args()

       try:
           with DeviceController.from_config(args.config) as ctrl:
               initialize(ctrl)
               profiles = [
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

               for index, profile in enumerate(profiles, start=1):
                   print(f"\n=== Profile {index}/{len(profiles)} ===")
                   result = run_profile(ctrl, profile)
                   print(result)

               shutdown(ctrl)
       except CommandError as exc:
           print(f"Hardware error: {exc}")
       except Exception as exc:
           print(f"Unexpected error: {exc}")

   if __name__ == '__main__':
       main()
