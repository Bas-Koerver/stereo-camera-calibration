import cv2 as cv
import json
import numpy as np
import os
import pathlib
from glob import glob


def splitfn(fn):
    path, fn = os.path.split(fn)
    name, ext = os.path.splitext(fn)
    return path, name, ext


def load_calibration(calibration_file):
    with open(calibration_file, 'r') as f:
        data = json.load(f)

    return np.array(data['camera_matrix']), np.array(data['dist_coefs'])


def stereo_calibrate(mtx1, dist1, mtx2, dist2, data_dir_left, data_dir_right):
    height = 5
    width = 7
    square_size = 0.0298
    marker_size = 0.0208
    aruco_dict_name = cv.aruco.DICT_6X6_50

    pattern_size = (width, height)

    # read the synched frames
    image_names_left = glob(data_dir_left)
    image_names_right = glob(data_dir_right)

    cl_images = []
    cr_images = []

    aruco_dict = cv.aruco.getPredefinedDictionary(aruco_dict_name)
    board = cv.aruco.CharucoBoard(pattern_size, square_size, marker_size, aruco_dict)
    charuco_detector = cv.aruco.CharucoDetector(board)

    for iml, imr in zip(image_names_left, image_names_right):
        _im = cv.imread(iml, 1)
        cl_images.append(_im)

        _im = cv.imread(imr, 1)
        cr_images.append(_im)

    # change this if stereo calibration not good.
    criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 100, 0.0001)

    # objp = np.zeros((pattern_size[1]*pattern_size[0],3), np.float32)
    # objp[:,:2] = np.mgrid[0:pattern_size[1],0:pattern_size[0]].T.reshape(-1,2)

    # frame dimensions. Frames should be the same size.
    width = cl_images[0].shape[1]
    height = cl_images[0].shape[0]

    # Pixel coordinates of checkerboards
    object_points = []
    imgpoints_left = []  # 2d points in image plane.
    imgpoints_right = []

    allCorners = {'left': [], 'right': []}
    allIds = {'left': [], 'right': []}

    # coordinates of the checkerboard in checkerboard world space.
    # objpoints = []  # 3d point in real world space

    for frame_l, frame_r in zip(cl_images, cr_images):
        grayl = cv.cvtColor(frame_l, cv.COLOR_BGR2GRAY)
        grayr = cv.cvtColor(frame_r, cv.COLOR_BGR2GRAY)

        charuco_corners_l, charuco_ids_l, _, _ = charuco_detector.detectBoard(grayl)
        charuco_corners_r, charuco_ids_r, _, _ = charuco_detector.detectBoard(grayr)

        if len(np.intersect1d(charuco_ids_r, charuco_ids_l)) > 10:
            allCorners['left'].append(charuco_corners_l)
            allCorners['right'].append(charuco_corners_r)
            allIds['left'].append(charuco_ids_l)
            allIds['right'].append(charuco_ids_r)

    for i in range(len(allCorners['left'])):
        common_ids = np.intersect1d(allIds['left'][i], allIds['right'][i])

        if len(common_ids) > 0:
            indices_left = np.isin(allIds['left'][i], common_ids).flatten()
            indices_right = np.isin(allIds['right'][i], common_ids).flatten()

            frame_obj_points_l, frame_img_points_l = board.matchImagePoints(allCorners['left'][i][indices_left],
                                                                            common_ids)
            frame_obj_points_r, frame_img_points_r = board.matchImagePoints(allCorners['right'][i][indices_right],
                                                                            common_ids)
            object_points.append(frame_obj_points_l)
            imgpoints_left.append(frame_img_points_l)
            imgpoints_right.append(frame_img_points_r)

    stereocalibration_flags = cv.CALIB_FIX_INTRINSIC
    ret, CM1, dist1, CM2, dist2, R, T, E, F = cv.stereoCalibrate(object_points, imgpoints_left, imgpoints_right, mtx1,
                                                                 dist1,
                                                                 mtx2, dist2, (width, height), criteria=criteria,
                                                                 flags=stereocalibration_flags)

    print(ret)
    return R, T


