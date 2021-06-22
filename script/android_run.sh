export PATH=$PATH:~/code/sdk/sdk/platform-tools
export ANDROID_SDK="/Users/tpfaff/code/sdk/sdk"
export ANDROID_NDK="/Users/tpfaff/code/ndk"
export PATH=$PATH:/Users/tpfaff/code/ndk/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin
make -j6 && ($(adb shell dumpsys power | grep mScreenOn | grep -oE '(true|false)') || adb shell "input keyevent 26") && adb shell "input keyevent HOME" && adb install -r ResidualVM-debug.apk && adb shell am start -a android.intent.action.MAIN -n org.residualvm.residualvm/.ResidualVMActivity
