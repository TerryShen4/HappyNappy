"""Pure signal-processing helpers: turn a raw IR/PPG window into a BPM.

These functions hold no state and do no I/O, which keeps them easy to read
and to test in isolation.
"""

import math

import heartpy as hp

from config import BANDPASS_CUTOFF, BANDPASS_ORDER, MIN_CONTACT_IR


def average_ir(ir_data):
    """Mean of an IR sample window (used as a contact-quality proxy)."""
    return sum(ir_data) / len(ir_data)


def has_good_contact(ir_data):
    """True when the sensor is reading skin (enough reflected IR)."""
    return average_ir(ir_data) >= MIN_CONTACT_IR


def calculate_bpm(ir_data, sample_rate):
    """Bandpass-filter the PPG window and return a BPM.

    Returns the BPM rounded to 2 decimals, or ``None`` if the result is not a
    finite number. May raise if HeartPy cannot process the signal at all --
    the caller is expected to handle that.
    """
    filtered = hp.filter_signal(
        ir_data,
        cutoff=BANDPASS_CUTOFF,
        sample_rate=sample_rate,
        order=BANDPASS_ORDER,
        filtertype="bandpass",
    )
    _, measures = hp.process(
        filtered,
        sample_rate=sample_rate,
        high_precision=True,
        clean_rr=True,
    )
    bpm = measures["bpm"]
    if not math.isfinite(bpm):
        return None
    return round(bpm, 2)
