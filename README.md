
LBA World
===========
C++/Qt-based Toolchain for LBA1 + LBA2 (Little Big Adventure, Adeline Software, 1994) 

Features
---------
1) Utility "flaplayer": Viewer for LBA1 cinematics (*.FLA)
2) Utility "lbaspriteviewer": LBA1/LBA2 2D-Sprite viewer (SPRITES.HQR + SPRITRAW.HQR)
3) Utility "lbamodelviewer": LBA1 3D-Models viewer (BODY.HQR + INVOBJ.HQR)

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

3) Dependencies, Download, Build + Run (Ubuntu 16.04)
```bash
$ sudo apt install libsdl-mixer1.2-dev
$ sudo apt install qt5-default qt5-qmake libqt5opengl5-dev
$ git clone https://github.com/solarix95/lbaworld.git
$ cd lbaworld/flaplayer
$ qmake && make -j8
$ ./flaplayer --lba1base=<dir-to-lba1-data>

e.g.
$ ./flaplayer --lba1base=/home/<yourname>/.steam/steamcmd/lba1steam/resources/LBA_FILES
$ ./lbaspriteviewer --lba1base=/home/<yourname>/.steam/steamcmd/lba1steam/ --lba2base=/home/<yourname>/.steam/steamcmd/lba2steam/
```

Milestones
---------

- [x] HQR-Reader (a very basic milestone :))
- [x] FLA-Player inluding sound (*.FLA: LBA cinematic clips)
- [x] LBA1 Model Viewer including animation keyframes
- [x] LBA1 Sprites
- [x] LBA2 (Raw)-Sprites
- [] WIP: LBA2 3d-Models
- [ ] Understanding Body -> Animation -> Actor -> Character -> Scene -> Scripts , ...
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



