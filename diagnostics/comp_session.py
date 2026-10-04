"""Supervised open-ended COMP run: sends COMP ON, then KEEP every 2 s until stopped.

Usage: python comp_session.py LOGNAME STOPFILE [MAX_MINUTES]
Creating STOPFILE sends COMP OFF and exits. If this script dies, the firmware
stops on its own within 10 s (no KEEP). Any firmware stop is logged and ends the run.
Log: logs/LOGNAME.txt with host timestamps.
"""
import pathlib, sys, time
import serial

name, stopfile = sys.argv[1], pathlib.Path(sys.argv[2])
max_s = float(sys.argv[3]) * 60 if len(sys.argv) > 3 else 15 * 60
log = pathlib.Path(__file__).resolve().parents[1] / "logs" / f"{name}.txt"
stopfile.unlink(missing_ok=True)
port = serial.Serial()
port.port, port.baudrate, port.timeout = "COM5", 115200, 0.05
port.dtr = port.rts = False  # avoid an intentional reset on open
start = time.monotonic()
buffer, last_keep, ended = "", 0.0, None

with open(log, "a", encoding="utf-8") as out:
    def note(text):
        line = f"[{time.monotonic() - start:7.2f}s] {text}"
        print(line, flush=True); out.write(line + "\n"); out.flush()

    def send(cmd):
        port.write(cmd.encode() + b"\n"); port.flush()

    def pump():
        global buffer, ended
        buffer += port.read(4096).decode("utf-8", errors="replace")
        *lines, buffer = buffer.split("\n")
        for line in lines:
            note(line.rstrip())
            if "SERVO SIGNAL OFF" in line or "COMP refused" in line:
                ended = line.strip()

    out.write(f"--- comp session {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
    try:
        port.open(); port.reset_input_buffer()
        send("COMP ON"); note(">>> SENT COMP ON")
        while ended is None:
            pump()
            now = time.monotonic()
            if stopfile.exists():
                send("COMP OFF"); note(">>> SENT COMP OFF (user stop)")
                end = now + 1
                while time.monotonic() < end: pump()
                break
            if now - start > max_s:
                send("COMP OFF"); note(">>> SENT COMP OFF (host max duration)")
                break
            if now - last_keep >= 2:
                send("KEEP"); last_keep = now
        if ended: note(f"RUN ENDED: {ended}")
    finally:
        if port.is_open: port.close()
        stopfile.unlink(missing_ok=True)
