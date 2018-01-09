#!/bin/sh 
cd /sdcard/test
dex2oat --dex-file=EvilModule.dex --oat-file=EvilModule.oat 
oatdump --oat-file=EvilModule.oat --output=EvilModule.dump