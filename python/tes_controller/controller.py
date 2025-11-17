import yaml
from typing import Optional, Dict, Any, Union, List
from .serial_client import SerialClient
from .drivers import TesController, LnaController, DacController

class DeviceController:
    """High-level controller that encapsulates SerialClient, TesController and LnaController.

    Usage:
        # from file
        ctrl = DeviceController.from_config('path/to/config.yaml')

        # or construct directly
        ctrl = DeviceController(port='/dev/ttyACM0', baud=115200, timeout=1.0, num_tes=6, num_lna=6)

    The DeviceController exposes:
      - client: SerialClient
      - tes: TesController
      - lna: LnaController
      - num_tes, num_lna

    Note: the underlying drivers are thin wrappers that send commands and parse YAML responses.
    """

    def __init__(self, port: str = '/dev/ttyACM0', baud: int = 115200, timeout: float = 1.0,
                 num_tes: int = 6, num_lna: int = 6, auto_open: bool = True):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.num_tes = num_tes
        self.num_lna = num_lna

        self.client = SerialClient(self.port, baud=self.baud, timeout=self.timeout)
        if auto_open:
            self.client.open()

        # Create per-channel controller instances bound to their channel numbers.
        # tes and lna are lists indexed by (channel-1).
        self.tes: List[TesController] = [TesController(self.client, i + 1) for i in range(self.num_tes)]
        self.lna: List[LnaController] = [LnaController(self.client, i + 1) for i in range(self.num_lna)]
        
        # DAC controller (single instance, no channel binding)
        self.dac = DacController(self.client)

    @classmethod
    def from_config(cls, path: str, auto_open: bool = True) -> 'DeviceController':
        """Load controller config from a YAML file.

        Expected keys: port, baud, timeout, num_tes, num_lna
        """
        with open(path, 'r', encoding='utf-8') as f:
            cfg = yaml.safe_load(f) or {}
        port = cfg.get('port', '/dev/ttyACM0')
        baud = cfg.get('baud', 115200)
        timeout = cfg.get('timeout', 1.0)
        num_tes = cfg.get('num_tes', 6)
        num_lna = cfg.get('num_lna', 6)
        return cls(port=port, baud=baud, timeout=timeout, num_tes=num_tes, num_lna=num_lna, auto_open=auto_open)

    def close(self):
        try:
            self.client.close()
        except Exception:
            pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()

    # ========== DAC Convenience Methods ==========
    def dac_set(self, value: int) -> Dict[str, Any]:
        """Set DAC value (0-0xFFFF)."""
        return self.dac.set_dac(value)

    def dac_get(self) -> Dict[str, Any]:
        """Get current DAC value."""
        return self.dac.get_dac()

    # ========== TES Convenience Methods ==========
    def tes_get_all(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get all TES channel data.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_all() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_all() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_all()

    def tes_enable(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Enable TES channel(s).
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].enable() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].enable() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].enable()

    def tes_disable(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Disable TES channel(s).
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].disable() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].disable() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].disable()

    def tes_set_current(self, 
                       channel: Union[int, List[int], None] = None,
                       current_mA: Union[float, List[float], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Set TES current in mA (0-20.0).
        
        Args:
            channel: int for single, list for multiple, None for all channels
            current_mA: float for single value, list for multiple values
            
        Examples:
            tes_set_current(1, 5.0)  # Set channel 1 to 5.0 mA
            tes_set_current(current_mA=5.0)  # Set all channels to 5.0 mA
            tes_set_current(current_mA=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])  # Set all channels to different values
            tes_set_current([1, 3, 5], 5.0)  # Set channels 1, 3, 5 to 5.0 mA
            tes_set_current([1, 3, 5], [1.0, 2.0, 3.0])  # Set channels to different values
            
        Returns:
            Single dict if setting one channel, list of dicts otherwise
        """
        if current_mA is None:
            raise ValueError("current_mA must be provided")
            
        # Case 1: Single channel, single value
        if isinstance(channel, int) and not isinstance(current_mA, list):
            self._check_tes_channel(channel)
            return self.tes[channel - 1].set_current(current_mA)
        
        # Case 2: All channels, single value
        if channel is None and not isinstance(current_mA, list):
            return [self.tes[i].set_current(current_mA) for i in range(self.num_tes)]
        
        # Case 3: All channels, array of values
        if channel is None and isinstance(current_mA, list):
            if len(current_mA) != self.num_tes:
                raise ValueError(f"current_mA list length {len(current_mA)} must match num_tes {self.num_tes}")
            return [self.tes[i].set_current(current_mA[i]) for i in range(self.num_tes)]
        
        # Case 4: Array of channels, single value
        if isinstance(channel, list) and not isinstance(current_mA, list):
            return [self.tes[ch - 1].set_current(current_mA) for ch in channel]
        
        # Case 5: Array of channels, array of values
        if isinstance(channel, list) and isinstance(current_mA, list):
            if len(channel) != len(current_mA):
                raise ValueError(f"channel and current_mA lists must have same length")
            return [self.tes[ch - 1].set_current(curr) for ch, curr in zip(channel, current_mA)]
        
        raise ValueError("Invalid combination of channel and current_mA arguments")

    def tes_inc_current(self, 
                       channel: Union[int, List[int], None] = None,
                       delta: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Increment TES current by delta.
        
        Args:
            channel: int for single, list for multiple, None for all channels
            delta: int for single value, list for multiple values
            
        Returns:
            Single dict if setting one channel, list of dicts otherwise
        """
        if delta is None:
            raise ValueError("delta must be provided")
            
        if isinstance(channel, int) and not isinstance(delta, list):
            self._check_tes_channel(channel)
            return self.tes[channel - 1].inc_current(delta)
        
        if channel is None and not isinstance(delta, list):
            return [self.tes[i].inc_current(delta) for i in range(self.num_tes)]
        
        if channel is None and isinstance(delta, list):
            if len(delta) != self.num_tes:
                raise ValueError(f"delta list length {len(delta)} must match num_tes {self.num_tes}")
            return [self.tes[i].inc_current(delta[i]) for i in range(self.num_tes)]
        
        if isinstance(channel, list) and not isinstance(delta, list):
            return [self.tes[ch - 1].inc_current(delta) for ch in channel]
        
        if isinstance(channel, list) and isinstance(delta, list):
            if len(channel) != len(delta):
                raise ValueError(f"channel and delta lists must have same length")
            return [self.tes[ch - 1].inc_current(d) for ch, d in zip(channel, delta)]
        
        raise ValueError("Invalid combination of channel and delta arguments")

    def tes_dec_current(self, 
                       channel: Union[int, List[int], None] = None,
                       delta: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Decrement TES current by delta.
        
        Args:
            channel: int for single, list for multiple, None for all channels
            delta: int for single value, list for multiple values
            
        Returns:
            Single dict if setting one channel, list of dicts otherwise
        """
        if delta is None:
            raise ValueError("delta must be provided")
            
        if isinstance(channel, int) and not isinstance(delta, list):
            self._check_tes_channel(channel)
            return self.tes[channel - 1].dec_current(delta)
        
        if channel is None and not isinstance(delta, list):
            return [self.tes[i].dec_current(delta) for i in range(self.num_tes)]
        
        if channel is None and isinstance(delta, list):
            if len(delta) != self.num_tes:
                raise ValueError(f"delta list length {len(delta)} must match num_tes {self.num_tes}")
            return [self.tes[i].dec_current(delta[i]) for i in range(self.num_tes)]
        
        if isinstance(channel, list) and not isinstance(delta, list):
            return [self.tes[ch - 1].dec_current(delta) for ch in channel]
        
        if isinstance(channel, list) and isinstance(delta, list):
            if len(channel) != len(delta):
                raise ValueError(f"channel and delta lists must have same length")
            return [self.tes[ch - 1].dec_current(d) for ch, d in zip(channel, delta)]
        
        raise ValueError("Invalid combination of channel and delta arguments")

    def tes_set_bits(self, 
                    channel: Union[int, List[int], None] = None,
                    value: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Set TES DAC bits directly (0-0xFFFFF).
        
        Args:
            channel: int for single, list for multiple, None for all channels
            value: int for single value, list for multiple values
            
        Returns:
            Single dict if setting one channel, list of dicts otherwise
        """
        if value is None:
            raise ValueError("value must be provided")
            
        if isinstance(channel, int) and not isinstance(value, list):
            self._check_tes_channel(channel)
            return self.tes[channel - 1].set_bits(value)
        
        if channel is None and not isinstance(value, list):
            return [self.tes[i].set_bits(value) for i in range(self.num_tes)]
        
        if channel is None and isinstance(value, list):
            if len(value) != self.num_tes:
                raise ValueError(f"value list length {len(value)} must match num_tes {self.num_tes}")
            return [self.tes[i].set_bits(value[i]) for i in range(self.num_tes)]
        
        if isinstance(channel, list) and not isinstance(value, list):
            return [self.tes[ch - 1].set_bits(value) for ch in channel]
        
        if isinstance(channel, list) and isinstance(value, list):
            if len(channel) != len(value):
                raise ValueError(f"channel and value lists must have same length")
            return [self.tes[ch - 1].set_bits(v) for ch, v in zip(channel, value)]
        
        raise ValueError("Invalid combination of channel and value arguments")

    def tes_get_bits(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get TES DAC bits.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_bits() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_bits() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_bits()

    def tes_get_shunt(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get TES shunt voltage.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_shunt() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_shunt() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_shunt()

    def tes_get_bus(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get TES bus voltage.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_bus() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_bus() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_bus()

    def tes_get_current(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get TES current.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_current() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_current() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_current()

    def tes_get_power(self, channel: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get TES power.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if channel is None:
            return [self.tes[i].get_power() for i in range(self.num_tes)]
        elif isinstance(channel, list):
            return [self.tes[ch - 1].get_power() for ch in channel]
        else:
            self._check_tes_channel(channel)
            return self.tes[channel - 1].get_power()

    # ========== LNA Convenience Methods ==========
    def lna_get_all(self, 
                   channel: Union[int, List[int], None] = None,
                   target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get all LNA data for target (GATE or DRAIN).
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].get_all(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].get_all(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].get_all(target)

    def lna_enable(self, 
                  channel: Union[int, List[int], None] = None,
                  target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Enable LNA target (GATE or DRAIN).
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].enable(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].enable(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].enable(target)

    def lna_disable(self, 
                   channel: Union[int, List[int], None] = None,
                   target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Disable LNA target (GATE or DRAIN).
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].disable(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].disable(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].disable(target)

    def lna_set_dac(self, 
                   channel: Union[int, List[int], None] = None,
                   target: Union[str, None] = None,
                   value: Union[int, List[int], None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Set LNA DAC value for target (0-0xFFFF).
        
        Args:
            channel: int for single, list for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            value: int for single value, list for multiple values
            
        Examples:
            lna_set_dac(1, 'GATE', 0x8000)  # Set channel 1 gate to 0x8000
            lna_set_dac(target='GATE', value=0x8000)  # Set all channels gate to 0x8000
            lna_set_dac(target='GATE', value=[0x1000, 0x2000, ...])  # Set all channels to different values
            lna_set_dac([1, 3], 'GATE', 0x8000)  # Set channels 1, 3 gate to 0x8000
            lna_set_dac([1, 3], 'GATE', [0x1000, 0x2000])  # Set channels to different values
            
        Returns:
            Single dict if setting one channel, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
        if value is None:
            raise ValueError("value must be provided")
            
        # Case 1: Single channel, single value
        if isinstance(channel, int) and not isinstance(value, list):
            self._check_lna_channel(channel)
            return self.lna[channel - 1].set_dac(target, value)
        
        # Case 2: All channels, single value
        if channel is None and not isinstance(value, list):
            return [self.lna[i].set_dac(target, value) for i in range(self.num_lna)]
        
        # Case 3: All channels, array of values
        if channel is None and isinstance(value, list):
            if len(value) != self.num_lna:
                raise ValueError(f"value list length {len(value)} must match num_lna {self.num_lna}")
            return [self.lna[i].set_dac(target, value[i]) for i in range(self.num_lna)]
        
        # Case 4: Array of channels, single value
        if isinstance(channel, list) and not isinstance(value, list):
            return [self.lna[ch - 1].set_dac(target, value) for ch in channel]
        
        # Case 5: Array of channels, array of values
        if isinstance(channel, list) and isinstance(value, list):
            if len(channel) != len(value):
                raise ValueError(f"channel and value lists must have same length")
            return [self.lna[ch - 1].set_dac(target, v) for ch, v in zip(channel, value)]
        
        raise ValueError("Invalid combination of channel and value arguments")

    def lna_get_shunt(self, 
                     channel: Union[int, List[int], None] = None,
                     target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get LNA shunt voltage for target.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].get_shunt(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].get_shunt(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].get_shunt(target)

    def lna_get_bus(self, 
                   channel: Union[int, List[int], None] = None,
                   target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get LNA bus voltage for target.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].get_bus(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].get_bus(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].get_bus(target)

    def lna_get_current(self, 
                       channel: Union[int, List[int], None] = None,
                       target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get LNA current for target.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].get_current(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].get_current(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].get_current(target)

    def lna_get_power(self, 
                     channel: Union[int, List[int], None] = None,
                     target: Union[str, None] = None) -> Union[Dict[str, Any], List[Dict[str, Any]]]:
        """Get LNA power for target.
        
        Args:
            channel: int for single channel, list of ints for multiple, None for all channels
            target: 'GATE' or 'DRAIN' (required)
            
        Returns:
            Single dict if channel is int, list of dicts otherwise
        """
        if target is None:
            raise ValueError("target must be provided ('GATE' or 'DRAIN')")
            
        if channel is None:
            return [self.lna[i].get_power(target) for i in range(self.num_lna)]
        elif isinstance(channel, list):
            return [self.lna[ch - 1].get_power(target) for ch in channel]
        else:
            self._check_lna_channel(channel)
            return self.lna[channel - 1].get_power(target)

    # ========== Direct Controller Access ==========
    def get_tes_controller(self, channel: int) -> TesController:
        """Return the TesController instance bound to the given channel."""
        self._check_tes_channel(channel)
        return self.tes[channel - 1]

    def get_lna_controller(self, channel: int) -> LnaController:
        """Return the LnaController instance bound to the given channel."""
        self._check_lna_channel(channel)
        return self.lna[channel - 1]

    def _check_tes_channel(self, channel: int) -> None:
        """Validate a TES channel is within configured bounds (1-based)."""
        if not isinstance(channel, int):
            raise TypeError(f"TES channel must be int, got {type(channel).__name__}")
        if channel < 1 or channel > self.num_tes:
            raise ValueError(f"TES channel {channel} out of range (1..{self.num_tes})")

    def _check_lna_channel(self, channel: int) -> None:
        """Validate an LNA channel is within configured bounds (1-based)."""
        if not isinstance(channel, int):
            raise TypeError(f"LNA channel must be int, got {type(channel).__name__}")
        if channel < 1 or channel > self.num_lna:
            raise ValueError(f"LNA channel {channel} out of range (1..{self.num_lna})")
