# jexpr

## Disclaimer

This code is exploratory, not for production use. Use at your own risk!

## Motivation

ExprTk is a useful way to integrate mathematical expressions into C++ code. It was thought that it might be useful to have a JSON-based interface around ExprTk.

## Interfaces

There are Python and javascript interfaces

### Python

The Python one is built with ``pybind11``, and can be built with:

```
pip -vv wheel .
```

in the top level folder containing this file. Then, this wheel gets installed into your environment.

### JS

The js one, built with Emscripten, can be built with the docker container (for ease of use). More docs in the ``interface/js``, folder.

## Contacts

Ian Bell, ian.bell@nist.gov