def calibrate(img_name, debug_dir):
    img_names = sorted(glob(img_name))
    if len(img_names) < 1:
        print("No images found")
        return None

    pathlib.Path(debug_dir).mkdir(parents=True, exist_ok=True)

    height = 5
    width = 7
    square_size = 0.0298
    marker_size = 0.0208
    aruco_dict_name = cv.aruco.DICT_6X6_50

    pattern_size = (width, height)

    obj_points = []
    img_points = []
    # h, w = cv.imread(img_names[0], cv.IMREAD_GRAYSCALE).shape[:2]
    image_size = cv.imread(img_names[0], cv.IMREAD_GRAYSCALE).shape[::-1]

    aruco_dict = cv.aruco.getPredefinedDictionary(aruco_dict_name)
    board = cv.aruco.CharucoBoard(pattern_size, square_size, marker_size, aruco_dict)
    charuco_detector = cv.aruco.CharucoDetector(board)

    def processImage(fn):
        print('processing %s... ' % fn)
        img_c = cv.imread(fn)
        img = cv.cvtColor(img_c, cv.COLOR_BGR2GRAY)
        if img is None:
            print("Failed to load", fn)
            return None

        assert image_size[0] == img.shape[1] and image_size[1] == img.shape[0], (
                "size: %d x %d ... " % (img.shape[1], img.shape[0]))
        found = False
        corners = 0

        corners, charucoIds, _, _ = charuco_detector.detectBoard(img)

        if (len(corners) > 3):
            frame_obj_points, frame_img_points = board.matchImagePoints(corners, charucoIds)
            found = True
        else:
            found = False

        img1 = img_c.copy()
        img2 = img_c.copy()
        img3 = img_c.copy()

        if frame_img_points is None or frame_img_points.size == 0 or frame_img_points.shape[0] < 3:
            return  # or just skip drawing

        x0 = int(frame_img_points[:, 0, 0].min())
        y0 = int(frame_img_points[:, 0, 1].min())
        x1 = int(frame_img_points[:, 0, 0].max())
        y1 = int(frame_img_points[:, 0, 1].max())

        # Guard: no NaNs/inf
        frame_img_points2 = frame_img_points.reshape(-1, 2)
        if not np.isfinite(frame_img_points2).all():
            return  # skip this frame

        # Normalize to contour format OpenCV likes: (N,1,2) int32
        # frame_img_points3 = np.ascontiguousarray(frame_img_points2.astype(np.int32)).reshape(-1, 1, 2)

        nx, ny = board.getChessboardSize()

        def id_to_rowcol(cid, nx):
            row = cid // nx
            col = cid % nx
            return row, col

        rows = {}
        for pt, cid in zip(corners.reshape(-1, 2), charucoIds.flatten()):
            r, c = id_to_rowcol(cid, nx)
            rows.setdefault(r, []).append((c, pt))

        for r, cols in rows.items():
            cols.sort(key=lambda x: x[0])  # sort by column index
            pts = np.array([p for _, p in cols], dtype=np.int32)

            if len(pts) >= 2:
                pts = pts.reshape(-1, 1, 2)
                cv.polylines(img1, [pts], False, (0, 0, 255), 2)

        # cv.polylines(img1, [box], True, (0, 255, 0), 3)

        cv.polylines(img3, [frame_img_points.astype(int)], False, (0, 0, 255), 2)

        cv.imshow('img1', img1)
        # cv.imshow('img2', img2)
        cv.imshow('img3', img3)
        while (True):
            k = cv.waitKey(1)
            if k == 27:
                break
        cv.destroyAllWindows()
        return

        if debug_dir:
            vis = cv.cvtColor(img, cv.COLOR_GRAY2BGR)

            cv.aruco.drawDetectedCornersCharuco(vis, corners, charucoIds=charucoIds)
            _path, name, _ext = splitfn(fn)
            outfile = os.path.join(debug_dir, name + '_board.png')
            cv.imwrite(outfile, vis)

        if not found:
            print('pattern not found')
            return None

        print('           %s... OK' % fn)
        return (frame_img_points, frame_obj_points)

    chessboards = [processImage(fn) for fn in img_names]

    chessboards = [x for x in chessboards if x is not None]
    for (corners, pattern_points) in chessboards:
        img_points.append(corners)
        obj_points.append(pattern_points)

    # calculate camera distortion
    rms, camera_matrix, dist_coefs, _rvecs, _tvecs = cv.calibrateCamera(obj_points, img_points, image_size, None, None)

    cv.matchI

    data = {
        "camera_matrix": camera_matrix.tolist(),
        "dist_coefs": dist_coefs.tolist(),
        "rms": rms,
    }
    with open(f"{debug_dir}/calibration.json", 'w') as outfile:
        json.dump(data, outfile)

    print("\nRMS:", rms)
    print("camera matrix:\n", camera_matrix)
    print("distortion coefficients: ", dist_coefs.ravel())

    mean_error = 0
    for i in range(len(obj_points)):
        img_points_2, _ = cv.projectPoints(obj_points[i], _rvecs[i], _tvecs[i], camera_matrix, dist_coefs)
        error = cv.norm(img_points[i], img_points_2, cv.NORM_L2) / len(img_points_2)
        mean_error += error

    print("total error: {}".format(mean_error / len(obj_points)))

    # undistort the image with the calibration
    print('')
    for fn in img_names if debug_dir else []:
        _path, name, _ext = splitfn(fn)
        img_found = os.path.join(debug_dir, name + '_board.png')
        outfile = os.path.join(debug_dir, name + '_undistorted.png')

        img = cv.imread(img_found)
        if img is None:
            continue

        h, w = img.shape[:2]
        newcameramtx, roi = cv.getOptimalNewCameraMatrix(camera_matrix, dist_coefs, (w, h), 1, (w, h))

        dst = cv.undistort(img, camera_matrix, dist_coefs, None, newcameramtx)

        # crop and save the image
        x, y, w, h = roi
        dst = dst[y:y + h, x:x + w]

        # print('Undistorted image written to: %s' % outfile)
        cv.imwrite(outfile, dst)

    print('Done')
    return rms, camera_matrix, dist_coefs, _rvecs, _tvecs


