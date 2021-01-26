### Styling
The repository contains a `.clang-format` file, which contains a fixed styleguide for this project. It can be applied manually with `clang-format` or automatically within your favourite IDE.

### Static Code Analysis
Use the `.clang-tidy` file with `clang-tidy` to perform a range of given static code checks.

### Test Coverage
Generate a coverage report with `gcovr` using either the `.gcovr-config` or `.gcovr-config-html` file to avoid regression. Therefore, unit tests must be run from the build folder with `./test/test_suite`. Additionally, run `gcovr --config {config-file}` from the root folder.
