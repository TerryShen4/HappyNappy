"""Sleep-stage tracking.

`SleepTracker` keeps the running BPM history and decides when to wake the
sleeper. All the detection state lives in one object instead of scattered
module globals, so the rules are easy to follow.
"""

from datetime import datetime

from config import (
    BASELINE_READINGS,
    DEEP_SLEEP_DROP_PCT,
    FALLING_ASLEEP_DROP_PCT,
)


class SleepTracker:
    """Accumulates BPM readings and flags the transition into deep sleep.

    The strategy: average the first ``BASELINE_READINGS`` BPMs into a resting
    baseline, then watch for a sustained drop. A large drop is treated as
    entering deep sleep -- the moment we want to wake the sleeper to avoid
    grogginess.
    """

    def __init__(self):
        self.bpm_history = []        # list of {"time": seconds, "bpm": value}
        self.start_time = None
        self.baseline_bpm = None
        self.sleep_stage = "awake"   # awake | falling_asleep | deep_sleep | demo_mode
        self.awake = False           # True once a wake alert has been armed/fired
        self._demo_triggered = False

    def record_bpm(self, bpm):
        """Record a new BPM reading and run detection.

        Returns a list of events for the caller to act on. Currently the only
        event is ``"wake"``, meaning a wake alert should be sent to the ESP32.
        """
        events = []

        if self.start_time is None:
            self.start_time = datetime.now()
        elapsed = (datetime.now() - self.start_time).total_seconds()
        self.bpm_history.append({"time": elapsed, "bpm": bpm})

        # Demo mode: if a wake was armed manually, fire once on the next reading.
        if self.awake and not self._demo_triggered:
            self._demo_triggered = True
            self.sleep_stage = "demo_mode"
            print("🎯 DEMO MODE: triggering wake alert!")
            events.append("wake")

        # Establish the resting baseline from the first N readings.
        if self.baseline_bpm is None and len(self.bpm_history) >= BASELINE_READINGS:
            self.baseline_bpm = (
                sum(r["bpm"] for r in self.bpm_history[:BASELINE_READINGS])
                / BASELINE_READINGS
            )
            threshold = self.baseline_bpm * (1 - DEEP_SLEEP_DROP_PCT / 100)
            print(
                f"✅ Baseline BPM established: {self.baseline_bpm:.2f} "
                f"(wake if BPM drops below {threshold:.2f})"
            )

        # Watch for a sustained drop from baseline once we have one.
        if self.baseline_bpm is not None and not self.awake:
            drop_pct = (self.baseline_bpm - bpm) / self.baseline_bpm * 100
            if drop_pct >= DEEP_SLEEP_DROP_PCT:
                self.awake = True
                self.sleep_stage = "deep_sleep"
                print(
                    f"🚨 DEEP SLEEP DETECTED! baseline {self.baseline_bpm:.1f} -> "
                    f"{bpm:.1f} BPM ({drop_pct:.1f}% drop)"
                )
                events.append("wake")
            elif drop_pct >= FALLING_ASLEEP_DROP_PCT and self.sleep_stage != "falling_asleep":
                self.sleep_stage = "falling_asleep"
                print(f"😴 Falling asleep... (BPM dropped {drop_pct:.1f}%)")

        return events

    def arm_wake(self):
        """Manually arm a wake alert (used by the test endpoint)."""
        self.awake = True

    @property
    def current_bpm(self):
        return self.bpm_history[-1]["bpm"] if self.bpm_history else None

    @property
    def total_readings(self):
        return len(self.bpm_history)