def compute_disparity(rectL, rectR):
    if len(rectL.shape) == 3:
        grayL = cv.cvtColor(rectL, cv.COLOR_BGR2GRAY)
        grayR = cv.cvtColor(rectR, cv.COLOR_BGR2GRAY)
    else:
        grayL, grayR = rectL, rectR

    min_disp = 0
    num_disp = 128  # multiple of 16
    block_size = 5

    stereo = cv.StereoSGBM_create(
        minDisparity=min_disp,
        numDisparities=num_disp,
        blockSize=block_size,
        P1=8 * block_size * block_size,
        P2=32 * block_size * block_size,
        disp12MaxDiff=1,
        uniquenessRatio=10,
        speckleWindowSize=50,
        speckleRange=2,
        preFilterCap=63,
        mode=cv.STEREO_SGBM_MODE_SGBM_3WAY,
    )

    disp = stereo.compute(grayL, grayR).astype(np.float32) / 16.0
    disp[disp <= 0] = np.nan  # treat invalid as NaN
    return disp


def transfer_bbox_left_to_right(bbox_left, disparity,
                                min_valid_fraction=0.05,
                                corner_win=9):
    """
    Transfer bbox from rectified left image to rectified right image
    using per-corner disparities.

    bbox_left: (x, y, w, h) in rectL coordinates
    disparity: float32 disp map (same size as rectL/rectR)
    returns: (xR, yR, wR, hR) in rectR coords, or None
    """

    H, W = disparity.shape[:2]
    x, y, w, h = bbox_left

    # Clamp bbox to image bounds
    x0 = max(int(round(x)), 0)
    y0 = max(int(round(y)), 0)
    x1 = min(int(round(x + w)), W)
    y1 = min(int(round(y + h)), H)

    if x1 <= x0 or y1 <= y0:
        return None

    # Fallback: median disparity in whole ROI
    disp_roi = disparity[y0:y1, x0:x1]
    valid_roi = ~np.isnan(disp_roi)
    valid_pixels = np.count_nonzero(valid_roi)
    total_pixels = disp_roi.size

    if total_pixels == 0 or valid_pixels < min_valid_fraction * total_pixels:
        # not enough info
        return None

    d_global = float(np.nanmedian(disp_roi))

    def sample_disp_at(cx, cy):
        """Median disparity in a small window around (cx, cy)."""
        hw = corner_win // 2
        xx0 = max(int(round(cx)) - hw, 0)
        yy0 = max(int(round(cy)) - hw, 0)
        xx1 = min(int(round(cx)) + hw + 1, W)
        yy1 = min(int(round(cy)) + hw + 1, H)

        patch = disparity[yy0:yy1, xx0:xx1]
        valid = ~np.isnan(patch)
        if not valid.any():
            return None
        return float(np.nanmedian(patch[valid]))

    # Four corners in the left rectified image
    corners_L = [
        (x0, y0),
        (x1, y0),
        (x0, y1),
        (x1, y1),
    ]

    corners_R_x = []
    for (cx, cy) in corners_L:
        d = sample_disp_at(cx, cy)
        if d is None:
            d = d_global  # fallback to global median
        xR = cx - d  # left camera is reference → x_right = x_left - disp
        corners_R_x.append(xR)

    # Build an axis-aligned bbox that covers all four projected corners
    xR_min = max(int(round(min(corners_R_x))), 0)
    xR_max = min(int(round(max(corners_R_x))), W)

    if xR_max <= xR_min:
        return None

    # y does not change under rectified stereo, keep same vertical span
    yR_min = y0
    yR_max = y1

    return (xR_min, yR_min, xR_max - xR_min, yR_max - yR_min)


