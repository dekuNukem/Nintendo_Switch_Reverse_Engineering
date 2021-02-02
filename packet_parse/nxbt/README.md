
# NXBT Bluetooth communication dissector

Plugin for Wireshark to dissect the bluetooth communication between a Controller and the Nintendo Switch (NX)

## How to install

Unless there is a compiled binary available for your version of Wireshark, you
need to compile wireshark from source. below is how one would do this.

Get Wireshark and dependencies
```
# apt-get install wireshark-dev Ninja
$ git clone https://gitlab.com/wireshark/wireshark.git
```

Copy this plugin into place
```
cp -r <this folder> wireshark/plugins/epan/
```

setup the build environment (in here `wireshark-build`).
Other make utilities are available, I used `ninja`.
```
mkdir wireshark-build
cd wireshark-build
cmake -DCUSTOM_PLUGIN_SRC_DIR="plugins/epan/nxbt" -G ninja ../wireshark
```

compile the entire thing, this takes 10-30 mins the first time.
```
ninja
```

to run this Wireshark, run
```
./run/wireshark
```

or instead copy the compiled plugin in `./run/plugins/<version>/epan/nxbt.so`
to your local wireshark's plugin directory, e.g. `~/.local/lib/wireshark/plugins`.

## If it isn't picked up

If it shows as HID, ignore this protocoll or disable it, then restart.

If you didn't actually capture bluetooth traffic, you might have to change the
dissectortable rules in `proto_reg_handoff_nxbt`.
