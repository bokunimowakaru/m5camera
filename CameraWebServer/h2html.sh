#!/bin/bash
../tools/header2gz.py ../CameraWebServer/camera_index_ov2640.h
gzip -d camera_index_ov2640.html.gz
