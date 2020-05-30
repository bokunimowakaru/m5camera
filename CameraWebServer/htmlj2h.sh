#!/bin/bash
gzip -k camera_index_ov2640_J.html
mv camera_index_ov2640_J.html.gz camera_index_ov2640.html.gz
../tools/gz2header.py ../CameraWebServer/camera_index_ov2640.html.gz
