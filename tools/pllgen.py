#!/usr/bin/env python3

import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generates PLL parameters for the SAI peripheral")
    parser.add_argument(
        "--samplerate",
        type=int,
        default="48000",
        help="Target sample rate",
    )
    parser.add_argument(
        "--error",
        type=lambda x: float(x) / 100.0,
        default=0,
        help="Allow for error margin in percent (0.5 => ±0.5%)",
    )
    args = parser.parse_args()

    # HSI / DIVM2 * (DIVN2 + (FRACN / 2^13)) / DIVP2 => PLL2P
    # Sample rate = PLL2P / 2048

    hsi = 64000000
    for fracn2 in range(0, 8191):
        for divm2 in range(1, 64):
            for divn2 in range(8, 421):
                for divp2 in range(1, 129):
                    freq = ((hsi / divm2) * (divn2 + fracn2 / (1<<13))) / divp2 / 2048
                    error = abs(1.0 - args.samplerate / freq)
                    if freq == args.samplerate:
                        print(f"DIVM2={divm2}, DIVN2={divn2}, FRACN2={fracn2} DIVP2={divp2} => {freq}")
                    elif error < args.error:
                        print(f"DIVM2={divm2}, DIVN2={divn2}, FRACN2={fracn2} DIVP2={divp2} => {freq:.5f}, error={error * 100:.5f}%")
