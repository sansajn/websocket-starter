# WebSockets starter project

C++ WebSocket starter template for easy prototyping on *Ubuntu 20.04 LTS* with Scons build system. As usualy just three simple steps *setup*, *clone* and *buil & run* and we are ready to start experiment with WebSockets. So let's dive in

> if you are not yet familiar with SCons watch out [SCons starter][scons-starter] tutorial

## 1. setup

Presented WebSocket channel implementation is build on top of *libsoup* library and it is also the only dependency there (except the build environment ofcourse). *Libsoup* library package can be installed with

```bash
sudo apt install libsoup2.4-dev
```

command.

> for the build environment use `sudo apt install g++ git scons` (if there is any package missing in the commnd, please let me know)


## 2. clone

Clone starter project repository with

```bash
git clone https://github.com/sansajn/test.git
```

command which creates `test` directory with `libsoup/ws_channel` subdirectory with starter project.


## 3. build & run

To start a build process run `scons`

```bash
scons -j16
```

> we can speed up building with `-jN` argument where `N` is number of available cores/threads

Currently two programs are produced, `eserv` (WebSocket echo server sample) and `test` (library unit tests).

To play with echo server sample, run

```console
$ ./eserv 
glib event loop created
listenning on ws://localhost:41001/test WebSocket address
press ctrl+c to quit
```

and open `websocket.html` file in a web-browser window and you should see

```
- WebSocket is supported by your Browser!
- ws << John
- ws >> John
```

on the page if everithing is working.

We are done, feel free to modify ...

See also [OGRE starter project][OGRE-starter], [SConst starter project][scons-starter] for more starter templates. 

[OGRE-starter]: https://github.com/sansajn/ogre-linux-starter
[scons-starter]: https://github.com/sansajn/scons-starter
