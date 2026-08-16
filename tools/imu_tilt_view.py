#!/usr/bin/env python3
"""Live roll/pitch plot from the flight controller's USB CDC debug log.

Reads lines like:
    who=0x69 accel=1074,2498,7911 gyro=-90,-113,-8
(see App/Src/app.cpp's LOG(...) call) from the board's serial port, derives
tilt angles from the raw accelerometer vector, and plots them in real time.

Usage:
    pip install pyserial matplotlib
    python tools/imu_tilt_view.py COM5
    python tools/imu_tilt_view.py COM5 --baud 115200 --window 200

Angle convention (standard accelerometer tilt formulas - only valid while
the board is roughly stationary, since this ignores the gyro entirely):
    roll  = atan2(accelY, accelZ)
    pitch = atan2(-accelX, sqrt(accelY^2 + accelZ^2))
Sign/axis may need flipping depending on how the sensor is physically
mounted relative to the board's "up" - adjust ROLL_SIGN/PITCH_SIGN below
if the plot moves the wrong way.
"""
import argparse
import math
import re
import sys
from collections import deque

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

LINE_RE = re.compile(
    r"accel=(-?\d+),(-?\d+),(-?\d+)\s+gyro=(-?\d+),(-?\d+),(-?\d+)"
)

ROLL_SIGN = 1
PITCH_SIGN = 1


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("port", help="Serial port, e.g. COM5")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (ignored by USB CDC, kept for compatibility)")
    parser.add_argument("--window", type=int, default=200, help="Number of samples to keep on screen")
    return parser.parse_args()


def accel_to_tilt(ax, ay, az):
    roll = math.degrees(math.atan2(ay, az)) * ROLL_SIGN
    pitch = math.degrees(math.atan2(-ax, math.hypot(ay, az))) * PITCH_SIGN
    return roll, pitch


def main():
    args = parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=1)

    times = deque(maxlen=args.window)
    rolls = deque(maxlen=args.window)
    pitches = deque(maxlen=args.window)
    sample_count = 0

    fig, ax = plt.subplots()
    roll_line, = ax.plot([], [], label="roll (deg)")
    pitch_line, = ax.plot([], [], label="pitch (deg)")
    ax.set_xlabel("sample")
    ax.set_ylabel("degrees")
    ax.set_ylim(-180, 180)
    ax.legend(loc="upper right")
    ax.grid(True)

    def update(_frame):
        nonlocal sample_count

        while ser.in_waiting:
            raw_line = ser.readline().decode("utf-8", errors="ignore").strip()
            match = LINE_RE.search(raw_line)
            if not match:
                continue

            ax_raw, ay_raw, az_raw = (int(match.group(i)) for i in (1, 2, 3))
            roll, pitch = accel_to_tilt(ax_raw, ay_raw, az_raw)

            times.append(sample_count)
            rolls.append(roll)
            pitches.append(pitch)
            sample_count += 1

        if times:
            roll_line.set_data(times, rolls)
            pitch_line.set_data(times, pitches)
            ax.set_xlim(max(0, sample_count - args.window), max(sample_count, args.window))

        return roll_line, pitch_line

    ani = animation.FuncAnimation(fig, update, interval=50, blit=False)
    plt.title(f"IMU tilt - {args.port}")
    plt.show()

    ser.close()


if __name__ == "__main__":
    try:
        main()
    except serial.SerialException as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        pass
