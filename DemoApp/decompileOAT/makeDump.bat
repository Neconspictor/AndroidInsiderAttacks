adb root
adb push dumpCommands.sh /sdcard/test/dumpCommands.sh
adb push EvilModule.dex /sdcard/test/EvilModule.dex
adb shell sh /sdcard/test/dumpCommands.sh
adb pull /sdcard/test/EvilModule.dump EvilModule.dump