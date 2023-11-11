# m5stamp-c3wamr

![](https://github.com/h1romas4/m5stamp-c3wamr/workflows/Build/badge.svg)

### Require

- [Setup ESF-IDF **v4.4.6**](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32c3/get-started/index.html#installation-step-by-step)

get_idf or (Windows) ESP-IDF **`4.4.6`** command prompt

```
# Linux or macOS ->
$ alias get_idf='. $HOME/esp/esp-idf/export.sh'
$ get_idf
# <-

Detecting the Python interpreter
... snip ...
Done! You can now compile ESP-IDF projects.
Go to the project directory and run:

  idf.py build

$ echo ${IDF_PATH}
/home/hiromasa/devel/toolchain/esp/esp-idf

$ riscv32-esp-elf-gcc -v
Using built-in specs.
COLLECT_GCC=riscv32-esp-elf-gcc
... snip ...
gcc version 8.4.0 (crosstool-NG esp-2021r2-patch5)
```

## Build

1. git clone and build (Please add `--recursive`)

```
git clone --recursive https://github.com/h1romas4/m5stamp-c3warm
cd m5stamp-c3warm
idf.py build
```

2. Write Partition table

```
idf.py partition-table-flash
```

3. Write TypeType font to SPIFFS

```
parttool.py write_partition --partition-name=font --partition-subtype=spiffs --input resources/spiffs_font.bin
```

4. Write WebAssembly(.wasm) to SPIFFS

```
parttool.py write_partition --partition-name=wasm --partition-subtype=spiffs --input resources/spiffs_wasm.bin
```

5. Write main program to go!

```
idf.py flash monitor
```

## Create SPIFFS partition file from file system

```
python ${IDF_PATH}/components/spiffs/spiffsgen.py 0x100000 resources/font resources/spiffs_font.bin
python ${IDF_PATH}/components/spiffs/spiffsgen.py 0x10000 resources/wasm resources/spiffs_wasm.bin
```

## Dependencies

Thanks for all the open source.

|Name|Version|License|
|-|-|--|
|[esp-idf](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32c3/get-started/index.html)|`v4.4.6`|BSD License|
|[arduino-esp32](https://github.com/espressif/arduino-esp32/releases/tag/2.0.13)|`2.0.14`|LGPL-2.1 License|
|[WebAssembly Micro Runtime (WAMR)](https://github.com/bytecodealliance/wasm-micro-runtime)|`1.2.3`|Apache-2.0 License|
|[AssemblyScript](https://github.com/AssemblyScript/assemblyscript)|`0.27.14`|Apache-2.0 License|
|[源真ゴシック](http://jikasei.me/font/genshin/)|-|SIL Open Font License 1.1|

## License

MIT License
