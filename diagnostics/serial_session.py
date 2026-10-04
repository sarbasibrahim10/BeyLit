"""Send BeyLit serial commands and capture a timestamped log.

Usage: python serial_session.py LOGNAME SECONDS [COMMAND ...]
Each COMMAND is sent in order; "WAIT:n" pauses n seconds while still reading.
Output is printed and appended to logs/LOGNAME.txt with host timestamps.
"""
import pathlib, sys, time
import serial

name, seconds, steps = sys.argv[1], float(sys.argv[2]), sys.argv[3:]
log = pathlib.Path(__file__).resolve().parents[1] / "logs" / f"{name}.txt"
port = serial.Serial()
port.port, port.baudrate, port.timeout = "COM5", 115200, 0.05
port.dtr = port.rts = False  # avoid an intentional reset on open
start = time.monotonic()
buffer = ""

def pump(out, until):
    global buffer
    while time.monotonic() < until:
        buffer += port.read(4096).decode("utf-8", errors="replace")
        *lines, buffer = buffer.split("\n")
        for line in lines:
            line = f"[{time.monotonic() - start:7.2f}s] {line.rstrip()}"
            print(line, flush=True); out.write(line + "\n")

with open(log, "a", encoding="utf-8") as out:
    out.write(f"--- session {time.strftime('%Y-%m-%d %H:%M:%S')} steps={steps}\n")
    try:
        port.open(); port.reset_input_buffer()
        for step in steps:
            if step.startswith("WAIT:"):
                pump(out, time.monotonic() + float(step[5:])); continue
            port.write(step.encode() + b"\n"); port.flush()
            note = f"[{time.monotonic() - start:7.2f}s] >>> SENT {step}"
            print(note, flush=True); out.write(note + "\n")
            pump(out, time.monotonic() + 0.3)
        pump(out, time.monotonic() + seconds)
    finally:
        if port.is_open: port.close()
