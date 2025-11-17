from typing import Optional, Dict, Any

class CommandError(RuntimeError):
    pass

class DacController:
    def __init__(self, client):
        self.client = client

    def _req(self, cmd: str) -> Dict[str, Any]:
        resp = self.client.command_and_read(cmd)
        if not isinstance(resp, dict):
            raise CommandError('Invalid response type')
        status = resp.get('status')
        result = resp.get('result')
        if status == 'error' or (isinstance(status, str) and status.lower() == 'error'):
            # include code/message if present
            code = None
            if isinstance(result, dict):
                code = result.get('code')
            raise CommandError({'status': status, 'result': result})
        return result or {}
    
    def set_dac(self, value: int) -> Dict[str, Any]:
        assert 0 <= value <= 0xFFFF, "value must be between 0 and 0xFFFF"
        cmd = f"DAC SET {value}"
        return self._req(cmd)
    
    def get_dac(self) -> Dict[str, Any]:
        cmd = "DAC GET"
        return self._req(cmd)
    
class TesController:
    """High-level wrapper for TES commands.

    Each instance is bound to a single 1-based channel number.
    """

    def __init__(self, client, channel: int):
        if not isinstance(channel, int):
            raise TypeError(f"channel must be int, got {type(channel).__name__}")
        if channel < 1:
            raise ValueError("channel must be >= 1")
        self.client = client
        self.channel = channel

    def _req(self, cmd: str) -> Dict[str, Any]:
        resp = self.client.command_and_read(cmd)
        if not isinstance(resp, dict):
            raise CommandError('Invalid response type')
        status = resp.get('status')
        result = resp.get('result')
        if status == 'error' or (isinstance(status, str) and status.lower() == 'error'):
            # include code/message if present
            code = None
            if isinstance(result, dict):
                code = result.get('code')
            raise CommandError({'status': status, 'result': result})
        return result or {}

    def enable(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} ENABLE"
        return self._req(cmd)

    def disable(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} DISABLE"
        return self._req(cmd)
    
    def set_current(self, current_mA: float) -> Dict[str, Any]:
        assert 0 <= current_mA <= 20.0, "current_mA must be between 0 and 20.0"
        cmd = f"TES {self.channel} SET {current_mA}"
        return self._req(cmd)
    
    def inc_current(self, delta: int) -> Dict[str, Any]:
        cmd = f"TES {self.channel} INC {delta}"
        return self._req(cmd)

    def dec_current(self, delta: int) -> Dict[str, Any]:
        cmd = f"TES {self.channel} DEC {delta}"
        return self._req(cmd)

    def set_bits(self, value: int) -> Dict[str, Any]:
        assert 0 <= value <= 0xFFFFF, "value must be between 0 and 0xFFFFF"
        cmd = f"TES {self.channel} SETINT {value}"
        return self._req(cmd)

    def get_all(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} GET"
        return self._req(cmd)

    def get_bits(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} BIT"
        return self._req(cmd)

    def get_shunt(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} SHUNT"
        return self._req(cmd)

    def get_bus(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} BUS"
        return self._req(cmd)

    def get_current(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} CURRENT"
        return self._req(cmd)

    def get_power(self) -> Dict[str, Any]:
        cmd = f"TES {self.channel} POWER"
        return self._req(cmd)


class LnaController:
    """Wrapper for LNA commands (gate/drain).

    Each instance is bound to a single 1-based channel number. Methods do not
    accept a channel parameter; they use the controller's channel.
    """

    def __init__(self, client, channel: int):
        if not isinstance(channel, int):
            raise TypeError(f"channel must be int, got {type(channel).__name__}")
        if channel < 1:
            raise ValueError("channel must be >= 1")
        self.client = client
        self.channel = channel

    def _check_target(self, target: str) -> None:
        if not isinstance(target, str):
            raise TypeError(f"target must be str, got {type(target).__name__}")
        if target.upper() not in ('GATE', 'DRAIN'):
            raise ValueError("target must be 'GATE' or 'DRAIN'")

    def _req(self, cmd: str):
        resp = self.client.command_and_read(cmd)
        if not isinstance(resp, dict):
            raise CommandError('Invalid response type')
        status = resp.get('status')
        if status == 'error' or (isinstance(status, str) and status.lower() == 'error'):
            raise CommandError(resp)
        return resp.get('result') or {}
    
    def enable(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} ENABLE {target}"
        return self._req(cmd)

    def disable(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} DISABLE {target}"
        return self._req(cmd)

    def set_dac(self, target: str, value: int) -> Dict[str, Any]:
        assert 0 <= value <= 0xFFFF, "value must be between 0 and 0xFFFF"
        self._check_target(target)
        cmd = f"LNA {self.channel} SET {target} {value}"
        return self._req(cmd)

    def get_all(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} GET {target}"
        return self._req(cmd)

    def get_shunt(self, target: str) -> Dict[str, Any]:
        self.channel._check_target(target)
        cmd = f"LNA {self.channel} SHUNT {target}"
        return self._req(cmd)

    def get_bus(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} BUS {target}"
        return self._req(cmd)

    def get_current(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} CURRENT {target}"
        return self._req(cmd)

    def get_power(self, target: str) -> Dict[str, Any]:
        self._check_target(target)
        cmd = f"LNA {self.channel} POWER {target}"
        return self._req(cmd)