import serial
import time
import yaml

class SerialClient:
    """Simple serial client to send a single-line command and read a YAML block response.

    The sketch prints a YAML block beginning with '---' and ending with a blank line.
    This class sends the command followed by a CRLF and reads until a blank line is seen.
    """

    def __init__(self, port: str, baud: int = 115200, timeout: float = 1.0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self._serial = None

    def open(self):
        if self._serial and self._serial.is_open:
            return
        self._serial = serial.Serial(self.port, self.baud, timeout=self.timeout)
        # small delay to allow MCU boot banners to settle
        time.sleep(0.1)

    def close(self):
        if self._serial:
            try:
                self._serial.close()
            except Exception:
                pass
            self._serial = None

    def send_command(self, cmd: str):
        """Send a command line to the device. Newline is appended automatically."""
        if not self._serial or not self._serial.is_open:
            self.open()
        line = cmd.strip() + "\r\n"
        self._serial.write(line.encode('utf-8'))

    def _read_block(self, timeout: float = None) -> str:
        """Read a YAML block from serial. Returns the raw string (including leading '---')."""
        if not self._serial or not self._serial.is_open:
            self.open()
        end_time = time.time() + (timeout if timeout is not None else self.timeout)
        lines = []
        saw_start = False
        while True:
            if time.time() > end_time:
                break
            raw = self._serial.readline()
            if not raw:
                continue
            try:
                line = raw.decode('utf-8', errors='ignore').rstrip('\r\n')
            except Exception:
                line = raw.decode('latin-1', errors='ignore').rstrip('\r\n')
            if not saw_start:
                if line.strip() == '---':
                    saw_start = True
                    lines.append(line)
                else:
                    # skip any startup noise until the YAML block
                    continue
            else:
                lines.append(line)
                # blank line ends the block
                if line.strip() == '':
                    break
        return '\n'.join(lines)

    def read_response(self, timeout: float = None) -> dict:
        """Read a response and return parsed YAML as a dict.

        Returns a dict with at least a 'status' key and 'result' mapping (if present).
        If no YAML block is received, raises RuntimeError.
        """
        raw = self._read_block(timeout=timeout)
        if not raw:
            raise RuntimeError('No response from device')
        # Parse YAML safely
        try:
            parsed = yaml.safe_load(raw)
        except Exception as e:
            # If YAML parse fails, return raw text in an envelope
            return {'status': 'parse_error', 'raw': raw, 'error': str(e)}
        return parsed

    def command_and_read(self, cmd: str, timeout: float = None) -> dict:
        self.send_command(cmd)
        return self.read_response(timeout=timeout)
