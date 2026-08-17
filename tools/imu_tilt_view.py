#!/usr/bin/env python3
import argparse
import re
import sys
from collections import deque

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

LINE_RE = re.compile(r"roll=(-?\d+\.?\d*)\s+pitch=(-?\d+\.?\d*)")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--window", type=int, default=200)
    return parser.parse_args()


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

            roll, pitch = (float(match.group(i)) for i in (1, 2))

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
