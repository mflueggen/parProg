## Metal FS Tutorial

This project illustrates how to create a simple operator for [Metal FS](https://github.com/osmhpi/metal_fs) by using a dockerized development environment.

The provided Dockerfile relies on a Xilinx Vivado 2018.1 Docker image, which you can obtain from here: ftp://qg8ptrv:bs2rvej@ftp.hpi.uni-potsdam.de

You can either start the development environment by using the Remote Containers extension in VS Code, or by running `start_linux`, `start_osx` or `start_win.bat` respectively. 
> Note for VS Code users: If you are using a non-Linux host and want to start Vivado X11 applications from the container, please change the DISPLAY environment variable parameter to `"DISPLAY=host.docker.internal:0.0"` in [`.devcontainer/devcontainer.json`](.devcontainer/devcontainer.json)

### Project Structure

The project is structured as follows:
```
- Makefile
- image.json
- src
  `-- hls_operator_colorfilter
      |-- Makefile
      |-- hls_operator_colorfilter.cpp
      |-- operator.json
      `-- testbench.cpp
```

The [`image.json`](image.json) file contains the project configuration: It specifies which FPGA to target and which operators to include in the image.

> Note: The default configuration does not enable the DRAM and NVMe capabilities of the N250S FPGA card.

The root [Makefile](Makefile) offers access to builtin SNAP targets (e.g. `make model`, `make sim`, `make image`).

Under [`src/hls_operator_colorfilter`](src/hls_operator_colorfilter), an example operator is defined that converts a bitmap image to grayscale.

Its [`operator.json`](src/hls_operator_colorfilter/operator.json) file contains the operator manifest which is used at build time to describe the operators configuration interface. Furthermore it specifies the command line options to be used at runtime.

[`hls_operator_colorfilter.cpp`](src/hls_operator_colorfilter/hls_operator_colorfilter.cpp) contains the operator implementation in Vivado HLS. [`testbench.cpp`](src/hls_operator_colorfilter/testbench.cpp) can be used to test the implementation in software by running `make test` from the operator directory.

### How to try it out

Once you have started the development Docker container, run `make model` to generate a simulation model. Afterwards, start a simulation environment with `make sim`.

In the simulation shell, first initialize the environment with `snap_maint`, then start the file system driver: `metal_fs /mnt`.

Start a second shell in the container and try out the operator with the provided bitmap file:
```
cat src/hls_operator_colorfilter/apples_simulation.bmp \
  | /mnt/operators/colorfilter \
  > out.bmp
```
