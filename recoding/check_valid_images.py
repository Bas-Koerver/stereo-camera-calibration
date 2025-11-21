import glob

import h5py
import numpy as np
import cv2


def frame_at_nth_trigger(
        h5_path: str,
        n: int,
        exposure_us: int,
        cd_path: str = "/CD/events",
        trig_path: str = "/EXT_TRIGGER/events",
        width: int | None = None,
        height: int | None = None,
        only_on: bool = False,  # if True: only CD events with p == 1
):
    """
    Generate a CD frame around the n-th positive external trigger.

    Parameters
    ----------
    h5_path : str
        Path to Prophesee HDF5 recording.
    n : int
        0-based index of positive external trigger to use.
    exposure_us : int
        Length of the window *before* the trigger (same time units as in file,
        often microseconds).
    cd_path : str
        HDF5 path to CD events dataset.
    trig_path : str
        HDF5 path to external trigger events dataset.
    width, height : int | None
        Sensor width/height. If None, inferred from the CD events in the window.
    only_on : bool
        If True, only use CD events with polarity 1.

    Returns
    -------
    frame : np.ndarray (H x W, dtype=np.uint16)
        Accumulated event counts.
    t_ref : int
        Timestamp of the n-th positive trigger.
    cd_slice : dict[str, np.ndarray]
        CD events in the window (x, y, t, p).
    """

    with h5py.File(h5_path, "r") as f:
        # ----- 1. Get n-th positive trigger -----
        trig = f[trig_path]
        trig_t = trig["t"][:]    # timestamps of trigger events
        trig_p = trig["p"][:]    # polarity of trigger events (0/1)

        pos_trig_idx = np.flatnonzero(trig_p == 1)
        if n < 0 or n >= len(pos_trig_idx):
            raise IndexError(
                f"Requested positive trigger {n}, but only {len(pos_trig_idx)} positive triggers in file."
            )

        idx_trig = pos_trig_idx[n]
        t_ref = int(trig_t[idx_trig])
        t_start = t_ref - exposure_us

        # ----- 2. Get CD events in [t_start, t_ref] -----
        cd = f[cd_path]
        cd_t_all = cd["t"][:]  # assume time-sorted

        # use searchsorted instead of full boolean mask to be fast on big files
        start_idx = np.searchsorted(cd_t_all, t_start, side="left")
        end_idx = np.searchsorted(cd_t_all, t_ref, side="right")

        cd_slice_raw = cd[start_idx:end_idx]

        x = cd_slice_raw["x"].astype(np.int64)
        y = cd_slice_raw["y"].astype(np.int64)
        t = cd_slice_raw["t"]
        p = cd_slice_raw["p"]

    # optional: only ON events
    if only_on:
        mask = (p == 1)
        x, y, t, p = x[mask], y[mask], t[mask], p[mask]

    # ----- 3. Infer / set frame size -----
    if x.size == 0 or y.size == 0:
        raise ValueError("No CD events in the specified time window.")

    if width is None:
        width = int(x.max()) + 1
    if height is None:
        height = int(y.max()) + 1

    # ----- 4. Accumulate events into a frame -----
    frame = np.zeros((height, width), dtype=np.uint16)

    # vectorized accumulation
    valid_mask = (x >= 0) & (x < width) & (y >= 0) & (y < height)
    x_valid = x[valid_mask]
    y_valid = y[valid_mask]

    np.add.at(frame, (y_valid, x_valid), 1)

    cd_slice = {"x": x, "y": y, "t": t, "p": p}
    return frame, t_ref, cd_slice



if __name__ == "__main__":
    path = "C:/Users/Bas_K/source/repos/Thesis/stereo-camera-calibration/recoding/cmake-build-release-visual-studio/data/eventfile/raw_recording_2025-11-21_08-13-23.hdf5"
    data_dir = "C:/Users/Bas_K/source/repos/Thesis/stereo-camera-calibration/recoding/cmake-build-release-visual-studio/data/"
    exposure_us = 33333

    rgb_images = glob.glob(data_dir + "/images/basler_rgb/" + "*.png")
    for rgb_image in rgb_images:
        frame_number = int(rgb_image.split("_")[-1].split(".png")[0])
        frame, t_ref, cd_events = frame_at_nth_trigger(
            path,
            frame_number,
            exposure_us,
            only_on=False,
        )



    print("Trigger timestamp:", t_ref)
    print("CD events in window:", len(cd_events["t"]))
    print("Frame shape:", frame.shape)

    # Quick visualization with OpenCV
    vis = frame.astype(np.float32)
    vis /= (vis.max() + 1e-9)
    vis = (vis * 255).astype(np.uint8)
    cv2.imwrite("event_frame.png", vis)
