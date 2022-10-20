# Mosquitto YAML auth plugin

Simple plugin that allows you to authenticate users against a YAML file.

## Sample configuration

```yaml
- username: user1
  password: password1
- username: user2
  password: password2
```

In `mosquitto.conf`

```ini
auth_plugin /path/to/mosquitto-yaml-auth.so
auth_opt_users_file /path/to/users.yaml
```

## Requirements

- `cmake`
- `pkg-config`
- `mosquitto-dev` (or `mosquitto-devel`)
- `llvm-dev` (or `llvm-devel`) and `llvm-static` (or `llvm-libs`)

## Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Building for the Mosquitto Docker image

I use the included `Dockerfile` to build the plugin for the Mosquitto Docker image, which I then simply place inside a
volume.

To build the plugin for the Mosquitto Docker image, run:

```bash
# Build the image and the plugin
docker build -t mosquitto-yaml-auth .
# Copy it outside the container
docker run --rm -v $PWD/libs/:/host -it mosquitto-yaml-auth cp /sources/build/libmosquitto_yaml_auth.so /host/
```

Find the built plugin in the `libs` directory.

Note that the plugin built this way only works on Alpine Linux.

## License

Licensed under the GNU Lesser General Public License v2.1 or later. See the [LICENSE](LICENSE) file for details.
