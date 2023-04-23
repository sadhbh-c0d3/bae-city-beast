# Bae City Beast

### Bae City Beast Microservice

---

## About

Microservice framework in C++20 built on top og *Boost.Beast*


## Building automatic

Create Docker container and build project within that container.
Start in *docker* directory:
```
cd ./docker
```

Edit `.Dockerfile` comment and uncomment following lines:
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

Create Docker container and build project within that container.
Start in *docker* directory:
```
cd ./docker
```

Edit `.Dockerfile` comment and uncomment following lines:
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
    ./enter-app.sh
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
    ./bin/run_app 0.0.0.0 8080 /home/bae-city-beast 1
```
