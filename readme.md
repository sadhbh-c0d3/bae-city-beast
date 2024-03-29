# Bae City Beast

### Bae City Beast Microservice

[![Watch My Video!](https://img.youtube.com/vi/TpqWKZnNAak/0.jpg)](https://youtu.be/TpqWKZnNAak)

---

## About

Microservice framework in C++20 built on top of *Boost.Beast*

The *Boost.Beast* comes with [example](https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/coro-ssl/http_server_coro_ssl.cpp). 

In this project I have organised code into classes, so that user can easily create microservice on top of *Boost.Beast*.

### Usage

User needs to define three classes:

- `SslCertificateLoaderConcept MySecurity` - should load SSL certificate
- `LoggerConcept MyLogger` - should write logging messages
- `ServerConcept MyServer` - should handle requests

and then HTTPS service can be started in following way:
```
    auto security = MySecurity{};
    auto config = bae::city::beast::SecureConfig<MySecurity>{security, address, port, thread_count};

    auto logger = MyLogger{};
    auto server = MyServer{document_root};
    auto service = bae::city::beast::Service<
        MyLogger, bae::city::beast::DynamicRequests, MyServer>{logger, server};
    
    return service(config);
```

See [main.cpp](https://github.com/sadhbh-c0d3/bae-city-beast/blob/main/src/main.cpp) for implementation examples.

### No Router

This is ultra-simple HTTPS microservice, barely a wrapper around Boost.Beast, and it does not provide complex routing.

### No Smart Pointers

Note that all objects in the example above are created on stack. No need to use smart pointers.

Also, **note** that use of smart pointers would defeat the idea of static (compile-time) polymorphism.

## Building automatic

Create Docker container and build project within that container.

Edit `./docker/.Dockerfile` comment and uncomment following lines:
```
# These will make docker container automatically build the solution
ADD "./build-and-run.sh" "/home/docker/"
CMD ["./build-and-run.sh"] 

# and if these are used instead of the other two above, we can manually build
#ADD "./run-forever.sh" "/home/docker/"
#CMD ["./run-forever.sh"]
```

Use Docker Compose:
```
    docker-compose up -d
```

It will build whole new container, and then it will build the sources, and run the image server.

## Building manually

Create Docker container and enter that container.

Edit `./docker/.Dockerfile` comment and uncomment following lines:
```
# These will make docker container automatically build the solution
#ADD "./build-and-run.sh" "/home/docker/"
#CMD ["./build-and-run.sh"]

# and if these are used instead of the other two above, we can manually build
ADD "./run-forever.sh" "/home/docker/"
CMD ["./run-forever.sh"]
```

Launch Docker container
```
    docker-compose up -d
```

Enter development environment within Docker container
```
    ./docker/enter-app.sh
```

Configure
```
    mkdir /home/build
    cd /home/build
    cmake /home/project
```

Build
```
    make
```

Test
```
    ctest
```

Run
```
    ./bin/run_app 0.0.0.0 8080 /home/volume 1
```

Test GET Request
```
   ./test-request.sh some-path
```

Test POST Request
```
   ./test-request.sh some-path -X POST -T some-file
```

## See also

[Other projects using Boost.Beast](https://github.com/boostorg/beast/wiki/Companies-and-Individuals-Using-Beast)
