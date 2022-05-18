# WebSocket C++ starter project

C++ WebSocket starter template for easy prototyping on *Ubuntu 22.04 LTS* with [Scons][scons] build system. As usualy just three simple steps *setup*, *clone* and *buil & run* and we are ready to start experiment with WebSockets. So let's dive in

> **tip**: if you are not yet familiar with SCons watch out [SCons starter][scons-starter] tutorial

## 1. setup

Presented WebSocket channel implementation is build on top of *libsoup* library and it is also the only dependency there (except the build environment ofcourse). *Libsoup* library package can be installed with

```bash
sudo apt install libsoup2.4-dev
```

command.

> **tip**: for the build environment use `sudo apt install g++ git scons` (if there is any package missing in the commnd, please let me know)


## 2. clone

Clone starter project repository with

```bash
git clone https://github.com/sansajn/websocket-starter.git
```

command which creates `websocket-starter` directory with starter project files.


## 3. build & run

To start a build process run `scons`

```bash
scons -j16
```

> **tip**: we can speed up building with `-jN` argument where `N` is number of available cores/threads

Which produce, `eserv` (WebSocket echo server sample) and `test` (library unit tests).

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

on the page if everything is working.


### Secure WebSocket connection

Secure WebSocket connection is also supported, there are `seserv` (Secure WebSocket echo server sample) and `sclient` (Secure client sample) sample programs to demonstrate.

> **note**: `seserv` and `sclient` are produced together with `eserv` ans `test` during build process

- run echo server with

```console
$ ./seserv 
glib event loop created
listenning on wss://localhost:41001/test WebSocket address
press ctrl+c to quit
glib event loop running ...
```

command

- run client with

```console
$ ./sclient 
glib event loop created
connectiong to wss://localhost:41001/test WebSocket address...
press ctrl+c to quit
glib event loop running ...
<<< hello!
>>> hello!
```

command. The client send `"hello!"` and expect the same replay from echo server.


We are done, feel free to modify ...

See also [OGRE starter project][OGRE-starter], [SConst starter project][scons-starter] for more starter templates. 

[OGRE-starter]: https://github.com/sansajn/ogre-linux-starter
[scons-starter]: https://github.com/sansajn/scons-starter
[scons]: https://scons.org/
