#!/usr/bin/env python3
import argparse
import statistics
import time
import urllib.parse
import urllib.request


def percentile(values, pct):
    if not values:
        return 0.0
    values = sorted(values)
    idx = int(round((pct / 100.0) * (len(values) - 1)))
    return values[max(0, min(len(values) - 1, idx))]


def main():
    parser = argparse.ArgumentParser(description="Measure simple FTHT HTTP latency and payload size.")
    parser.add_argument("url")
    parser.add_argument("-n", "--requests", type=int, default=200)
    parser.add_argument("--method", choices=("GET", "POST"), default="GET")
    parser.add_argument("--body", default="")
    parser.add_argument("--ftht-client", action="store_true", help="Send X-FTHT-Client: fetch for partial updates.")
    args = parser.parse_args()

    headers = {}
    data = None
    if args.method == "POST":
        data = args.body.encode("utf-8")
        headers["Content-Type"] = "application/x-www-form-urlencoded;charset=UTF-8"
    if args.ftht_client:
        headers["X-FTHT-Client"] = "fetch"

    times_ms = []
    sizes = []
    for _ in range(args.requests):
        req = urllib.request.Request(args.url, data=data, headers=headers, method=args.method)
        start = time.perf_counter()
        with urllib.request.urlopen(req, timeout=10) as response:
            payload = response.read()
        elapsed_ms = (time.perf_counter() - start) * 1000.0
        times_ms.append(elapsed_ms)
        sizes.append(len(payload))

    print(f"requests={args.requests}")
    print(f"method={args.method}")
    print(f"ftht_client={1 if args.ftht_client else 0}")
    print(f"bytes_avg={statistics.mean(sizes):.1f}")
    print(f"latency_avg_ms={statistics.mean(times_ms):.3f}")
    print(f"latency_p50_ms={statistics.median(times_ms):.3f}")
    print(f"latency_p95_ms={percentile(times_ms, 95):.3f}")
    print(f"latency_min_ms={min(times_ms):.3f}")
    print(f"latency_max_ms={max(times_ms):.3f}")


if __name__ == "__main__":
    main()