def warp_bbox_raw_to_rect(bbox_raw,
                          K, dist,
                          R_rect, P_rect,
                          nx=10, ny=6):
    """
    Map a bbox from RAW (distorted) image coordinates to the RECTIFIED image.

    bbox_raw : (x, y, w, h) on the raw image (1280x720 for the DVS).
    K, dist  : intrinsics and distortion of the *raw* camera.
    R_rect   : rectification rotation from cv.stereoRectify (R1).
    P_rect   : new projection matrix from cv.stereoRectify (P1).
    nx, ny   : number of samples in x / y inside the box to approximate warping.

    Returns: (x_rect, y_rect, w_rect, h_rect) on rectified image.
    """

    x, y, w, h = bbox_raw

    # sample a grid inside the bbox (inclusive end)
    xs = np.linspace(x, x + w, nx, dtype=np.float32)
    ys = np.linspace(y, y + h, ny, dtype=np.float32)
    grid_x, grid_y = np.meshgrid(xs, ys)  # shape (ny, nx)

    frame_img_points_raw = np.stack([grid_x, grid_y], axis=-1).reshape(-1, 1, 2)  # (N,1,2)

    # undistort + rectify all sample points
    pts_rect = cv.undistortPoints(pts_raw, K, dist, R=R_rect, P=P_rect)
    pts_rect = pts_rect.reshape(-1, 2)  # (N,2), in pixel coords because P_rect given

    x_min, y_min = pts_rect.min(axis=0)
    x_max, y_max = pts_rect.max(axis=0)

    x_min = int(np.floor(x_min))
    y_min = int(np.floor(y_min))
    x_max = int(np.ceil(x_max))
    y_max = int(np.ceil(y_max))

    return (x_min, y_min, x_max - x_min, y_max - y_min)


