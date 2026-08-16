#!/usr/bin/env python3
import argparse
import math
import re
import sys

import numpy as np
import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

LINE_RE = re.compile(r"roll=(-?\d+\.?\d*)\s+pitch=(-?\d+\.?\d*)")

ARM_TIPS = np.array([
    [1, 1, 0],
    [1, -1, 0],
    [-1, -1, 0],
    [-1, 1, 0],
], dtype=float) / math.sqrt(2)
DIAGONAL_PAIRS = [(0, 2), (1, 3)]
NOSE = np.array([1.3, 0, 0])
CENTER = np.array([0, 0, 0])


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("--baud", type=int, default=115200)
    return parser.parse_args()


def rotation_matrix(roll, pitch, yaw=0.0):
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)

    rx = np.array([[1, 0, 0], [0, cr, -sr], [0, sr, cr]])
    ry = np.array([[cp, 0, sp], [0, 1, 0], [-sp, 0, cp]])
    rz = np.array([[cy, -sy, 0], [sy, cy, 0], [0, 0, 1]])
    return rz @ ry @ rx


def main():
    args = parse_args()
    ser = serial.Serial(args.port, args.baud, timeout=1)

    fig = plt.figure()
    ax3d = fig.add_subplot(111, projection="3d")
    ax3d.set_xlim(-2, 2)
    ax3d.set_ylim(-2, 2)
    ax3d.set_zlim(-2, 2)
    ax3d.set_xlabel("X (forward)")
    ax3d.set_ylabel("Y (right)")
    ax3d.set_zlabel("Z (up)")

    arm_lines = [ax3d.plot([], [], [], "o-", lw=3, markersize=8)[0] for _ in DIAGONAL_PAIRS]
    nose_line, = ax3d.plot([], [], [], "r-", lw=2)

    state = {"roll": 0.0, "pitch": 0.0}

    def update(_frame):
        while ser.in_waiting:
            raw_line = ser.readline().decode("utf-8", errors="ignore").strip()
            match = LINE_RE.search(raw_line)
            if not match:
                continue
            roll_deg, pitch_deg = (float(match.group(i)) for i in (1, 2))
            state["roll"] = math.radians(roll_deg)
            state["pitch"] = math.radians(pitch_deg)

        rot = rotation_matrix(state["roll"], state["pitch"])
        tips_r = ARM_TIPS @ rot.T
        nose_r = rot @ NOSE
        center_r = rot @ CENTER

        for line, (i, j) in zip(arm_lines, DIAGONAL_PAIRS):
            p1, p2 = tips_r[i], tips_r[j]
            line.set_data([p1[0], p2[0]], [p1[1], p2[1]])
            line.set_3d_properties([p1[2], p2[2]])

        nose_line.set_data([center_r[0], nose_r[0]], [center_r[1], nose_r[1]])
        nose_line.set_3d_properties([center_r[2], nose_r[2]])

        ax3d.set_title(f"roll={math.degrees(state['roll']):.1f} deg  pitch={math.degrees(state['pitch']):.1f} deg")

        return arm_lines + [nose_line]

    ani = animation.FuncAnimation(fig, update, interval=50, blit=False)
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
