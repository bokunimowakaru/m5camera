#!/usr/bin/env python3
# coding: utf-8

################################################################################
# ZIP圧縮されたファイルをヘッダファイルに変換します（CameraWebServer用）
#
################################################################################
# 使用、改変、再配布は自由に行えますが、無保証です。権利情報の改変は不可です。
# Copyright (c) 2020 Wataru KUNINO
################################################################################

camera	 = 'ov2640' # 'ov2640'or 'ov3660'

filename = '../CameraWebServer/camera_index_'+camera+'.html.gz'
saveto	 = '../CameraWebServer/camera_index_'+camera+'.h'

from time import sleep
from sys import argv					# 本プログラムの引数argvを取得する
print(argv[0])							# プログラム名を表示する
if len(argv) == 2:
	filename = argv[1]
	i = filename.find('.html.gz')
	if i > 0 and i + 8 == len(filename):
		saveto = filename[0:i] + '.h'

print('input filename  =',filename)
print('output filename =',saveto)
fp = open(filename, mode='br')
data_array = fp.read()
fp.close()
out = []
for d in data_array:
	out.append(d)

fp = open(saveto, mode='w')
print('#define index_ov2640_html_gz_len',len(out), file = fp)
print('const uint8_t index_ov2640_html_gz[] = {', file = fp)
i=0
for d in out:
	# print(' ' + hex(d), end='', file = fp)
	print(' 0x' + format(d, '02X'), end='', file = fp)
	i += 1
	if i % 16 == 0:
		print(',', file = fp)
	else:
		if i < len(out):
			print(',', end='', file = fp)
print('\n};', file = fp)
fp.close()