if __name__ == '__main__':
    # Left camera is Prophesee, Right camera is Basler
    data_dir_left = "C:/Users/Bas_K/source/repos/Thesis/data/images/prophesee_dvs/*.png"
    debug_dir_left = "C:/Users/Bas_K/source/repos/Thesis/data/output/prophesee_dvs/"
    data_dir_right = "C:/Users/Bas_K/source/repos/Thesis/data/images/basler_rgb/*.png"
    debug_dir_right = "C:/Users/Bas_K/source/repos/Thesis/data/output/basler_rgb/"
    calib_json_left = "C:/Users/Bas_K/source/repos/Thesis/data/output/prophesee_dvs/calibration.json"
    calib_json_right = "C:/Users/Bas_K/source/repos/Thesis/data/output/basler_rgb/calibration.json"
    debug_dir = "C:/Users/Bas_K/source/repos/Thesis/data/output/"

    # rms_l, camera_matrix_l, dist_coefs_l, _rvecs_l, _tvecs_l = calibrate(data_dir_left, debug_dir_left)
    rms_r, camera_matrix_r, dist_coefs_r, _rvecs_r, _tvecs_r = calibrate(data_dir_right, debug_dir_right)

    # camera_matrix_l, dist_coefs_l = load_calibration(calib_json_left)
    # camera_matrix_r, dist_coefs_r = load_calibration(calib_json_right)
    #
    # rawL = cv.imread("C:/Users/Bas_K/source/repos/Thesis/data/output/prophesee_dvs/frame_2057_board.png")
    # rawR = cv.imread("C:/Users/Bas_K/source/repos/Thesis/data/output/basler_rgb/frame_2057_board.png")
    #
    # R, T = stereo_calibrate(camera_matrix_l, dist_coefs_l, camera_matrix_r, dist_coefs_r, data_dir_left, data_dir_right)
    #
    # R1, R2, P1, P2, Q, roi1, roi2 = cv.stereoRectify(camera_matrix_l, dist_coefs_l, camera_matrix_r, dist_coefs_r,
    #                                                  (1920, 1200), R, T, alpha=0.89)
    # # (1920, 1200)
    # # (1280, 720)
    # map1L, map2L = cv.initUndistortRectifyMap(camera_matrix_l, dist_coefs_l, R1, P1, (1920, 1200), cv.CV_16SC2)
    # map1R, map2R = cv.initUndistortRectifyMap(camera_matrix_r, dist_coefs_r, R2, P2, (1920, 1200), cv.CV_16SC2)
    #
    # rectL = cv.remap(rawL, map1L, map2L, cv.INTER_LINEAR)
    # rectR = cv.remap(rawR, map1R, map2R, cv.INTER_LINEAR)
    #
    # disp = compute_disparity(rectL, rectR)
    #
    # bbox_left_raw = (656, 526, 400, 192)  # from your detector on rectL
    # bbox_rect_left = warp_bbox_raw_to_rect(
    #     bbox_left_raw,
    #     camera_matrix_l, dist_coefs_l,
    #     R1, P1,
    # )
    #
    # bbox_right = transfer_bbox_left_to_right(bbox_rect_left, disp)
    #
    # print("rectL shape:", rectL.shape)
    # print("rectR shape:", rectR.shape)
    # print("bbox_left:", bbox_rect_left)
    #
    # disp_roi = disp[
    #     bbox_rect_left[1]:bbox_rect_left[1] + bbox_rect_left[3],
    #     bbox_rect_left[0]:bbox_rect_left[0] + bbox_rect_left[2],
    # ]
    # valid = ~np.isnan(disp_roi)
    # print("valid disparity fraction:",
    #       np.count_nonzero(valid) / disp_roi.size if disp_roi.size > 0 else 0,
    #       )
    # print("median disparity in bbox:", np.nanmedian(disp_roi))
    #
    # print("bbox_right:", bbox_right)
    #
    # xL, yL, wL, hL = bbox_rect_left
    # cv.rectangle(rectL, (xL, yL), (xL + wL, yL + hL), (0, 255, 0), 2)
    #
    # if bbox_right is not None:
    #     xR, yR, wR, hR = bbox_right
    #     cv.rectangle(rectR, (xR, yR), (xR + wR, yR + hR), (0, 0, 255), 2)
    #
    # cv.imshow("rectL", rectL)
    # cv.imshow("rectR", rectR)
    # cv.waitKey(0)
    #
    # cv.destroyAllWindows()
