
LBA World
===========
C++/Qt-based runtime asset library and toolchain for LBA1 + LBA2
(Little Big Adventure, Adeline Software, 1994).

Features
---------
1) Runtime HQR access with indexed, lazy block decompression
2) CLI utility `hqrpak`: inspect, validate, extract or create HQR archives
3) Utility "flaplayer": Viewer for LBA1 cinematics (*.FLA)
4) Utility "lbaspriteviewer": LBA1/LBA2 2D-Sprite viewer (SPRITES.HQR + SPRITRAW.HQR)
5) Utility "lbamodelviewer": LBA1 3D-Models viewer (BODY.HQR + INVOBJ.HQR)
6) Application "lbaworld": foundation for a new LBA game using the original assets (WIP)

![FLAPlayer](https://github.com/solarix95/lbaworld/blob/master/doc/flaplayer.png)
![FLAPlayer](https://github.com/solarix95/lbaworld/blob/master/doc/modelviewer.png)

First steps
---------
1) Buy "Little Big Adventure" on steam
2) "I'm on Linux: can't download LBA1 from Steam": 
No problem, buy it first on steam, then download LBA and/or LBA2 with "steamcmd"
```
$ sudo apt-get install steamcmd
$ steamcmd 
Steam> @sSteamCmdForcePlatformType windows
Steam> login <your-steam-account>
Steam> force_install_dir lba1steam
Steam> app_update 397330 validate
Steam> force_install_dir lba2steam
Steam> app_update 398000 validate
Steam> quit
```
see also:
* https://developer.valvesoftware.com/wiki/SteamCMD#Linux
* https://steamdb.info/search/?a=app&q=lba

3) Dependencies, sibling repositories and build (Qt 5)
```bash
sudo apt install build-essential qtbase5-dev qt5-qmake libqt5opengl5-dev \
    libsdl1.2-dev libsdl-mixer1.2-dev libassimp-dev

git clone https://github.com/solarix95/libqtr3d.git
git clone https://github.com/solarix95/lbaworld.git
mkdir lbaworld-build
cd lbaworld-build
qmake ../lbaworld/lbaworld.pro
make -j8
```

The default build expects `libqtr3d` and `lbaworld` to be sibling directories.
For a different location, pass `LIBQTR3D_PATH=/absolute/path/to/libqtr3d` to
qmake. SFML is not required; its experimental backend can be enabled with
`CONFIG+=lbaworld_sfml`.

Run applications directly from the shadow-build `bin` directory:

```bash
./bin/lbaworld --lba1base=<dir-to-lba1-data>
./bin/lbaspriteviewer --lba1base=<dir-to-lba1-data> --lba2base=<dir-to-lba2-data>
./bin/lbwmodelviewer --lba1base=<dir-to-lba1-data> --entity=0 --animation-demo
```

`LbaRess` indexes the original HQR archives and reads entries on demand. It does
not require an exported asset directory. Data paths can be supplied with the
command-line options above or the `LBA1BASE`, `LBA2BASE` and `LBAWBASE`
environment variables.

LBA World resources use the same API with either a packed HQR archive or an
unpacked development directory. For example, `lbaw://maps/flat` resolves
`flat` through entry 0 of `MAPS.HQR`, or through
`maps/0000-index.json` when the loose `maps` directory is present. Loose
archives override same-named HQR files and are read on every access, so editing
does not require repacking. Entry filenames begin with their numeric archive
index, such as `0001-flat.json`; gaps remain unavailable archive entries.

The repository contains the initial loose map resources in `data/maps`. They
can be used directly with `--lbawbase=../lbaworld/data`. The normal data build
also packs the exact same files into `bin/lbawbase/MAPS.HQR` for deployment.

The main application starts without a map. Open the console with `F1` and load
the initial plane with `map flat`. The command reads `lbaw://maps/flat` through
the resource manager, validates the versioned JSON map description, loads
Twinsen directly from the original LBA1 `FILE3D.HQR`, `BODY.HQR`, `ANIM.HQR`
and `RESS.HQR`, and places him at the map's `playerSpawn`. Use Up/Down to walk
forward/backward and Left/Right to turn; the original idle, walk and backward
walk animations provide the root motion. For startup automation, pass
`--exec="map flat"`.

`lbwmodelviewer` retains its BODY/ANIM index controls. Passing `--entity=N`
additionally enables the runtime character path: logical bodies and named
animations are resolved through `FILE3D.HQR`, then BODY and ANIM entries are
read directly from their original archives. `--logical-body=N` selects a
logical body (default `0`), and `--animation-demo` runs
`idle -> walk -> idle` including actor root motion.

HQR diagnostics:

```bash
./bin/hqrpak --list BODY.HQR
./bin/hqrpak --validate BODY.HQR
./bin/hqrpak --extract BODY.HQR extracted-body
```

Milestones
---------

- [x] Validating HQR reader with lazy runtime access
- [x] FLA-Player inluding sound (*.FLA: LBA cinematic clips)
- [x] LBA1 Model Viewer including animation keyframes
- [x] FILE3D character mapping and named LBA1 animations
- [x] LBA1 animation timing, loop transitions and actor root motion
- [x] LBA1 Sprites
- [x] LBA2 (Raw)-Sprites
- [] WIP: LBA2 3d-Models
- [ ] Character state machine -> Scene -> Scripts, ...
- [ ] Proper 3D-Layers (SceneGraph)
- [ ] ...
- [ ] LBA3 (what else)

Bugs
---------

Other ressources
---------
* blender-lba: https://github.com/vegard/blender-lba
* twine-e: https://github.com/xesf/twin-e
* lba2 remake: https://github.com/agrande/lba2remake
* LBA1 Files: http://lbafileinfo.kazekr.net/index.php?title=LBA_1_files
