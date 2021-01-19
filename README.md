# Rock Replay ![pipeline](https://git.hb.dfki.de/badge_server/rock-cpp/rock_replay/pipeline) ![build](https://git.hb.dfki.de/badge_server/rock-cpp/rock_replay/build) ![test](https://git.hb.dfki.de/badge_server/rock-cpp/rock_replay/test) ![test_coverage](https://git.hb.dfki.de/badge_server/rock-cpp/rock_replay/test_coverage)

A fast log file replay package for rock's logging system, written completely in C++. Currently, it utilizes Qt4, which is abandoned in Ubuntu 20.04 and later. Qt5 support is planned to be implemented next.


## Usage
Run the following from a terminal with sourced env.sh:
```
Usage: rock-replay2 {logfile|*}.log or folder
Available options:
  --help                show this message
  --prefix arg          add prefix to all tasks
  --verbose             show additional output
  --log-files arg       log files
```

## Bug Reports and Feature Requests
Please use the [GitHub Issue Tracker](https://github.com/rock-cpp/rock_replay/issues) of this repository.

## Contributing

### Styling
The repository contains a `.clang-format` file, which contains a fixed styleguide for this project. It can be applied manually with `clang-format` or automatically within your favourite IDE.

### Static Code Analysis
Use the `.clang-tidy` file with `clang-tidy` to perform a range of given static code checks.

### Test Coverage
Generate a coverage report with `gcovr` using either the `.gcovr-config` or `.gcovr-config-html` file to avoid regression. Therefore, unit tests must be run from the build folder with `./test/test_suite`. Additionally, run `gcovr --config {config-file}` from the root folder.

## License
Todo

## Maintainer
Todo




