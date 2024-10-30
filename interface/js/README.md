# BUILD

A tiny docker container to compile jexpr to WASM and javascript

To build: ``docker-compose up --build``

Will generate ``jexpr.js`` and ``jexpr.wasm`` files

# RUN

Start a webserver with: ``python -m http.server``, navigate to ``localhost:8000/test_jexpr.html``, check console output, you can fiddle with the HTML file contents to experiment