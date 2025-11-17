"""Example usage of the tes_controller Python client.

Edit `python/config_example.yaml` or create `python/config.yaml` and point to your serial port.

This script demonstrates:
- loading the DeviceController from config
- using the new overloaded API for single/multiple/all channels
- reading TES parameters (single channel, multiple channels, all channels)
- setting TES output current with various patterns
- reading LNA parameters
- enable/disable operations on multiple channels

Run:
    python python/example.py

Before running, update `python/config_example.yaml` or create `python/config.yaml` with your device port.
"""

from tes_controller import DeviceController
from tes_controller.drivers import CommandError
import time

CONFIG_PATH = 'python/config_example.yaml'


def separator(title):
    """Print a nice separator for examples."""
    print(f"\n{'='*60}")
    print(f"  {title}")
    print('='*60)


def main():
    # Create controller from config. Use context manager to ensure clean close.
    with DeviceController.from_config(CONFIG_PATH) as ctrl:
        
        # ============================================================
        # EXAMPLE 1: Single Channel Operations
        # ============================================================
        separator("Example 1: Single Channel Operations")
        
        try:
            print('Getting TES channel 1 data...')
            tes_resp = ctrl.tes_get_all(channel=1)
            print(f'TES 1: {tes_resp}')
            
            print('\nSetting TES channel 1 current to 1.0 mA...')
            set_resp = ctrl.tes_set_current(channel=1, current_mA=1.0)
            print(f'Response: {set_resp}')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 2: Multiple Specific Channels
        # ============================================================
        separator("Example 2: Multiple Specific Channels")
        
        try:
            print('Getting data from TES channels 1, 3, and 5...')
            multi_resp = ctrl.tes_get_all(channel=[1, 3, 5])
            for i, resp in enumerate(multi_resp, 1):
                print(f'  Channel {[1,3,5][i-1]}: {resp}')
            
            print('\nSetting channels 1, 3, 5 to same current (2.0 mA)...')
            set_resp = ctrl.tes_set_current(channel=[1, 3, 5], current_mA=2.0)
            print(f'  Set {len(set_resp)} channels successfully')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 3: All Channels Operations
        # ============================================================
        separator("Example 3: All Channels Operations")
        
        try:
            print('Getting data from ALL TES channels...')
            all_resp = ctrl.tes_get_all()  # No channel parameter = all channels
            for i, resp in enumerate(all_resp, 1):
                print(f'  Channel {i}: {resp}')
            
            print('\nSetting ALL channels to same current (1.5 mA)...')
            set_resp = ctrl.tes_set_current(current_mA=1.5)
            print(f'  Set {len(set_resp)} channels successfully')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 4: Array of Different Values
        # ============================================================
        separator("Example 4: Setting Different Values Per Channel")
        
        try:
            print('Setting each TES channel to a different current...')
            # Assuming 6 TES channels
            currents = [0.5, 1.0, 1.5, 2.0, 2.5, 3.0]
            set_resp = ctrl.tes_set_current(current_mA=currents)
            print(f'  Set {len(set_resp)} channels to different values')
            
            time.sleep(0.2)
            
            print('\nReading back currents from all channels...')
            curr_resp = ctrl.tes_get_current()
            for i, resp in enumerate(curr_resp, 1):
                print(f'  Channel {i}: {resp.get("current_mA", "N/A")} mA')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 5: Selective Channels with Different Values
        # ============================================================
        separator("Example 5: Selective Channels with Different Values")
        
        try:
            print('Setting channels 2, 4, 6 to different currents...')
            set_resp = ctrl.tes_set_current(
                channel=[2, 4, 6],
                current_mA=[1.2, 2.4, 3.6]
            )
            print(f'  Set {len(set_resp)} channels')
            
            print('\nReading back just those channels...')
            curr_resp = ctrl.tes_get_current(channel=[2, 4, 6])
            for ch, resp in zip([2, 4, 6], curr_resp):
                print(f'  Channel {ch}: {resp.get("current_mA", "N/A")} mA')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 6: Enable/Disable Multiple Channels
        # ============================================================
        separator("Example 6: Enable/Disable Operations")
        
        try:
            print('Disabling all TES channels...')
            disable_resp = ctrl.tes_disable()  # No channel = all channels
            print(f'  Disabled {len(disable_resp)} channels')
            
            time.sleep(0.2)
            
            print('\nEnabling channels 1, 2, 3...')
            enable_resp = ctrl.tes_enable(channel=[1, 2, 3])
            print(f'  Enabled {len(enable_resp)} channels')
            
            time.sleep(0.2)
            
            print('\nEnabling single channel 4...')
            enable_resp = ctrl.tes_enable(channel=4)
            print(f'  Response: {enable_resp}')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 7: Increment/Decrement Operations
        # ============================================================
        separator("Example 7: Increment/Decrement Current")
        
        try:
            print('Incrementing channel 1 current by 100 bits...')
            inc_resp = ctrl.tes_inc_current(channel=1, delta=100)
            print(f'  Response: {inc_resp}')
            
            print('\nDecrementing channels 1, 2, 3 by 50 bits each...')
            dec_resp = ctrl.tes_dec_current(channel=[1, 2, 3], delta=50)
            print(f'  Modified {len(dec_resp)} channels')
            
            print('\nIncrementing all channels by different amounts...')
            deltas = [10, 20, 30, 40, 50, 60]
            inc_resp = ctrl.tes_inc_current(delta=deltas)
            print(f'  Modified {len(inc_resp)} channels')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 8: LNA Operations (with target parameter)
        # ============================================================
        separator("Example 8: LNA Operations")
        
        try:
            print('Getting LNA channel 1 DRAIN data...')
            lna_resp = ctrl.lna_get_all(channel=1, target='DRAIN')
            print(f'  LNA 1 DRAIN: {lna_resp}')
            
            print('\nGetting GATE data from all LNA channels...')
            lna_all = ctrl.lna_get_all(target='GATE')
            for i, resp in enumerate(lna_all, 1):
                print(f'  LNA {i} GATE: {resp}')
            
            print('\nSetting LNA channels 1, 2 GATE to different DAC values...')
            set_resp = ctrl.lna_set_dac(
                channel=[1, 2],
                target='GATE',
                value=[0x4000, 0x6000]
            )
            print(f'  Set {len(set_resp)} channels')
            
            print('\nEnabling DRAIN on all LNA channels...')
            enable_resp = ctrl.lna_enable(target='DRAIN')
            print(f'  Enabled {len(enable_resp)} channels')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 9: DAC Operations (global, no channel)
        # ============================================================
        separator("Example 9: Global DAC Operations")
        
        try:
            print('Setting global DAC to 0x8000...')
            dac_resp = ctrl.dac_set(value=0x8000)
            print(f'  Response: {dac_resp}')
            
            print('\nReading global DAC value...')
            dac_val = ctrl.dac_get()
            print(f'  DAC value: {dac_val}')
            
        except CommandError as e:
            print(f'Error: {e}')

        # ============================================================
        # EXAMPLE 10: Getting Other TES Parameters
        # ============================================================
        separator("Example 10: Reading Other TES Parameters")
        
        try:
            print('Getting shunt voltage from all channels...')
            shunt_resp = ctrl.tes_get_shunt()
            for i, resp in enumerate(shunt_resp, 1):
                print(f'  Channel {i} shunt: {resp}')
            
            print('\nGetting power from channels 1 and 3...')
            power_resp = ctrl.tes_get_power(channel=[1, 3])
            for ch, resp in zip([1, 3], power_resp):
                print(f'  Channel {ch} power: {resp}')
            
            print('\nGetting bus voltage from channel 1...')
            bus_resp = ctrl.tes_get_bus(channel=1)
            print(f'  Channel 1 bus: {bus_resp}')
            
        except CommandError as e:
            print(f'Error: {e}')

        separator("Examples Complete!")
        print("\nAll examples executed. Check the output above for results.")
        print("Note: Some commands may fail if your hardware doesn't support them")
        print("or if channels are out of range for your configuration.\n")


if __name__ == '__main__':
    main()
