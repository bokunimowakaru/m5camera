#!/bin/bash
gzip -k camera_index_ov2640.html
../tools/gz2header.py ../CameraWebServer/camera_index_ov2640.html.gz
