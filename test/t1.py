# -*- coding:utf-8 -*- #
"""
-------------------------------------------------------------------
   File Name：     t1.py
   Author :       wdian
   create date：   2021/7/1
-------------------------------------------------------------------
"""

import os
import ctypes as cty
import numpy as np

root_path = os.path.dirname(__file__)
lib_path = "../release/libCalibrate.so"

dll_cal = cty.cdll.LoadLibrary(os.path.join(root_path, lib_path))

arr = np.fromfile("./data/output-float32-1_255_13_13.bin", dtype=np.float32)

min_val = np.min(arr)
max_val = np.max(arr)
th = max(abs(min_val), abs(max_val))

hist, hist_edges = np.histogram(arr, bins=8001, range=(-th, th))
hist = hist.astype(np.float32)
hist_edges = hist_edges.astype(np.float32)

hist.tofile('./data/hist.bin')
hist_edges.tofile('./data/hist_edges.bin')

hist_ptr = hist.ctypes.data_as(cty.POINTER(cty.c_float))
hist_size = cty.c_int(len(hist))
hist_edges_ptr = hist_edges.ctypes.data_as(cty.POINTER(cty.c_float))
out_threshold = cty.pointer(cty.c_float())
out_divergence = cty.pointer(cty.c_float())
num_quantized_bins = cty.c_int(255*2+1)

dll_cal.calibrate(hist_ptr, hist_size, hist_edges_ptr, out_threshold, out_divergence, num_quantized_bins)
print(out_threshold.contents.value)
print(out_divergence.contents.value)
