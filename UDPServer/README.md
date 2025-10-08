Build on MacOS for AMD
    docker build --platform=linux/amd64 -t isparkes/udp-quote-server:002 .

Build on MacOS for MacOS
    docker build -t isparkes/udp-quote-server:001m .

Run it
    docker run -p 2222:2222/udp isparkes/udp-quote-server:001m 

Push it
    docker push isparkes/udp-quote-server:004