# Overview

Embedded application with CLI for testing STM32 ADC peripheral model.

## Build

CMake based with make wrapper. To build, will need ARM's gnurm toolchain on path.

```
make build
```
or
```
TOOLCHAIN_DIR=path/to/toolchain make build
```
or with invoke & docker (will pull image grifcj/gnurm)
```
python3 -m venv venv
source venv/bin/activate
pip install invoke
invoke build
```
