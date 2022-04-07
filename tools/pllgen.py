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
    args = parser.parse_args()

    # HSI / DIVM2 * DIVN2 / DIVP2 => PLL2P
    # Sample rate = PLL2P / 2048

    hsi = 64000000
    for divm2 in range(1, 64):
        for divn2 in range(8, 421):
            for divp2 in range(1, 129):
                freq = ((hsi / divm2) * divn2) / divp2 / 2048.0
                if freq == args.samplerate:
                    print(f"DIVM2={divm2}, DIVN2={divn2}, DIVP2={divp2} => freq")
