FROM ubuntu:latest

RUN apt update --fix-missing
RUN apt install -y g++ build-essential cmake libssl-dev libboost-all-dev 

RUN mkdir -p /home/docker
RUN mkdir -p /home/project
RUN mkdir -p /home/build
RUN mkdir -p /home/volume

WORKDIR "/home/docker"

# These will make docker container automatically build the solution
#ADD "./build-and-run.sh" "/home/docker/"
#CMD ["./build-and-run.sh"]

# and if these are used instead of the other two above, we can manually build
ADD "./run-forever.sh" "/home/docker/"
CMD ["./run-forever.sh"]

