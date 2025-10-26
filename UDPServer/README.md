Build on MacOS for AMD
    docker build --platform=linux/amd64 -t isparkes/udp-quote-server:005 .

Build on MacOS for MacOS
    docker build -t isparkes/udp-quote-server:005m .

Run it
    docker run -p 2222:2222/udp isparkes/udp-quote-server:005m 

Push it
    docker push isparkes/udp-quote-server:005